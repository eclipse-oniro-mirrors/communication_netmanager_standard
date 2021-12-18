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
#include "net_policy_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyClient::NetPolicyClient() : netPolicyService_(nullptr), deathRecipient_(nullptr) {}

NetPolicyClient::~NetPolicyClient() {}

NetPolicyResultCode NetPolicyClient::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }
    return proxy->SetUidPolicy(uid, policy);
}

NetUidPolicy NetPolicyClient::GetUidPolicy(uint32_t uid)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return NetUidPolicy::NET_POLICY_NONE;
    }
    return proxy->GetUidPolicy(uid);
}

std::vector<uint32_t> NetPolicyClient::GetUids(NetUidPolicy policy)
{
    std::vector<uint32_t> uids;
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return uids;
    }
    uids = proxy->GetUids(policy);
    return uids;
}

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, bool metered)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return false;
    }
    return proxy->IsUidNetAccess(uid, metered);
}

bool NetPolicyClient::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    sptr<INetPolicyService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return false;
    }
    return proxy->IsUidNetAccess(uid, ifaceName);
}

sptr<INetPolicyService> NetPolicyClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (netPolicyService_ != nullptr) {
        return netPolicyService_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOGE("GetProxy, get SystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_POLICY_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOGE("get Remote service failed");
        return nullptr;
    }

    deathRecipient_ = (std::make_unique<NetPolicyDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOGE("add death recipient failed");
        return nullptr;
    }

    netPolicyService_ = iface_cast<INetPolicyService>(remote);
    if (netPolicyService_ == nullptr) {
        NETMGR_LOGE("get Remote service proxy failed");
        return nullptr;
    }
    return netPolicyService_;
}

void NetPolicyClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOGI("on remote died");
    if (remote == nullptr) {
        NETMGR_LOGE("remote object is nullptr");
        return;
    }

    std::lock_guard lock(mutex_);
    if (netPolicyService_ == nullptr) {
        NETMGR_LOGE("netPolicyService_ is nullptr");
        return;
    }

    sptr<IRemoteObject> local = netPolicyService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOGE("proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    netPolicyService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS
