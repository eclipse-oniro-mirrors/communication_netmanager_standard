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

#include "net_conn_client.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnClient::NetConnClient() : NetConnService_(nullptr), deathRecipient_(nullptr) {}

NetConnClient::~NetConnClient() {}

int32_t NetConnClient::SystemReady()
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->SystemReady();
}

int32_t NetConnClient::RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->RegisterNetSupplier(netType, ident, netCapabilities);
}

int32_t NetConnClient::UnregisterNetSupplier(uint32_t supplierId)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UnregisterNetSupplier(supplierId);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("The parameter of proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->RegisterNetConnCallback(callback);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("The parameter of proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->RegisterNetConnCallback(netSpecifier, callback);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UnregisterNetConnCallback(callback);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UnregisterNetConnCallback(netSpecifier, callback);
}

int32_t NetConnClient::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UpdateNetSupplierInfo(supplierId, netSupplierInfo);
}

int32_t NetConnClient::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UpdateNetCapabilities(supplierId, netCapabilities);
}

int32_t NetConnClient::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    sptr<INetConnService> proxy = GetProxy();
    if (proxy == nullptr) {
        NETMGR_LOGE("proxy is nullptr");
        return IPC_PROXY_ERR;
    }
    return proxy->UpdateNetLinkInfo(supplierId, netLinkInfo);
}

sptr<INetConnService> NetConnClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (NetConnService_) {
        NETMGR_LOGI("get proxy is ok");
        return NetConnService_;
    }
    NETMGR_LOGI("execute GetSystemAbilityManager");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        NETMGR_LOGE("GetProxy, get SystemAbilityManager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = sam->CheckSystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID);
    if (remote == nullptr) {
        NETMGR_LOGE("get Remote service failed");
        return nullptr;
    }
    deathRecipient_ = (std::make_unique<NetConnDeathRecipient>(*this)).release();
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipient_))) {
        NETMGR_LOGE("add death recipient failed");
        return nullptr;
    }
    NetConnService_ = iface_cast<INetConnService>(remote);
    if (NetConnService_ == nullptr) {
        NETMGR_LOGE("get Remote service proxy failed");
        return nullptr;
    }
    return NetConnService_;
}

void NetConnClient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    NETMGR_LOGI("on remote died");
    if (remote == nullptr) {
        NETMGR_LOGE("remote object is nullptr");
        return;
    }
    std::lock_guard lock(mutex_);
    if (NetConnService_ == nullptr) {
        NETMGR_LOGE("OnRemoteDied NetConnService_ is nullptr");
        return;
    }
    sptr<IRemoteObject> local = NetConnService_->AsObject();
    if (local != remote.promote()) {
        NETMGR_LOGE("OnRemoteDied proxy and stub is not same remote object");
        return;
    }
    local->RemoveDeathRecipient(deathRecipient_);
    NetConnService_ = nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS
