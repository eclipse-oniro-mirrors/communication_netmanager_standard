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

#ifndef NET_CONN_SERVICE_PROXY_H
#define NET_CONN_SERVICE_PROXY_H

#include "iremote_proxy.h"

#include "i_net_conn_service.h"

namespace OHOS {
namespace NetManagerStandard {
class NetConnServiceProxy : public IRemoteProxy<INetConnService> {
public:
    explicit NetConnServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~NetConnServiceProxy();
    int32_t SystemReady() override;
    int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities,
        uint32_t &supplierId) override;
    int32_t UnregisterNetSupplier(uint32_t supplierId) override;
    int32_t RegisterNetSupplierCallback(uint32_t supplierId, const sptr<INetSupplierCallback> &callback) override;
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) override;
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) override;
    int32_t UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;
    int32_t UpdateNetStateForTest(const sptr<NetSpecifier> &netSpecifier, int32_t netState) override;
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override;
    int32_t UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities) override;
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override;
    int32_t  ActivateNetwork(const sptr<NetSpecifier>& netSpecifier,
        const sptr<INetConnCallback>& callback, uint32_t& reqId) override;
    int32_t  DeactivateNetwork(uint32_t& reqId) override;
    int32_t  GetDefaultNet(int32_t& netId) override;
    int32_t HasDefaultNet(bool &flag) override;
    int32_t GetIfaceNameByType(uint32_t netType, const std::string &ident, std::string &ifaceName) override;
    int32_t RegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;
    int32_t UnRegisterNetDetectionCallback(int32_t netId, const sptr<INetDetectionCallback> &callback) override;
    int32_t NetDetection(int32_t netId) override;
    int32_t GetAddressesByName(const std::string &host, int32_t netId, std::vector<INetAddr> &addrList) override;
    int32_t GetAddressByName(const std::string &host, int32_t netId, INetAddr &addr) override;
    int32_t GetSpecificNet(uint32_t type, std::list<int32_t> &netIdList) override;
    int32_t GetAllNets(std::list<int32_t> &netIdList) override;
    int32_t GetSpecificUidNet(int32_t uid, int32_t &netId) override;
    int32_t GetConnectionProperties(int32_t netId, NetLinkInfo &info) override;
    int32_t GetNetCapabilities(int32_t netId, uint64_t &cap) override;
    int32_t BindSocket(int32_t socket_fd, int32_t netId) override;
    int32_t SetAirplaneMode(bool state) override;
    int32_t RestoreFactoryData() override;

private:
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<NetConnServiceProxy> delegator_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_PROXY_H
