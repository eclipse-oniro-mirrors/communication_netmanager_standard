/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "net_policy_traffic.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>
#include <cinttypes>

#include "system_ability_definition.h"

#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_quota_policy.h"
#include "net_specifier.h"
#include "net_stats_info.h"
#include "net_manager_center.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyTraffic::NetPolicyTraffic(sptr<NetPolicyFile> netPolicyFile) : netPolicyFile_(netPolicyFile)
{
    monthDay_ = {MONTH_THIRTY_ONE, MONTH_TWENTY_EIGHT, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE, MONTH_THIRTY_ONE, MONTH_THIRTY,
        MONTH_THIRTY_ONE, MONTH_THIRTY, MONTH_THIRTY_ONE};
}

bool NetPolicyTraffic::IsPolicyValid(NetUidPolicy policy)
{
    switch (policy) {
        case NetUidPolicy::NET_POLICY_NONE:
        case NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND:
        case NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED:
        case NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND:
        case NetUidPolicy::NET_POLICY_ALLOW_ALL:
        case NetUidPolicy::NET_POLICY_REJECT_ALL: {
            return true;
        }
        default: {
            NETMGR_LOG_E("Invalid policy [%{public}d]", static_cast<uint32_t>(policy));
            return false;
        }
    }
}

bool NetPolicyTraffic::IsNetPolicyTypeValid(NetQuotaPolicyType netType)
{
    switch (netType) {
        case NetQuotaPolicyType::NET_POLICY_MOBILE:
        case NetQuotaPolicyType::NET_POLICY_ETHERNET:
        case NetQuotaPolicyType::NET_POLICY_WIFI:
        case NetQuotaPolicyType::NET_POLICY_BLUETOOTH:
        case NetQuotaPolicyType::NET_POLICY_PROXY: {
            return true;
        }
        default: {
            NETMGR_LOG_E("Invalid netType [%{public}d]", static_cast<uint32_t>(netType));
            return false;
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::AddPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("AddPolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD, uid, policy)) {
        NETMGR_LOG_E("AddPolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetPolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE, uid, policy)) {
        NETMGR_LOG_E("SetPolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::DeletePolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("DeletePolicyByUid netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, policy)) {
        NETMGR_LOG_E("DeletePolicyByUid WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

std::int64_t NetPolicyTraffic::GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec;
}

int32_t NetPolicyTraffic::GetPeriodEndTime()
{
    struct tm *timeNow;
    time_t second = time(nullptr);
    if (!second) {
        NETMGR_LOG_E("time second error");
        return -1;
    }
    timeNow = localtime(&second);
    if (timeNow == nullptr) {
        NETMGR_LOG_E("timeNow is nullptr");
        return -1;
    }

    if (timeNow->tm_mon == NET_POLICY_FEBRUARY
        && ((timeNow->tm_year % NET_POLICY_LEAP_YEAR_FOUR == 0
        && timeNow->tm_year % NET_POLICY_LEAP_YEAR_ONEHUNDRED != 0)
        || timeNow->tm_year % NET_POLICY_LEAP_YEAR_FOURHUNDRED == 0)) {
        return (monthDay_[timeNow->tm_mon] + NET_POLICY_LEAP_YEAR_ONE) * NET_POLICY_ONEDAYTIME;
    } else {
        return monthDay_[timeNow->tm_mon] * NET_POLICY_ONEDAYTIME;
    }
}

void NetPolicyTraffic::CheckNetStatsOverLimit(const std::vector<NetPolicyCellularPolicy> &cellularPolicies,
    const sptr<NetPolicyCallback> netPolicyCallback)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_W("cellularPolicies size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < cellularPolicies.size(); ++i) {
        /* -1 : unlimited */
        if (cellularPolicies[i].limitBytes_ == -1) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicies[i].subscriberId_, true);
            }
            continue;
        }
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(NET_TYPE_CELLULAR,
            cellularPolicies[i].subscriberId_, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, cellularPolicies[i].periodStartTime_,
            cellularPolicies[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);
        /*  The traffic exceeds the limit. You need to notify telephony to shut down the network. */
        if (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < cellularPolicies[i].limitBytes_) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicies[i].subscriberId_, true);
            }
        } else {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(cellularPolicies[i].subscriberId_, false);
            }
        }
    }
}

void NetPolicyTraffic::CheckNetStatsOverLimit(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies,
    const sptr<NetPolicyCallback> netPolicyCallback)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_W("quotaPolicies size is 0.");
        return;
    }

    int32_t periodEndTime = GetPeriodEndTime();
    if (periodEndTime <= 0) {
        NETMGR_LOG_E("periodEndTime error.");
        return;
    }

    std::string ifaceName;
    for (uint32_t i = 0; i < quotaPolicies.size(); ++i) {
        /* only control cellular traffic */
        if (static_cast<NetQuotaPolicyType>(quotaPolicies[i].netType_) != NetQuotaPolicyType::NET_POLICY_MOBILE) {
            NETMGR_LOG_I("need not notify telephony netType_[%{public}d]", quotaPolicies[i].netType_);
            continue;
        }
        int32_t ret = NetManagerCenter::GetInstance().GetIfaceNameByType(quotaPolicies[i].netType_,
            quotaPolicies[i].subscriberId_, ifaceName);
        if (ret != 0 || ifaceName.empty()) {
            NETMGR_LOG_E("GetIfaceNameByType ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NetStatsInfo netStatsInfo;
        ret = NetManagerCenter::GetInstance().GetIfaceStatsDetail(ifaceName, quotaPolicies[i].periodStartTime_,
            quotaPolicies[i].periodStartTime_ + periodEndTime, netStatsInfo);
        if (ret != 0) {
            NETMGR_LOG_E("GetIfaceStatsDetail ret [%{public}d] ifaceName [%{public}s]", ret, ifaceName.c_str());
            continue;
        }
        NETMGR_LOG_I("GetIfaceStatsDetail txBytes_[%{public}" PRId64 "] rxBytes_[%{public}" PRId64 "]",
            netStatsInfo.txBytes_, netStatsInfo.rxBytes_);

        /* Sleep time is not up Or nerverSnooze : lastLimitSnooze_=1 */
        if ((quotaPolicies[i].lastLimitSnooze_ >= quotaPolicies[i].periodStartTime_ ||
            quotaPolicies[i].lastLimitSnooze_ == -1)
            && (netStatsInfo.txBytes_ + netStatsInfo.rxBytes_ < quotaPolicies[i].limitBytes_)) {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(quotaPolicies[i].subscriberId_, true);
            }
        } else {
            if (netPolicyCallback != nullptr) {
                netPolicyCallback->NotifyNetStrategySwitch(quotaPolicies[i].subscriberId_, false);
            }
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::SetNetPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    for (uint32_t i = 0; i < quotaPolicies.size(); ++i) {
        int32_t netPolicyType = static_cast<int32_t>(quotaPolicies[i].netType_);
        if (!IsNetPolicyTypeValid(static_cast<NetQuotaPolicyType>(netPolicyType))) {
            NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", netPolicyType);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (!IsNetPolicyPeriodDurationValid(quotaPolicies[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", quotaPolicies[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(quotaPolicies, netPolicyCallback);

    if (!netPolicyFile_->WriteFile(quotaPolicies)) {
        NETMGR_LOG_E("SetNetPolicies WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyTraffic::IsNetPolicyPeriodDurationValid(const std::string &periodDuration)
{
    if (periodDuration.empty()) {
        NETMGR_LOG_E("periodDuration is empty");
        return false;
    }

    std::string subString = periodDuration.substr(0, PERIODDURATION_POS_NUM_ONE);
    if (subString != POLICY_QUOTA_MONTH_U && subString != POLICY_QUOTA_MONTH_L) {
        NETMGR_LOG_E("periodDuration must Mx");
        return false;
    }

    subString = periodDuration.substr(PERIODDURATION_POS_NUM_ONE, periodDuration.length() - PERIODDURATION_POS_NUM_ONE);
    int32_t day = static_cast<int32_t>(std::stol(subString));
    if (day > DAY_THIRTY_ONE || day < DAY_ONE) {
        NETMGR_LOG_E("periodDuration must Mx, x is [%{public}d] - [%{public}d]", DAY_ONE, DAY_THIRTY_ONE);
        return false;
    }

    return true;
}

NetPolicyResultCode NetPolicyTraffic::SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("cellularPolicies size is 0");
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    for (uint32_t i = 0; i < cellularPolicies.size(); ++i) {
        if (!IsNetPolicyPeriodDurationValid(cellularPolicies[i].periodDuration_)) {
            NETMGR_LOG_E("periodDuration [%{public}s] must Mx", cellularPolicies[i].periodDuration_.c_str());
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }

        if (cellularPolicies[i].limitAction_ < LIMIT_ACTION_ONE
            || cellularPolicies[i].limitAction_ > LIMIT_ACTION_THREE) {
            NETMGR_LOG_E("limitAction [%{public}d] must 1-3 ", cellularPolicies[i].limitAction_);
            return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
        }
    }

    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(cellularPolicies, netPolicyCallback);

    if (!netPolicyFile_->WriteFile(cellularPolicies)) {
        NETMGR_LOG_E("SetCellularPolicies WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    return NetPolicyResultCode::ERR_NONE;
}

bool NetPolicyTraffic::IsQuotaPolicyExist(const NetPolicyQuotaPolicy &quotaPolicy)
{
    std::vector<NetPolicyQuotaPolicy> quotaPolicies;
    if (netPolicyFile_->GetNetPolicies(quotaPolicies) != NetPolicyResultCode::ERR_NONE) {
        NETMGR_LOG_E("GetNetPolicies failed");
        return false;
    }

    if (quotaPolicies.empty()) {
        NETMGR_LOG_E("quotaPolicies is empty");
        return false;
    }

    for (uint32_t i = 0; i < quotaPolicies.size(); i++) {
        if (quotaPolicy.netType_ == quotaPolicies[i].netType_
            && quotaPolicy.subscriberId_ == quotaPolicies[i].subscriberId_) {
            NETMGR_LOG_D("netQuotaPolicy exist");
            return true;
        }
    }

    return false;
}

NetPolicyResultCode NetPolicyTraffic::SetSnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy,
    const sptr<NetPolicyCallback> &netPolicyCallback)
{
    if (!IsNetPolicyTypeValid(static_cast<NetQuotaPolicyType>(quotaPolicy.netType_))) {
        NETMGR_LOG_E("NetPolicyType is invalid policy[%{public}d]", static_cast<int32_t>(quotaPolicy.netType_));
        return NetPolicyResultCode::ERR_INVALID_QUOTA_POLICY;
    }

    if (!IsQuotaPolicyExist(quotaPolicy)) {
        NETMGR_LOG_E("quotaPolicy is not exist");
        return NetPolicyResultCode::ERR_QUOTA_POLICY_NOT_EXIST;
    }

    std::vector<NetPolicyQuotaPolicy> quotaPolicies = {quotaPolicy};
    /* Set the sleep time to the current time. */
    quotaPolicies[0].lastLimitSnooze_ = GetCurrentTime();
    /* Judge whether the flow exceeds the limit */
    CheckNetStatsOverLimit(quotaPolicies, netPolicyCallback);
    if (!netPolicyFile_->WriteFile(quotaPolicies)) {
        NETMGR_LOG_E("SetSnoozePolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOG_E("SetIdleTrustlist netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    /* If it exists, update it directly */
    for (auto iter = idleTrustList_.begin(); iter != idleTrustList_.end(); ++iter) {
        if (uid == *iter) {
            if (!isTrustlist) {
                idleTrustList_.erase(iter);
                return NetPolicyResultCode::ERR_NONE;
            } else {
                return NetPolicyResultCode::ERR_NONE;
            }
        }
    }
    /* Does not exist, add it */
    if (isTrustlist) {
        idleTrustList_.emplace_back(uid);
    }
    /* Determine whether the app is idle ? than update netd's interface. */
    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    if (idleTrustList_.empty()) {
        NETMGR_LOG_I("idleTrustList_ is empty.");
    } else {
        uids = idleTrustList_;
    }

    return NetPolicyResultCode::ERR_NONE;
}

void NetPolicyTraffic::ClearIdleTrustList()
{
    idleTrustList_.clear();
}
} // namespace NetManagerStandard
} // namespace OHOS
