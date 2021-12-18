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
#include "net_service.h"

#include "net_conn_types.h"
#include "broadcast_manager.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetService::NetService(
    const std::string &ident, NetworkType networkType, NetCapabilities netCapability, sptr<Network> &network)
    : ident_(ident), networkType_(networkType), netCapability_(netCapability), network_(network)
{}

void NetService::SetIdent(const std::string &ident)
{
    ident_ = ident;
}

void NetService::SetNetworkType(const NetworkType &networkType)
{
    networkType_ = networkType;
}

void NetService::SetNetCapability(const NetCapabilities &netCapability)
{
    netCapability_ = netCapability;
}

void NetService::SetServiceState(const ServiceState &serviceState)
{
    state_ = serviceState;
}

std::string NetService::GetIdent() const
{
    return ident_;
}

NetworkType NetService::GetNetworkType() const
{
    return networkType_;
}

NetCapabilities NetService::GetNetCapability() const
{
    return netCapability_;
}

ServiceState NetService::GetServiceState() const
{
    return state_;
}

sptr<Network> NetService::GetNetwork() const
{
    return network_;
}

int32_t NetService::ServiceConnect()
{
    NETMGR_LOGI("service connect");
    if (IsConnecting()) {
        NETMGR_LOGI("this service is connecting");
        return ERR_SERVICE_CONNECTING;
    }
    if (IsConnected()) {
        NETMGR_LOGI("this service is already connected");
        return ERR_SERVICE_CONNECTED;
    }

    // Filtering is not a custom network service type
    switch (networkType_) {
        case NET_TYPE_CELLULAR:
            break;
        case NET_TYPE_UNKNOWN:
        default:
            NETMGR_LOGE("thWe parameter networkType_[%{public}d] passed in is invalid", networkType_);
            return ERR_INVALID_NETORK_TYPE;
    }
    UpdateServiceState(SERVICE_STATE_IDLE);

    // Call network class to activate the network
    if (NetworkConnect() < 0) {
        NETMGR_LOGE("this service request network failed");

        UpdateServiceState(SERVICE_STATE_FAILURE);
        NetworkDisConnect();
        UpdateServiceState(SERVICE_STATE_DISCONNECTED);
        UpdateServiceState(SERVICE_STATE_IDLE);
        return ERR_SERVICE_REQUEST_CONNECT_FAIL;
    }
    // Update the network status after activating the network successfully
    UpdateServiceState(SERVICE_STATE_READY);
    NETMGR_LOGI("this service request network is successful");

    return ERR_SERVICE_REQUEST_SUCCESS;
}

int32_t NetService::ServiceDisConnect()
{
    NETMGR_LOGI("service disconnect");
    if (state_ == SERVICE_STATE_DISCONNECTING) {
        NETMGR_LOGI("this service is disconnecting");
        return ERR_SERVICE_DISCONNECTING;
    }
    if (state_ == SERVICE_STATE_DISCONNECTED) {
        NETMGR_LOGI("this service has been disconnected");
        return ERR_SERVICE_DISCONNECTED_SUCCESS;
    }

    UpdateServiceState(SERVICE_STATE_DISCONNECTING);
    NETMGR_LOGI("NetworkDisConnect start");
    // Call network class to deactivate the network
    if (NetworkDisConnect() < 0) {
        NETMGR_LOGE("this service failed to disconnect");
        UpdateServiceState(SERVICE_STATE_FAILURE);
        return ERR_SERVICE_DISCONNECTED_FAIL;
    }
    // Update the network status after deactivating the network successfully
    UpdateServiceState(SERVICE_STATE_DISCONNECTED);
    UpdateServiceState(SERVICE_STATE_IDLE);
    NETMGR_LOGI("this service successfully disconnected");

    return ERR_SERVICE_DISCONNECTED_SUCCESS;
}

int32_t NetService::ServiceAutoConnect()
{
    NETMGR_LOGI("service auto connect");
    return ServiceConnect();
}

int32_t NetService::NetworkConnect()
{
    int32_t retCode = -1;

    UpdateServiceState(SERVICE_STATE_CONNECTING);
    NETMGR_LOGI("execute NetworkConnect()");
    // Call Network class activate the network
    if (network_->NetworkConnect(netCapability_)) {
        retCode = 0;
    }
    NETMGR_LOGI("connect is [%{public}s]", (retCode > -1) ? "successfull" : "failed");
    return retCode;
}

int32_t NetService::NetworkDisConnect()
{
    int32_t retCode = -1;
    // Call Network class deactivate the network
    if (network_->NetworkDisconnect(netCapability_)) {
        retCode = 0;
    }
    NETMGR_LOGI("disconnect is [%{public}s]",  (retCode > -1) ? "successfull" : "failed");
    return retCode;
}

void NetService::UpdateServiceState(ServiceState serviceState)
{
    switch (serviceState) {
        case SERVICE_STATE_IDLE:
        case SERVICE_STATE_CONNECTING:
        case SERVICE_STATE_READY:
        case SERVICE_STATE_CONNECTED:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
        case SERVICE_STATE_FAILURE:
            state_ = serviceState;
            break;
        case SERVICE_STATE_UNKNOWN:
        default:
            state_ = SERVICE_STATE_FAILURE;
            break;
    }

    BroadcastInfo info;
    // EventFwk::CommonEventSupport::COMMON_EVENT_NETMANAGER_CONNECTION_STATE_CHANGED
    info.action = "usual.event.netmanager.NETMANAGER_CONNECTION_STATE_CHANGED";
    info.data = "Net Manager Connection State Changed";
    info.code = static_cast<int32_t>(serviceState);
    info.ordered = true;
    std::string netTypeName = std::to_string(static_cast<int32_t>(networkType_));
    std::map<std::string, std::string> param = {{"NetType", netTypeName}};
    DelayedSingleton<BroadcastManager>::GetInstance()->SendBroadcast(info, param);

    sptr<NetConnCallbackInfo> netConnCallback = (std::make_unique<NetConnCallbackInfo>()).release();
    if (netConnCallback == nullptr) {
        NETMGR_LOGE("make_unique<NetConnCallbackInfo>() failed");
        return;
    }
    netConnCallback->netState_ = static_cast<int32_t>(serviceState);
    netConnCallback->netType_ = static_cast<int32_t>(networkType_);
    NotifyNetConnStateChanged(netConnCallback);

    NETMGR_LOGI("serviceState is [%{public}d]", state_);
}

bool NetService::IsConnecting() const
{
    bool isConnecting = false;

    switch (state_) {
        case SERVICE_STATE_UNKNOWN:
        case SERVICE_STATE_FAILURE:
        case SERVICE_STATE_IDLE:
            isConnecting = network_->IsNetworkConnecting();
            break;
        case SERVICE_STATE_CONNECTING:
            isConnecting = true;
            break;
        case SERVICE_STATE_READY:
        case SERVICE_STATE_CONNECTED:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
        default:
            break;
    }

    NETMGR_LOGI("isConnecting is [%{public}d]", isConnecting);
    return isConnecting;
}

bool NetService::IsConnected() const
{
    bool isConnected = false;
    switch (state_) {
        case SERVICE_STATE_UNKNOWN:
        case SERVICE_STATE_FAILURE:
        case SERVICE_STATE_IDLE:
        case SERVICE_STATE_CONNECTING:
        case SERVICE_STATE_DISCONNECTING:
        case SERVICE_STATE_DISCONNECTED:
            break;
        case SERVICE_STATE_READY:
        case SERVICE_STATE_CONNECTED:
            isConnected = true;
            break;
        default:
            break;
    }
    NETMGR_LOGI("isConnected is [%{public}d]", isConnected);
    return isConnected;
}

void NetService::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("The parameter callback is null");
        return;
    }

    for (auto iter = netConnCallback_.begin(); iter != netConnCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            NETMGR_LOGI("netConnCallback_ had this callback");
            return;
        }
    }
    netConnCallback_.emplace_back(callback);
}

int32_t NetService::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("The parameter of callback is null");
        return ERR_SERVICE_NULL_PTR;
    }

    for (auto iter = netConnCallback_.begin(); iter != netConnCallback_.end(); ++iter) {
        if (callback->AsObject().GetRefPtr() == (*iter)->AsObject().GetRefPtr()) {
            netConnCallback_.erase(iter);
            return ERR_NONE;
        }
    }

    return ERR_NO_REGISTERED;
}

int32_t NetService::NotifyNetConnStateChanged(const sptr<NetConnCallbackInfo> &info)
{
    for (const auto &callback : netConnCallback_) {
        callback->NetConnStateChanged(info);
    }

    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
