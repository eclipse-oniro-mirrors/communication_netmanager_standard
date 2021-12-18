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

#include "dev_interface_state.h"

#include "net_conn_client.h"
#include "net_mgr_log_wrapper.h"

#include "netLink_rtnl.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
DevInterfaceState::DevInterfaceState()
{
    netSupplierInfo_ = std::make_unique<NetSupplierInfo>().release();
}

DevInterfaceState::~DevInterfaceState() {}

void DevInterfaceState::SetDevName(const std::string &devName)
{
    devName_ = devName;
}

void DevInterfaceState::SetDevHWaddr(const std::vector<uint8_t> &hwAddr)
{
    devHWaddr_ = hwAddr;
}

void DevInterfaceState::SetNetCapabilities(uint64_t netCapabilities)
{
    netCapabilities_ = netCapabilities;
}

void DevInterfaceState::SetLinkUp(bool up)
{
    linkUp_ = up;
}

void DevInterfaceState::SetLowerUp(bool lowerUp)
{
    lowerUp_ = lowerUp;
}

void DevInterfaceState::SetlinkInfo(sptr<NetLinkInfo> &linkInfo)
{
    linkInfo_ = linkInfo;
}

void DevInterfaceState::SetIfcfg(sptr<InterfaceConfiguration> &ifcfg)
{
    ifcfg_ = ifcfg;
    if (ifcfg_->mode_ == STATIC) {
        UpdateLinkInfo();
        SetIpAddr();
        if (connLinkState_ == LINK_AVAILABLE) {
            RemoteUpdateNetLinkInfo();
        }
    }
}

void DevInterfaceState::SetDhcpReqState(bool dhcpReqState)
{
    dhcpReqState_ = dhcpReqState;
}

std::string DevInterfaceState::GetDevName() const
{
    return devName_;
}

std::vector<uint8_t> DevInterfaceState::GetHWaddr() const
{
    return devHWaddr_;
}

uint64_t DevInterfaceState::GetNetCapabilities() const
{
    return netCapabilities_;
}

bool DevInterfaceState::GetLinkUp() const
{
    return linkUp_;
}

bool DevInterfaceState::GetLowerUp() const
{
    return lowerUp_;
}

sptr<NetLinkInfo> DevInterfaceState::GetLinkInfo() const
{
    return linkInfo_;
}

sptr<InterfaceConfiguration> DevInterfaceState::GetIfcfg() const
{
    return ifcfg_;
}

IPSetMode DevInterfaceState::GetIPSetMode() const
{
    if (ifcfg_ == nullptr) {
        return IPSetMode::STATIC;
    }
    return ifcfg_->mode_;
}

bool DevInterfaceState::GetDhcpReqState() const
{
    return dhcpReqState_;
}

int32_t DevInterfaceState::RemoteRegisterNetSupplier()
{
    if (connLinkState_ == UNREGISTERED) {
        netSupplier_ = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetSupplier(networkType_,
            devName_, netCapabilities_);
        if (netSupplier_ > MINIMUM_SUPPLIER_ID) {
            connLinkState_ = REGISTERED;
        }
        NETMGR_LOGI("DevInterfaceCfg RemoteRegisterNetSupplier netSupplier_[%{public}d]", netSupplier_);
    }
    return netSupplier_;
}

int32_t DevInterfaceState::RemoteUnregisterNetSupplier()
{
    if (connLinkState_ == UNREGISTERED) {
        return NETMANAGER_ERROR;
    }
    int ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetSupplier(netSupplier_);
    if (!ret) {
        connLinkState_ = UNREGISTERED;
        netSupplier_ = 0;
    }
    return ret;
}

int32_t DevInterfaceState::RemoteUpdateNetLinkInfo()
{
    if (connLinkState_ == LINK_UNAVAILABLE) {
        NETMGR_LOGE("DevInterfaceCfg RemoteUpdateNetLinkInfo regState_:LINK_UNAVAILABLE");
        return NETMANAGER_ERROR;
    }
    if (linkInfo_ == nullptr) {
        NETMGR_LOGE("DevInterfaceCfg RemoteUpdateNetLinkInfo linkInfo_ is nullptr");
        return NETMANAGER_ERROR;
    }
    return DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetLinkInfo(netSupplier_, linkInfo_);
}

int32_t DevInterfaceState::RemoteUpdateNetSupplierInfo()
{
    if (connLinkState_ == UNREGISTERED) {
        NETMGR_LOGE("DevInterfaceCfg RemoteUpdateNetSupplierInfo regState_:UNREGISTERED");
        return NETMANAGER_ERROR;
    }
    if (netSupplierInfo_ == nullptr) {
        NETMGR_LOGE("DevInterfaceCfg RemoteUpdateNetSupplierInfo netSupplierInfo_ is nullptr");
        return NETMANAGER_ERROR;
    }
    UpdateSupplierAvailable();
    return DelayedSingleton<NetConnClient>::GetInstance()->UpdateNetSupplierInfo(netSupplier_, netSupplierInfo_);
}

void DevInterfaceState::UpdateLinkInfo()
{
    if (!ifcfg_ && ifcfg_->mode_ != STATIC) {
        return;
    }
    if (linkInfo_ == nullptr) {
        linkInfo_ = std::make_unique<NetLinkInfo>().release();
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    std::list<INetAddr>().swap(linkInfo_->dnsList_);
    linkInfo_->netAddrList_.push_back(ifcfg_->ipStatic_.ipAddr_);
    struct Route route;
    route.iface_ = devName_;
    route.destination_ = ifcfg_->ipStatic_.route_;
    route.gateway_ = ifcfg_->ipStatic_.gate_;
    linkInfo_->routeList_.push_back(route);
    for (auto it = ifcfg_->ipStatic_.dnsServers_.begin(); it != ifcfg_->ipStatic_.dnsServers_.end(); ++it) {
        linkInfo_->dnsList_.push_back(*it);
    }
}

void DevInterfaceState::UpdateLinkInfo(const std::string &iface, const OHOS::Wifi::DhcpResult &result)
{
    NETMGR_LOGI("DevInterfaceCfg::UpdateLinkInfo");
    if (linkInfo_ == nullptr) {
        linkInfo_ = std::make_unique<NetLinkInfo>().release();
    }
    std::list<INetAddr>().swap(linkInfo_->netAddrList_);
    std::list<Route>().swap(linkInfo_->routeList_);
    INetAddr ipAddr;
    ipAddr.type_ = result.iptype;
    ipAddr.address_ = result.strYourCli;
    linkInfo_->netAddrList_.push_back(ipAddr);
    struct Route route;
    INetAddr gate;
    INetAddr destination;
    route.iface_ = iface;
    if (result.strServer != result.strRouter1) {
        gate.address_ = result.strServer;
        if (result.strRouter1 == "*") {
            destination.address_ = "0.0.0.0";
        } else {
            destination.address_ = result.strRouter1;
        }
        route.destination_ = destination;
        route.gateway_ = gate;
        linkInfo_->routeList_.push_back(route);
    }
    if (result.strServer != result.strRouter2) {
        gate.address_ = result.strServer;
        if (result.strRouter2 == "*") {
            destination.address_ = "0.0.0.0";
        } else {
            destination.address_ = result.strRouter2;
        }
        route.destination_ = destination;
        route.gateway_ = gate;
        linkInfo_->routeList_.push_back(route);
    }
    ipAddr.address_ = result.strDns1;
    linkInfo_->dnsList_.push_back(ipAddr);
    ipAddr.address_ = result.strDns2;
    linkInfo_->dnsList_.push_back(ipAddr);
}

void DevInterfaceState::SetIpAddr()
{
    NetLinkRtnl::SetIpAddr(devName_, ifcfg_->ipStatic_.ipAddr_.address_);
}

void  DevInterfaceState::UpdateSupplierAvailable()
{
    bool isAvailable = linkUp_ & lowerUp_;
    netSupplierInfo_->isAvailable_ = isAvailable;
    if (isAvailable) {
        connLinkState_ = LINK_AVAILABLE;
    } else {
        connLinkState_ = LINK_UNAVAILABLE;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS