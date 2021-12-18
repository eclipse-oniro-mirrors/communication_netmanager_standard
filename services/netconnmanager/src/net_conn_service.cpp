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
#include "net_conn_service.h"

#include "system_ability_definition.h"

#include "net_conn_types.h"
#include "net_service.h"
#include "net_supplier.h"
#include "netd_controller.h"
#include "net_mgr_log_wrapper.h"
#include "broadcast_manager.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<NetConnService>::GetInstance().get());

NetConnService::NetConnService()
    : SystemAbility(COMM_NET_CONN_MANAGER_SYS_ABILITY_ID, true), registerToService_(false),
      state_(STATE_STOPPED)
{
}

NetConnService::~NetConnService() {}

void NetConnService::OnStart()
{
    if (state_ == STATE_RUNNING) {
        NETMGR_LOGI("the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOGE("init failed");
        return;
    }
    state_ = STATE_RUNNING;
}

void NetConnService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool NetConnService::Init()
{
    if (!REGISTER_LOCAL_RESULT) {
        NETMGR_LOGE("Register to local sa manager failed");
        registerToService_ = false;
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<NetConnService>::GetInstance().get())) {
            NETMGR_LOGE("Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}

int32_t NetConnService::SystemReady()
{
    NETMGR_LOGI("System ready.");
    return 0;
}

int32_t NetConnService::RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities)
{
    NETMGR_LOGI("register supplier, netType[%{public}d] ident[%{public}s] netCapabilities[%{public}lld]",
        netType, ident.c_str(), netCapabilities);

    // According to netType, ident, get the supplier from the list and save the supplierId in the list
    if (netType >= NET_TYPE_MAX || netType <= NET_TYPE_UNKNOWN) {
        NETMGR_LOGE("netType parameter invalid");
        return ERR_INVALID_NETORK_TYPE;
    }

    sptr<NetSupplier> supplier = GetNetSupplierFromList(netType, ident);
    if (supplier != nullptr) {
        NETMGR_LOGI("supplier already exists.");
        return supplier->GetSupplierId();
    }

    // If there is no supplier in the list, create a supplier
    supplier = (std::make_unique<NetSupplier>(static_cast<NetworkType>(netType), ident)).release();
    if (supplier == nullptr) {
        NETMGR_LOGE("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }

    // create network
    sptr<Network> network = (std::make_unique<Network>(supplier)).release();
    if (network == nullptr) {
        NETMGR_LOGE("network is nullptr");
        return ERR_NO_NETWORK;
    }

    // create service by netCapabilities
    NetworkType type = static_cast<NetworkType>(netType);
    if (netCapabilities & NET_CAPABILITIES_INTERNET) {
        auto service = std::make_unique<NetService>(ident, type, NET_CAPABILITIES_INTERNET, network).release();
        if (service != nullptr) {
            netServices_.push_back(service);
            defaultNetService_ = service;
        }
    }

    if (netCapabilities & NET_CAPABILITIES_MMS) {
        auto service = std::make_unique<NetService>(ident, type, NET_CAPABILITIES_MMS, network).release();
        if (service != nullptr) {
            netServices_.push_back(service);
        }
    }

    // save supplier, network to list
    netSupplier_.push_back(supplier);
    networks_.push_back(network);
    NETMGR_LOGI("netSupplier_ size[%{public}d] networks_ size[%{public}d] netServices_ size[%{public}d]",
        netSupplier_.size(), networks_.size(), netServices_.size());

     // connect service
    if (defaultNetService_ != nullptr) {
        NETMGR_LOGI("service is connecting...");
        int32_t result = defaultNetService_->ServiceConnect();
        if (result != ERR_SERVICE_REQUEST_SUCCESS) {
            NETMGR_LOGE("connect service failed, errCode: %{public}X", result);
            reConnectTimer_.StartOnce(CONNECT_SERVICE_WAIT_TIME, NetConnService::ReConnectServiceTask);
        }
    }

    return supplier->GetSupplierId();
}

void NetConnService::ReConnectServiceTask()
{
    NETMGR_LOGI("defaultNetService reConnectService start");
    int32_t result = DelayedSingleton<NetConnService>::GetInstance()->ReConnectService();
    if (result != ERR_SERVICE_REQUEST_SUCCESS) {
        NETMGR_LOGE("service reconnection failed! errCode:%{public}d", result);
        return;
    }

    NETMGR_LOGI("defaultNetService reConnectService successfully!");
}

int32_t NetConnService::ReConnectService()
{
    if (defaultNetService_ == nullptr) {
        NETMGR_LOGE("default service is nullptr");
        return  ERR_SERVICE_NULL_PTR;
    }
    if (defaultNetService_->IsConnected() || defaultNetService_->IsConnecting()) {
        defaultNetService_->ServiceDisConnect();
    }
    NETMGR_LOGI("service is connecting...");
    return defaultNetService_->ServiceConnect();
}

int32_t NetConnService::UnregisterNetSupplier(uint32_t supplierId)
{
    NETMGR_LOGI("UnregisterNetSupplier supplierId[%{public}d]", supplierId);
    // Remove supplier from the list based on supplierId
    sptr<NetSupplier> supplier = GetNetSupplierFromListById(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOGE("supplier doesn't exist.");
        return ERR_NO_SUPPLIER;
    }

    sptr<Network> network = GetNetworkFromListBySupplierId(supplierId);
    if (network == nullptr) {
        NETMGR_LOGE("GetNetworkFromListBySupplierId get error, network is nullptr");
        DeleteSupplierFromListById(supplierId);
        return ERR_NO_NETWORK;
    }

    DeleteServiceFromListByNet(*network);
    DeleteNetworkFromListBySupplierId(supplierId);
    DeleteSupplierFromListById(supplierId);
    NETMGR_LOGI("netSupplier_ size[%{public}d], networks_ size[%{public}d], netServices_ size[%{public}d]",
                netSupplier_.size(), networks_.size(), netServices_.size());

    return ERR_NONE;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("The parameter callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netServices_.empty()) {
        NETMGR_LOGE("netServices_ is empty");
        return ERR_NO_ANY_NET_TYPE;
    }

    for (auto &netService : netServices_) {
        netService->RegisterNetConnCallback(callback);
    }

    return ERR_NONE;
}

int32_t NetConnService::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOGE("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netServices_.empty()) {
        NETMGR_LOGE("netServices_ is empty");
        return ERR_NO_ANY_NET_TYPE;
    }

    for (auto it = netServices_.begin(); it != netServices_.end(); ++it) {
        NetCapabilities netCapabilit = (*it)->GetNetCapability();
        if ((*it)->GetNetworkType() == netSpecifier->netType_
            && (netCapabilit & netSpecifier->netCapabilities_) == netCapabilit) {
            (*it)->RegisterNetConnCallback(callback);
            return ERR_NONE;
        }
    }

    return ERR_NET_TYPE_NOT_FOUND;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netServices_.empty()) {
        NETMGR_LOGE("netServices_ is empty");
        return ERR_NO_ANY_NET_TYPE;
    }

    for (auto &netService : netServices_) {
        netService->UnregisterNetConnCallback(callback);
    }

    return ERR_NONE;
}

int32_t NetConnService::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOGE("The parameter of netSpecifier or callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    if (netServices_.empty()) {
        NETMGR_LOGE("netServices_ is empty");
        return ERR_NO_ANY_NET_TYPE;
    }

    for (auto it = netServices_.begin(); it != netServices_.end(); ++it) {
        NetCapabilities netCapabilit = (*it)->GetNetCapability();
        if ((*it)->GetNetworkType() == netSpecifier->netType_
            && (netCapabilit & netSpecifier->netCapabilities_) == netCapabilit) {
            return (*it)->UnregisterNetConnCallback(callback);
        }
    }

    return ERR_NET_TYPE_NOT_FOUND;
}

int32_t NetConnService::UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo)
{
    NETMGR_LOGI("Update supplier info: supplierId[%{public}d]", supplierId);
    if (netSupplierInfo == nullptr) {
        NETMGR_LOGE("netSupplierInfo is nullptr");
        return ERR_INVALID_PARAMS;
    }

    NETMGR_LOGI("Update supplier info: netSupplierInfo[%{public}s]", netSupplierInfo->ToString("").c_str());

    // According to supplierId, get the supplier from the list
    sptr<NetSupplier> supplier = GetNetSupplierFromListById(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOGE("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }

    // Call NetSupplier class to update network connection status information
    sptr<Network> network = GetNetworkFromListBySupplierId(supplier->GetSupplierId());
    if (network == nullptr) {
        NETMGR_LOGE("network is nullptr");
        return ERR_NO_NETWORK;
    }
    network->UpdateNetSupplierInfo(*netSupplierInfo);

    return ERR_NONE;
}

int32_t NetConnService::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    NETMGR_LOGI("supplierId[%{public}d] netCapabilities[%{public}lld]", supplierId, netCapabilities);
    // According to supplierId, get the supplier from the list
    sptr<NetSupplier> supplier = GetNetSupplierFromListById(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOGE("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }

    // According to netId, get network from the list
    sptr<Network> network = GetNetworkFromListBySupplierId(supplier->GetSupplierId());
    if (network == nullptr) {
        NETMGR_LOGE("network is nullptr");
        return ERR_NO_NETWORK;
    }
    auto type = supplier->GetNetSupplierType();
    auto ident = supplier->GetNetSupplierIdent();
    // Create or delete network services based on the netCapabilities
    if (netCapabilities & NET_CAPABILITIES_INTERNET) {
        if (!IsServiceInList(network->GetNetId(), NET_CAPABILITIES_INTERNET)) {
            auto service = std::make_unique<NetService>(ident, type, NET_CAPABILITIES_INTERNET, network).release();
            netServices_.push_back(service);
        }
    } else {
        if (IsServiceInList(network->GetNetId(), NET_CAPABILITIES_INTERNET)) {
            DeleteServiceFromListByCap(network->GetNetId(), NET_CAPABILITIES_INTERNET);
        }
    }

    if (netCapabilities & NET_CAPABILITIES_MMS) {
        if (!IsServiceInList(network->GetNetId(), NET_CAPABILITIES_MMS)) {
            auto service = std::make_unique<NetService>(ident, type, NET_CAPABILITIES_MMS, network).release();
            netServices_.push_back(service);
        }
    } else {
        if (IsServiceInList(network->GetNetId(), NET_CAPABILITIES_MMS)) {
            DeleteServiceFromListByCap(network->GetNetId(), NET_CAPABILITIES_MMS);
        }
    }
    NETMGR_LOGI("netSupplier_ size[%{public}d], networks_ size[%{public}d], netServices_ size[%{public}d]",
                netSupplier_.size(), networks_.size(), netServices_.size());
    return ERR_NONE;
}

int32_t NetConnService::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    NETMGR_LOGI("supplierId[%{public}d]", supplierId);
    if (netLinkInfo == nullptr) {
        NETMGR_LOGE("netLinkInfo is nullptr");
        return ERR_INVALID_PARAMS;
    }

    NETMGR_LOGI("Update netlink info: netLinkInfo[%{public}s]", netLinkInfo->ToString("").c_str());
    // According to supplierId, get the supplier from the list
    sptr<NetSupplier> supplier = GetNetSupplierFromListById(supplierId);
    if (supplier == nullptr) {
        NETMGR_LOGE("supplier is nullptr");
        return ERR_NO_SUPPLIER;
    }
    // According to supplier id, get network from the list
    sptr<Network> network = GetNetworkFromListBySupplierId(supplier->GetSupplierId());
    if (network == nullptr) {
        NETMGR_LOGE("network is nullptr");
        return ERR_NO_NETWORK;
    }
    // Call Network class to update network link attribute information
    network->UpdateNetLinkInfo(*netLinkInfo);
    return ERR_NONE;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromList(
    uint32_t netType, const std::string &ident)
{
    for (auto it = netSupplier_.begin(); it != netSupplier_.end(); ++it) {
        auto supplierType = (*it)->GetNetSupplierType();
        auto supplierIdent = (*it)->GetNetSupplierIdent();
        if ((netType == supplierType) && (ident.compare(supplierIdent) == 0)) {
            return *it;
        }
    }

    NETMGR_LOGE("net supplier is nullptr");
    return nullptr;
}

sptr<NetSupplier> NetConnService::GetNetSupplierFromListById(uint32_t supplierId)
{
    for (auto it = netSupplier_.begin(); it != netSupplier_.end(); ++it) {
        auto id = (*it)->GetSupplierId();
        if (supplierId == id) {
            return *it;
        }
    }

    NETMGR_LOGE("net supplier is nullptr");
    return nullptr;
}

void NetConnService::DeleteSupplierFromListById(uint32_t supplierId)
{
    for (auto it = netSupplier_.begin(); it != netSupplier_.end(); ++it) {
        auto id = (*it)->GetSupplierId();
        if (supplierId == id) {
            netSupplier_.erase(it);
            return;
        }
    }
}

void NetConnService::DeleteNetworkFromListBySupplierId(uint32_t supplierId)
{
    for (auto it = networks_.begin(); it != networks_.end(); ++it) {
        sptr<NetSupplier> netSupplier = (*it)->GetNetSupplier();
        if ((netSupplier != nullptr) && (netSupplier->GetSupplierId() == supplierId)) {
            networks_.erase(it);
            return;
        }
    }
}

sptr<Network> NetConnService::GetNetworkFromListBySupplierId(uint32_t supplierId)
{
    for (auto it = networks_.begin(); it != networks_.end(); ++it) {
        sptr<NetSupplier> netSupplier = (*it)->GetNetSupplier();
        if ((netSupplier != nullptr) && (netSupplier->GetSupplierId() == supplierId)) {
            return *it;
        }
    }

    NETMGR_LOGE("network is nullptr");
    return nullptr;
}

void NetConnService::DeleteServiceFromListByNet(const Network &network)
{
    sptr<Network> currNetwork = nullptr;
    for (auto it = netServices_.begin(); it != netServices_.end();) {
        currNetwork = (*it)->GetNetwork();
        if (currNetwork != nullptr && *currNetwork == network) {
            netServices_.erase(it++);
        } else {
            ++it;
        }
    }
}

bool NetConnService::DeleteServiceFromListByCap(int32_t netId, const NetCapabilities &netCapability)
{
    sptr<Network> network = nullptr;
    for (auto it = netServices_.begin(); it != netServices_.end(); ++it) {
        network = (*it)->GetNetwork();
        if (network == nullptr) {
            continue;
        }
        if ((network->GetNetId() == netId) && (netCapability == (*it)->GetNetCapability())) {
            netServices_.erase(it);
            return true;
        }
    }

    return false;
}

bool NetConnService::IsServiceInList(int32_t netId, const NetCapabilities &netCapability) const
{
    sptr<Network> network = nullptr;
    for (auto it = netServices_.begin(); it != netServices_.end(); ++it) {
        network = (*it)->GetNetwork();
        if (network == nullptr) {
            continue;
        }
        if ((network->GetNetId() == netId) && (netCapability == (*it)->GetNetCapability())) {
            return true;
        }
    }

    return false;
}
} // namespace NetManagerStandard
} // namespace OHOS
