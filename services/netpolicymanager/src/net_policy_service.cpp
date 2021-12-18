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

#include "system_ability_definition.h"

#include "net_policy_constants.h"
#include "net_policy_define.h"
#include "net_policy_file.h"
#include "net_policy_traffic.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetPolicyService>::GetInstance().get());

NetPolicyService::NetPolicyService()
    : SystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID, true), registerToService_(false), state_(STATE_STOPPED)
{
    netPolicyFile_ = (std::make_unique<NetPolicyFile>()).release();
    netPolicyTraffic_ = (std::make_unique<NetPolicyTraffic>(netPolicyFile_)).release();
}

NetPolicyService::~NetPolicyService() {}

void NetPolicyService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        return;
    }
    if (!Init()) {
        NETMGR_LOGE("init failed");
        return;
    }
    state_ = STATE_RUNNING;
}

void NetPolicyService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetPolicyService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOGE("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetPolicyService>::GetInstance().get())) {
            NETMGR_LOGE("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }

    bool error = netPolicyFile_->InitPolicy();
    if (!error) {
        NETMGR_LOGE("InitPolicyTraffic failed");
        return false;
    }

    return true;
}

NetPolicyResultCode NetPolicyService::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("SetUidPolicy info: uid[%{public}d] policy[%{public}d]", uid, static_cast<uint32_t>(policy));
    /* delete uid policy */
    if (policy == NetUidPolicy::NET_POLICY_NONE) {
        return netPolicyTraffic_->DeleteUidPolicy(uid, policy);
    }

    /* update policy */
    if (!netPolicyFile_->IsUidPolicyExist(uid)) {
        return netPolicyTraffic_->AddUidPolicy(uid, policy);
    } else {
        return netPolicyTraffic_->SetUidPolicy(uid, policy);
    }
}

NetUidPolicy NetPolicyService::GetUidPolicy(uint32_t uid)
{
    NETMGR_LOGI("GetUidPolicy info: uid[%{public}d]", uid);
    return netPolicyFile_->GetUidPolicy(uid);
}

std::vector<uint32_t> NetPolicyService::GetUids(NetUidPolicy policy)
{
    NETMGR_LOGI("GetUids info: policy[%{public}d]", static_cast<uint32_t>(policy));
    std::vector<uint32_t> uids;
    if (!netPolicyFile_->GetUids(policy, uids)) {
        NETMGR_LOGE("GetUids  failed");
    };
    return uids;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, bool metered)
{
    NETMGR_LOGI("IsUidNetAccess info: uid[%{public}d]", uid);
    NetUidPolicy uidPolicy = netPolicyFile_->GetUidPolicy(uid);
    if (uidPolicy == NetUidPolicy::NET_POLICY_NONE) {
        return true;
    }

    NETMGR_LOGI("IsUidNetAccess info: policy[%{public}d]", static_cast<uint32_t>(uidPolicy));
    if (static_cast<uint32_t>(uidPolicy) & NET_POLICY_ALLOW_MASK) {
        return true;
    }

    return false;
}

bool NetPolicyService::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    NETMGR_LOGI("IsUidNetAccess info: uid[%{public}d] ifaceName[%{public}s]", uid, ifaceName.c_str());
    NetUidPolicy uidPolicy = netPolicyFile_->GetUidPolicy(uid);
    if (uidPolicy == NetUidPolicy::NET_POLICY_NONE) {
        return true;
    }

    if (static_cast<uint32_t>(uidPolicy) & NET_POLICY_ALLOW_MASK) {
        return true;
    }

    return false;
}
} // namespace NetManagerStandard
} // namespace OHOS
