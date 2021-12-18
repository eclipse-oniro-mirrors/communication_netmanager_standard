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

#ifndef ETHERNET_MANAGEMENT_H
#define ETHERNET_MANAGEMENT_H

#include <map>
#include <mutex>

#include "iservice_registry.h"
#include "i_dhcp_result_notify.h"
#include "dhcp_service.h"
#include "system_ability_definition.h"

#include "dev_interface_state.h"
#include "nlk_event_handle.h"
#include "netLink_rtnl.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t DHCP_TIMEOUT = 60;
class EthernetManagement : public NlkEventHandle {
public:
    class EthDhcpResultNotify : public OHOS::Wifi::IDhcpResultNotify {
    public:
        explicit EthDhcpResultNotify(EthernetManagement &ethernetManagement);
        ~EthDhcpResultNotify() override;
        void OnSuccess(int status, const std::string &ifname, OHOS::Wifi::DhcpResult &result) override;
        void OnFailed(int status, const std::string &ifname, const std::string &reason) override;
        void OnSerExitNotify(const std::string& ifname) override;

    private:
        EthernetManagement &ethernetManagement_;
    };

public:
    EthernetManagement();
    ~EthernetManagement();
    void Init();
    void UpdateInterfaceState(const std::string &dev, bool up, bool lowerUp);
    int32_t UpdateDevInterfaceState(const std::string &iface, sptr<InterfaceConfiguration> cfg);
    int32_t UpdateDevInterfaceLinkInfo(const std::string &iface, const OHOS::Wifi::DhcpResult &result);
    sptr<InterfaceConfiguration> GetDevInterfaceCfg(const std::string &iface);
    int32_t IsActivate(const std::string &iface);
    std::vector<std::string> GetActivateInterfaces();
    void RegisterNlk(NetLinkRtnl &nlk);
    void Handle(const struct NlkEventInfo &info) override;

private:
    sptr<INetConnService> netConnService_ = nullptr;
    std::map<std::string, sptr<DevInterfaceState>> devs_;
    std::unique_ptr<OHOS::Wifi::IDhcpService> dhcpService_ = nullptr;
    std::unique_ptr<EthDhcpResultNotify> dhcpResultNotify_ = nullptr;
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_MANAGEMENT_H