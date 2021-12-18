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

#ifndef I_NET_CONN_SERVICE_H
#define I_NET_CONN_SERVICE_H

#include <string>

#include "iremote_broker.h"

#include "net_link_info.h"
#include "net_specifier.h"
#include "net_supplier_info.h"
#include "i_net_conn_callback.h"

namespace OHOS {
namespace NetManagerStandard {
class INetConnService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetConnService");
    enum {
        CMD_NM_START,
        CMD_NM_REGISTER_NET_SUPPLIER,
        CMD_NM_SYSTEM_READY,
        CMD_NM_REGISTER_NET_CONN_CALLBACK,
        CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER,
        CMD_NM_UNREGISTER_NET_CONN_CALLBACK,
        CMD_NM_UNREGISTER_NET_CONN_CALLBACK_BY_SPECIFIER,
        CMD_NM_REG_NET_SUPPLIER,
        CMD_NM_UNREG_NETWORK,
        CMD_NM_SET_NET_SUPPLIER_INFO,
        CMD_NM_SET_NET_CAPABILTITES,
        CMD_NM_SET_NET_LINK_INFO,
        CMD_NM_END,
    };

public:
    virtual int32_t SystemReady() = 0;
    virtual int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities) = 0;
    virtual int32_t UnregisterNetSupplier(uint32_t supplierId) = 0;
    virtual int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) = 0;
    virtual int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) = 0;
    virtual int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) = 0;
    virtual int32_t UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) = 0;
    virtual int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) = 0;
    virtual int32_t UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities) = 0;
    virtual int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_CONN_SERVICE_H
