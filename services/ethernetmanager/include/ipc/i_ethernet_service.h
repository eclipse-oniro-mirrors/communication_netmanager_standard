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

#ifndef I_ETHERNET_SERVICE_H
#define I_ETHERNET_SERVICE_H

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "interface_configuration.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr int32_t GET_CFG_SUC = 1;
class IEthernetService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.IEthernetService");
    enum {
        CMD_SET_IF_CFG,
        CMD_GET_IF_CFG,
        CMD_IS_ACTIVATE,
        CMD_GET_ACTIVATE_INTERFACE,
    };

public:
    virtual int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic) = 0;
    virtual sptr<InterfaceConfiguration> GetIfaceConfig(const std::string &iface) = 0;
    virtual int32_t IsActivate(const std::string &iface) = 0;
    virtual std::vector<std::string> GetActivateInterfaces() = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_ETHERNET_SERVICE_H