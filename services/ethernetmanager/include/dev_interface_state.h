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

#ifndef DEV_INTERFACE_CFG_H
#define DEV_INTERFACE_CFG_H

#include <string>
#include <vector>

#include "refbase.h"
#include "net_specifier.h"
#include "net_link_info.h"
#include "net_supplier_info.h"

#include "interface_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
class DevInterfaceState : public virtual RefBase {
    typedef enum {
        REGISTERED,
        UNREGISTERED,
        LINK_AVAILABLE,
        LINK_UNAVAILABLE
    } ConnLinkState;

public:
    DevInterfaceState();
    ~DevInterfaceState();
    void SetDevName(const std::string &devName);
    void SetDevHWaddr(const std::vector<uint8_t> &hwAddr);
    void SetNetCapabilities(uint64_t netCapabilities);
    void SetLinkUp(bool up);
    void SetLowerUp(bool lowerUp);
    void SetlinkInfo(sptr<NetLinkInfo> &linkInfo);
    void SetIfcfg(sptr<InterfaceConfiguration> &ifcfg);
    void SetDhcpReqState(bool dhcpReqState);
    void UpdateLinkInfo(const INetAddr &ipAddr, const INetAddr &gateWay, const INetAddr &route, const INetAddr &dns1,
        const INetAddr &dns2);
    std::string GetDevName() const;
    std::vector<uint8_t> GetHWaddr() const;
    uint64_t GetNetCapabilities() const;
    bool GetLinkUp() const;
    bool GetLowerUp() const;
    sptr<NetLinkInfo> GetLinkInfo() const;
    sptr<InterfaceConfiguration> GetIfcfg() const;
    IPSetMode GetIPSetMode() const;
    bool GetDhcpReqState() const;

    void RemoteRegisterNetSupplier();
    void RemoteUnregisterNetSupplier();
    void RemoteUpdateNetLinkInfo();
    void RemoteUpdateNetSupplierInfo();

private:
    void UpdateLinkInfo();
    void UpdateSupplierAvailable();
    void SetIpAddr();

private:
    ConnLinkState connLinkState_ = UNREGISTERED;
    uint32_t netSupplier_ = 0;
    std::string devName_;
    std::vector<uint8_t> devHWaddr_;
    bool linkUp_ = false;
    bool lowerUp_ = false;
    bool dhcpReqState_ = false;
    sptr<NetLinkInfo> linkInfo_ = nullptr;
    sptr<NetSupplierInfo> netSupplierInfo_ = nullptr;
    sptr<InterfaceConfiguration> ifcfg_ = nullptr;
    const NetworkType networkType_ = NET_TYPE_ETHERNET;
    uint64_t netCapabilities_ = NET_CAPABILITIES_INTERNET;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DEV_INTERFACE_CFG_H