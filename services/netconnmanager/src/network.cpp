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

#include "network.h"

#include "net_id_manager.h"
#include "netd_controller.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
Network::Network(sptr<NetSupplier> &supplier) : supplier_(supplier)
{
    netId_ = DelayedSingleton<NetIdManager>::GetInstance()->ReserveNetId();
    NetdController::GetInstance()->CreateNetworkCache(netId_);
}

Network::~Network()
{
    NetdController::GetInstance()->DestoryNetworkCache(netId_);
}

bool Network::operator==(const Network &network) const
{
    return (supplier_ != nullptr && network.supplier_ != nullptr) &&
        *supplier_ == *(network.supplier_) &&
        netId_ == network.netId_;
}

bool Network::NetworkConnect(const NetCapabilities &netCapability)
{
    NETMGR_LOGI("supplier is connecting");
    if (isConnected_) {
        NETMGR_LOGI("supplier is connected");
        return true;
    }

    // Call NetSupplier class to activate the network
    NETMGR_LOGI("SupplierDisconnection processing");
    bool ret = supplier_->SupplierDisconnection(netCapability);
    if (!ret) {
        NETMGR_LOGE("connect failed");
        return ret;
    }

    isConnecting_ = true;
    isConnected_ = true;
    return ret;
}

bool Network::NetworkDisconnect(const NetCapabilities &netCapability)
{
    NETMGR_LOGI("supplier is disConnecting");
    if (!isConnecting_ && !isConnected_) {
        NETMGR_LOGI("no connecting or connected");
        return false;
    }

    // Call NetSupplier class to deactivate the network
    NETMGR_LOGI("SupplierDisconnection processing");
    bool ret = supplier_->SupplierDisconnection(netCapability);
    if (!ret) {
        NETMGR_LOGE("disconnect failed");
    }

    return ret;
}

bool Network::UpdateNetLinkInfo(const NetLinkInfo &netLinkInfo)
{
    NETMGR_LOGI("update net link information process");
    UpdateInterfaces(netLinkInfo);
    UpdateRoutes(netLinkInfo);
    UpdateDnses(netLinkInfo);
    updateMtu(netLinkInfo);
    netLinkInfo_ = netLinkInfo;
    return true;
}

int32_t Network::GetNetId() const
{
    return netId_;
}

void Network::SetIpAdress(const INetAddr &ipAdress)
{
    ipAddr_ = ipAdress;
}

void Network::SetDns(const INetAddr &dns)
{
    dns_ = dns;
}

void Network::SetRoute(const Route &route)
{
    route_ = route;
}

NetLinkInfo Network::GetNetLinkInfo() const
{
    return netLinkInfo_;
}

INetAddr Network::GetIpAdress() const
{
    return ipAddr_;
}

INetAddr Network::GetDns() const
{
    return dns_;
}

Route Network::GetRoute() const
{
    return route_;
}

sptr<NetSupplier> Network::GetNetSupplier() const
{
    return supplier_;
}

bool Network::UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo)
{
    NETMGR_LOGI("process strart");
    supplier_->UpdateNetSupplierInfo(netSupplierInfo);

    if (!isPhyNetCreated_) {
        std::string permission;
        // Create a physical network
        NetdController::GetInstance()->NetworkCreatePhysical(netId_, 0);
        isPhyNetCreated_ = true;
    }
    return true;
}

bool Network::IsNetworkConnecting() const
{
    return isConnecting_;
}

void Network::SetConnected(bool connected)
{
    isConnected_ = connected;
}

void Network::SetConnecting(bool connecting)
{
    isConnecting_ = connecting;
}

void Network::UpdateInterfaces(const NetLinkInfo &netLinkInfo)
{
    if (netLinkInfo.ifaceName_ == netLinkInfo_.ifaceName_) {
        return;
    }

    // Call netd to add and remove interface
    if (!netLinkInfo.ifaceName_.empty()) {
        NetdController::GetInstance()->NetworkAddInterface(netId_, netLinkInfo.ifaceName_);
    }
    if (!netLinkInfo_.ifaceName_.empty()) {
        NetdController::GetInstance()->NetworkRemoveInterface(netId_, netLinkInfo_.ifaceName_);
    }
}

void Network::UpdateRoutes(const NetLinkInfo &netLinkInfo)
{
    for (auto it = netLinkInfo.routeList_.begin(); it != netLinkInfo.routeList_.end(); ++it) {
        const struct Route &route = *it;
        if (std::find(netLinkInfo_.routeList_.begin(), netLinkInfo_.routeList_.end(), *it) ==
            netLinkInfo_.routeList_.end()) {
                NetdController::GetInstance()->NetworkAddRoute(
                    netId_, route.iface_, route.destination_.address_, route.gateway_.address_);
        }
    }

    for (auto it = netLinkInfo_.routeList_.begin(); it != netLinkInfo_.routeList_.end(); ++it) {
        const struct Route &route = *it;
        if (std::find(netLinkInfo.routeList_.begin(), netLinkInfo.routeList_.end(), *it) ==
            netLinkInfo.routeList_.end()) {
                NetdController::GetInstance()->NetworkRemoveRoute(
                    netId_, route.iface_, route.destination_.address_, route.gateway_.address_);
        }
    }
}

void Network::UpdateDnses(const NetLinkInfo &netLinkInfo)
{
    std::vector<std::string> servers;
    std::vector<std::string> doamains;
    for (auto it = netLinkInfo.dnsList_.begin(); it != netLinkInfo.dnsList_.end(); ++it) {
        auto dns = *it;
        servers.push_back(dns.address_);
        doamains.push_back(dns.hostName_);
    }
    // Call netd to set dns
    NetdController::GetInstance()->SetResolverConfig(netId_, 0, 1, servers, doamains);
}

void Network::updateMtu(const NetLinkInfo &netLinkInfo)
{
    if (netLinkInfo.mtu_ == netLinkInfo_.mtu_) {
        return;
    }

    NetdController::GetInstance()->InterfaceSetMtu(netLinkInfo.ifaceName_, netLinkInfo.mtu_);
}
} // namespace NetManagerStandard
} // namespace OHOS
