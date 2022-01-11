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
#include "net_policy_service.h"

#include <sys/time.h>
#include <unistd.h>
#include <cinttypes>

#include "system_ability_definition.h"

#include "net_policy_cellular_policy.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_quota_policy.h"
#include "net_policy_traffic.h"

#include "net_mgr_log_wrapper.h"
#include "net_settings.h"
#include "net_manager_center.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get());

NetPolicyService::NetPolicyService()
    : SystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netPolicyFile_ = (std::make_unique<NetPolicyFile>()).release();
    netPolicyTraffic_ = (std::make_unique<NetPolicyTraffic>(netPolicyFile_)).release();
    netPolicyFirewall_ = (std::make_unique<NetPolicyFirewall>(netPolicyFile_)).release();
    netPolicyCallback_ = (std::make_unique<NetPolicyCallback>()).release();
}

NetPolicyService::~NetPolicyService() {}

void NetPolicyService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("NetPolicyService::OnStart begin timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
    if (state_ == STATE_RUNNING) {
        return;
    }
    if (!Init()) {
        NETMGR_LOG_E("init failed");
        return;
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, NULL);
    NETMGR_LOG_D("NetPolicyService::OnStart end timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
}

void NetPolicyService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetPolicyService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOG_E("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetPolicyService>::GetInstance().get())) {
            NETMGR_LOG_E("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }

    bool error = netPolicyFile_->InitPolicy();
    if (!error) {
        NETMGR_LOG_E("InitPolicyTraffic failed");
        return false;
    }
    serviceComm_ = (std::make_unique<NetPolicyServiceCommon>()).release();
    NetManagerCenter::GetInstance().RegisterPolicyService(serviceComm_);
    return true;
}

NetPolicyResultCode NetPolicyService::SetPolicyByUid(uint32_t uid, NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NetPolicyResultCode ret = NetPolicyResultCode::ERR_INTERNAL_ERROR;
    NETMGR_LOG_I("SetPolicyByUid info: uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    /* delete uid policy */
    if (policy == NetUidPolicy::NET_POLICY_NONE) {
        ret = netPolicyTraffic_->DeletePolicyByUid(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
        return ret;
    }

    /* update policy */
    if (!netPolicyFile_->IsUidPolicyExist(uid)) {
        ret = netPolicyTraffic_->AddPolicyByUid(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    } else {
        ret = netPolicyTraffic_->SetPolicyByUid(uid, policy);
        if (ret == NetPolicyResultCode::ERR_NONE) {
            netPolicyCallback_->NotifyNetUidPolicyChanged(uid, policy);
        }
    }

    return ret;
}

NetUidPolicy NetPolicyService::GetPolicyByUid(uint32_t uid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetPolicyByUid info: uid[%{public}d]", uid);
    return netPolicyFile_->GetPolicyByUid(uid);
}

std::vector<uint32_t> NetPolicyService::GetUidsByPolicy(NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetUidsByPolicy info: policy[%{public}d]", static_cast<uint32_t>(policy));
    std::vector<uint32_t> uids;
    if (!netPolicyFile_->GetUidsByPolicy(policy, uids)) {
        NETMGR_LOG_E("GetUidsByPolicy  failed");
    };

    return uids;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, bool metered)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("IsUidNetAccess info: uid[%{public}d]", uid);

    if (NetSettings::GetInstance().IsSystem(uid)) {
        return true;
    }

    NetUidPolicy uidPolicy = netPolicyFile_->GetPolicyByUid(uid);
    if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_ALL)) {
        return false;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_ALL)) {
        return true;
    }

    if (!metered) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED)) {
        return false;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED) ||
        (static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED)) {
        return true;
    }

    if (NetSettings::GetInstance().IsUidForeground(uid)) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_REJECT_METERED_BACKGROUND)) {
        return false;
    } else if (netPolicyFile_->GetBackgroundPolicy()) {
        return true;
    } else if ((static_cast<uint32_t>(uidPolicy) &
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND)) ==
        static_cast<uint32_t>(NetUidPolicy::NET_POLICY_ALLOW_METERED_BACKGROUND)) {
        return true;
    }

    return false;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    bool metered = netPolicyFile_->IsInterfaceMetered(ifaceName);

    return IsUidNetAccess(uid, metered);
}

int32_t NetPolicyService::RegisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        NETMGR_LOG_E("RegisterNetPolicyCallback parameter callback is null");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    netPolicyCallback_->RegisterNetPolicyCallback(callback);

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

int32_t NetPolicyService::UnregisterNetPolicyCallback(const sptr<INetPolicyCallback> &callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callback == nullptr) {
        NETMGR_LOG_E("UnregisterNetPolicyCallback parameter callback is null");
        return static_cast<int32_t>(NetPolicyResultCode::ERR_INTERNAL_ERROR);
    }

    netPolicyCallback_->UnregisterNetPolicyCallback(callback);

    return static_cast<int32_t>(NetPolicyResultCode::ERR_NONE);
}

NetPolicyResultCode NetPolicyService::SetNetPolicies(const std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetNetPolicies quotaPolicySize[%{public}zd]", quotaPolicies.size());
    if (quotaPolicies.empty()) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return netPolicyTraffic_->SetNetPolicies(quotaPolicies, netPolicyCallback_);
}

NetPolicyResultCode NetPolicyService::GetNetPolicies(std::vector<NetPolicyQuotaPolicy> &quotaPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetNetPolicies begin");
    return netPolicyFile_->GetNetPolicies(quotaPolicies);
}

NetPolicyResultCode NetPolicyService::SetCellularPolicies(const std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetCellularPolicies cellularPolicies[%{public}zd]", cellularPolicies.size());
    if (cellularPolicies.empty()) {
        NETMGR_LOG_E("cellularPolicies size 0");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    NetPolicyResultCode ret = netPolicyTraffic_->SetCellularPolicies(cellularPolicies, netPolicyCallback_);
    if (ret == NetPolicyResultCode::ERR_NONE) {
        netPolicyCallback_->NotifyNetCellularPolicyChanged(cellularPolicies);
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyService::GetCellularPolicies(std::vector<NetPolicyCellularPolicy> &cellularPolicies)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCellularPolicies begin");
    return netPolicyFile_->GetCellularPolicies(cellularPolicies);
}

NetPolicyResultCode NetPolicyService::ResetFactory(const std::string &subscriberId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("ResetFactory begin");
    netPolicyTraffic_->ClearIdleTrustList();
    return netPolicyFile_->ResetFactory(subscriberId);
}

NetPolicyResultCode NetPolicyService::SetBackgroundPolicy(bool backgroundPolicy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetBackgroundPolicy begin");

    bool oldBackgroundPolicy = netPolicyFile_->GetBackgroundPolicy();
    NetPolicyResultCode ret = netPolicyFile_->SetBackgroundPolicy(backgroundPolicy);
    if (ret != NetPolicyResultCode::ERR_NONE) {
        return ret;
    }

    bool newBackgroundPolicy = netPolicyFile_->GetBackgroundPolicy();
    if (newBackgroundPolicy != oldBackgroundPolicy) {
        netPolicyCallback_->NotifyNetBackgroundPolicyChanged(newBackgroundPolicy);
    }

    return ret;
}

bool NetPolicyService::GetBackgroundPolicy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetBackgroundPolicy begin");
    return netPolicyFile_->GetBackgroundPolicy();
}

bool NetPolicyService::GetBackgroundPolicyByUid(uint32_t uid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetBackgroundPolicyByUid begin");
    return netPolicyFirewall_->GetBackgroundPolicyByUid(uid);
}

NetBackgroundPolicy NetPolicyService::GetCurrentBackgroundPolicy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetCurrentBackgroundPolicy begin");
    return netPolicyFirewall_->GetCurrentBackgroundPolicy();
}

NetPolicyResultCode NetPolicyService::SetSnoozePolicy(const NetPolicyQuotaPolicy &quotaPolicy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetSnoozePolicy begin");

    return netPolicyTraffic_->SetSnoozePolicy(quotaPolicy, netPolicyCallback_);
}

NetPolicyResultCode NetPolicyService::SetIdleTrustlist(uint32_t uid, bool isTrustlist)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("SetIdleTrustlist info: uid[%{public}d] isTrustlist[%{public}d]", uid,
        static_cast<uint32_t>(isTrustlist));

    return netPolicyTraffic_->SetIdleTrustlist(uid, isTrustlist);
}

NetPolicyResultCode NetPolicyService::GetIdleTrustlist(std::vector<uint32_t> &uids)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOG_I("GetIdleTrustlist begin");
    return netPolicyTraffic_->GetIdleTrustlist(uids);
}
} // namespace NetManagerStandard
} // namespace OHOS
