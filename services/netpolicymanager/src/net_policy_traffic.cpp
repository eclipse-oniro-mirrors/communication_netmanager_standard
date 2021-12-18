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

#include "net_mgr_log_wrapper.h"
#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyTraffic::NetPolicyTraffic(sptr<NetPolicyFile> netPolicyFile) : netPolicyFile_(netPolicyFile)
{
}

bool NetPolicyTraffic::IsPolicyValid(NetUidPolicy &policy)
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
            NETMGR_LOGE("Invalid policy [%{public}d]", static_cast<uint32_t>(policy));
            return false;
        }
    }
}

NetPolicyResultCode NetPolicyTraffic::AddUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOGI("AddUidPolicy info:uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOGE("AddUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_ADD, uid, policy)) {
        NETMGR_LOGE("AddUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOGI("SetUidPolicy info:uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOGE("SetUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE, uid, policy)) {
        NETMGR_LOGE("SetUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}

NetPolicyResultCode NetPolicyTraffic::DeleteUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    NETMGR_LOGI("DeleteUidPolicy info:uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    if (netPolicyFile_ == nullptr) {
        NETMGR_LOGE("DeleteUidPolicy netPolicyFile is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!IsPolicyValid(policy)) {
        return NetPolicyResultCode::ERR_INVALID_POLICY;
    }

    if (!netPolicyFile_->WriteFile(NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE, uid, policy)) {
        NETMGR_LOGE("DeleteUidPolicy WriteFile failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return NetPolicyResultCode::ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS