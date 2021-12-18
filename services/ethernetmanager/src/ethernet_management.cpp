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

#include "ethernet_management.h"

#include "net_mgr_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
EthernetManagement::EthDhcpResultNotify::EthDhcpResultNotify(EthernetManagement &ethernetManagement)
    : ethernetManagement_(ethernetManagement)
{
}

EthernetManagement::EthDhcpResultNotify::~EthDhcpResultNotify() {}

void EthernetManagement::EthDhcpResultNotify::OnSuccess(int status, const std::string &ifname,
    OHOS::Wifi::DhcpResult &result)
{
    NETMGR_LOGI("Enter EthernetManagement::EthDhcpResultNotify::OnSuccess "
        "ifname=[%{public}s], iptype=[%{public}d], strYourCli=[%{public}s], "
        "strServer=[%{public}s], strSubnet=[%{public}s], strDns1=[%{public}s], "
        "strDns2=[%{public}s] strRouter1=[%{public}s] strRouter2=[%{public}s]",
        ifname.c_str(), result.iptype, result.strYourCli.c_str(), result.strServer.c_str(), result.strSubnet.c_str(),
        result.strDns1.c_str(), result.strDns2.c_str(), result.strRouter1.c_str(), result.strRouter2.c_str());
        ethernetManagement_.UpdateDevInterfaceLinkInfo(ifname, result);
    return;
}

void EthernetManagement::EthDhcpResultNotify::OnFailed(int status, const std::string &ifname, const std::string &reason)
{
    NETMGR_LOGI("Enter EthernetManagement::EthDhcpResultNotify::OnFailed");
    return;
}

void EthernetManagement::EthDhcpResultNotify::OnSerExitNotify(const std::string& ifname)
{
    NETMGR_LOGI("EthernetManagement::EthDhcpResultNotify::OnSerExitNotify");
    return;
}

EthernetManagement::EthernetManagement()
{
    dhcpService_.reset(std::make_unique<OHOS::Wifi::DhcpService>().release());
    dhcpResultNotify_.reset(std::make_unique<EthDhcpResultNotify>(*this).release());
}

EthernetManagement::~EthernetManagement() {}

void EthernetManagement::UpdateInterfaceState(const std::string &dev, bool up, bool lowerUp)
{
    NETMGR_LOGI("EthernetManagement UpdateInterfaceState dev[%{public}s] up[%{public}d] lowerUp[%{public}d]",
        dev.c_str(), up, lowerUp);
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(dev);
    if (fit == devs_.end()) {
        return;
    }
    sptr<DevInterfaceState> devState= fit->second;
    devState->SetLinkUp(up);
    devState->SetLowerUp(lowerUp);
    IPSetMode mode = devState->GetIPSetMode();
    bool dhcpReqState = devState->GetDhcpReqState();
    NETMGR_LOGI("EthernetManagement UpdateInterfaceState mode[%{public}d] dhcpReqState[%{public}d]",
        static_cast<int32_t>(mode), dhcpReqState);
    if (lowerUp) {
        devState->RemoteUpdateNetSupplierInfo();
        if (mode == DHCP && !dhcpReqState) {
            NETMGR_LOGI("EthernetManagement StartDhcpClient[%{public}s]", dev.c_str());
            if (dhcpService_ == nullptr) {
                NETMGR_LOGE("EthernetManagement::UpdateInterfaceState dhcpService_ is nullptr");
                return;
            }
            dhcpService_->StartDhcpClient(dev, false);
            if (dhcpService_->GetDhcpResult(dev, dhcpResultNotify_.get(), DHCP_TIMEOUT) != 0) {
                NETMGR_LOGE(" Dhcp connection failed.\n");
            }
            devState->SetDhcpReqState(true);
        } else {
            devState->RemoteUpdateNetLinkInfo();
        }
    } else {
        if (mode == DHCP && dhcpReqState) {
            NETMGR_LOGI("EthernetManagement StopDhcpClient[%{public}s]", dev.c_str());
            if (dhcpService_ == nullptr) {
                NETMGR_LOGI("EthernetManagement::UpdateInterfaceState dhcpService_ is nullptr");
                return;
            }
            dhcpService_->StopDhcpClient(dev, false);
            devState->SetDhcpReqState(false);
        }
        devState->RemoteUpdateNetSupplierInfo();
    }
}

int32_t EthernetManagement::UpdateDevInterfaceState(const std::string &iface, sptr<InterfaceConfiguration> cfg)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_LOGE("The iface[%{public}s] device or device information does not exist", iface.c_str());
        return ETHERNET_ERROR;
    }
    if (!fit->second->GetLinkUp()) {
        return ETHERNET_ERROR;
    }
    fit->second->SetIfcfg(cfg);
    return ETHERNET_SUCCESS;
}

int32_t EthernetManagement::UpdateDevInterfaceLinkInfo(const std::string &iface, const OHOS::Wifi::DhcpResult &result)
{
    NETMGR_LOGI("EthernetManagement::UpdateDevInterfaceLinkInfo");
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_LOGE("The iface[%{public}s] device or device information does not exist", iface.c_str());
        return ETHERNET_ERROR;
    }
    if (!fit->second->GetLinkUp()) {
        return ETHERNET_ERROR;
    }
    fit->second->UpdateLinkInfo(iface, result);
    fit->second->RemoteUpdateNetLinkInfo();
    return ETHERNET_SUCCESS;
}

sptr<InterfaceConfiguration> EthernetManagement::GetDevInterfaceCfg(const std::string &iface)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_LOGE("The iface[%{public}s] device does not exist", iface.c_str());
        return nullptr;
    }
    return fit->second->GetIfcfg();
}

int32_t EthernetManagement::IsActivate(const std::string &iface)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto fit = devs_.find(iface);
    if (fit == devs_.end() || fit->second == nullptr) {
        NETMGR_LOGE("The iface[%{public}s] device does not exist", iface.c_str());
        return ETHERNET_ERROR;
    }
    return static_cast<int32_t>(fit->second->GetLinkUp());
}

std::vector<std::string> EthernetManagement::GetActivateInterfaces()
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<std::string> a;
    for (auto it = devs_.begin(); it != devs_.end(); ++it) {
        a.push_back(it->first);
    }
    return a;
}

void EthernetManagement::RegisterNlk(NetLinkRtnl &nlk)
{
    nlk.RegisterHandle(this);
}

void EthernetManagement::Handle(const struct NlkEventInfo &info)
{
    bool up = static_cast<bool>(info.ifiFlags_ & IFF_UP);
    bool lowerUp = static_cast<bool>(info.ifiFlags_ & IFF_LOWER_UP);
    NETMGR_LOGI("EthernetManagement Handle info dev[%{public}s] up[%{public}d] lowerUp[%{public}d]",
        info.iface_.c_str(), up, lowerUp);
    UpdateInterfaceState(info.iface_, up, lowerUp);
}

void EthernetManagement::Init()
{
    std::vector<NlkEventInfo> linkInfos;
    NetLinkRtnl::GetLinkInfo(linkInfos);
    if (linkInfos.size() <= 0) {
        NETMGR_LOGE("EthernetManagement link list is empty");
        return;
    }
    NETMGR_LOGI("EthernetManagement devs size[%{public}d]", linkInfos.size());
    for (auto it = linkInfos.begin(); it != linkInfos.end(); it++) {
        std::string devName = it->iface_;
        NETMGR_LOGI("EthernetManagement devName[%{public}s]", devName.c_str());
        if (devName.empty()) {
            continue;
        }
        sptr<DevInterfaceState> devState = std::make_unique<DevInterfaceState>().release();
        devs_.insert(std::make_pair(devName, devState));
        sptr<InterfaceConfiguration> ifCfg = std::make_unique<InterfaceConfiguration>().release();
        ifCfg->mode_ = STATIC;
        devState->SetIfcfg(ifCfg);
        std::vector<uint8_t> hwAddr = NetLinkRtnl::GetHWaddr(devName);
        bool up = it->ifiFlags_ & IFF_UP;
        bool lowerUp = it->ifiFlags_ & IFF_LOWER_UP;
        devState->SetDevName(devName);
        devState->SetDevHWaddr(hwAddr);
        devState->SetLinkUp(up);
        devState->SetLowerUp(lowerUp);
        devState->RemoteRegisterNetSupplier();
        if (up && lowerUp) {
            devState->RemoteUpdateNetSupplierInfo();
        }
    }
    NETMGR_LOGI("EthernetManagement devs_ size[%{public}d", devs_.size());
}
} // namespace NetManagerStandard
} // namespace OHOS