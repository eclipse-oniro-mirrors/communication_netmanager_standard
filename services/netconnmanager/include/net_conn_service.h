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

#ifndef NET_CONN_SERVICE_H
#define NET_CONN_SERVICE_H

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "singleton.h"
#include "system_ability.h"

#include "ipc/net_conn_service_stub.h"
#include "net_service.h"
#include "net_supplier.h"
#include "network.h"
#include "timer.h"

namespace OHOS {
namespace NetManagerStandard {
constexpr uint32_t CONNECT_SERVICE_WAIT_TIME = 6000;
class NetConnService : public SystemAbility,
                             public NetConnServiceStub,
                             public std::enable_shared_from_this<NetConnService> {
    DECLARE_DELAYED_SINGLETON(NetConnService)
    DECLARE_SYSTEM_ABILITY(NetConnService)

    using NET_SERVICE_LIST = std::list<sptr<NetService>>;
    using NET_NETWORK_LIST = std::list<sptr<Network>>;
    using NET_SUPPLIER_LIST = std::list<sptr<NetSupplier>>;

public:
    void OnStart() override;
    void OnStop() override;
     /**
     * @brief The interface in NetConnService can be called when the system is ready
     *
     * @return Returns 0, the system is ready, otherwise the system is not ready
     */
    int32_t SystemReady() override;

     /**
     * @brief The interface is register the network
     *
     * @param netType Network Type
     * @param ident Unique identification of mobile phone card
     * @param netCapabilities Network capabilities registered by the network supplier
     *
     * @return The id of the network supplier
     */
    int32_t RegisterNetSupplier(uint32_t netType, const std::string &ident, uint64_t netCapabilities) override;

    /**
     * @brief The interface is unregister the network
     *
     * @param supplierId The id of the network supplier
     *
     * @return Returns 0, unregister the network successfully, otherwise it will fail
     */
    int32_t UnregisterNetSupplier(uint32_t supplierId) override;

     /**
     * @brief Register net connection callback
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

     /**
     * @brief Register net connection callback by NetSpecifier
     *
     * @param netSpecifier specifier information
     * @param callback The callback of INetConnCallback interface
     *
     * @return Returns 0, successfully register net connection callback, otherwise it will failed
     */
    int32_t RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;

     /**
     * @brief Unregister net connection callback
     *
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     */
    int32_t UnregisterNetConnCallback(const sptr<INetConnCallback> &callback) override;

     /**
     * @brief Unregister net connection callback by NetSpecifier
     *
     * @return Returns 0, successfully unregister net connection callback, otherwise it will fail
     */
    int32_t UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
        const sptr<INetConnCallback> &callback) override;

    /**
     * @brief The interface is update network connection status information
     *
     * @param supplierId The id of the network supplier
     * @param netSupplierInfo network connection status information
     *
     * @return Returns 0, successfully update the network connection status information, otherwise it will fail
     */
    int32_t UpdateNetSupplierInfo(uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo) override;

    /**
     * @brief The interface is Create or delete network services based on the supplierId and the netCapabilities
     *
     * @param supplierId The id of the network supplier
     * @param netCapabilities Network capabilities registered by the network supplier
     *
     * @return Returns 0, successfully create network service or delete network service, otherwise fail
     */
    int32_t UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities) override;

    /**
     * @brief The interface is update network link attribute information
     *
     * @param supplierId The id of the network supplier
     * @param netLinkInfo network link attribute information
     *
     * @return Returns 0, successfully update the network link attribute information, otherwise it will fail
     */
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo) override;
    static void ReConnectServiceTask();

private:
    bool Init();
    sptr<NetSupplier> GetNetSupplierFromList(
        uint32_t netType, const std::string &ident);
    sptr<NetSupplier> GetNetSupplierFromListById(uint32_t supplierId);
    sptr<Network> GetNetworkFromListBySupplierId(uint32_t supplierId);
    void DeleteSupplierFromListById(uint32_t supplierId);
    void DeleteNetworkFromListBySupplierId(uint32_t supplierId);
    bool DeleteServiceFromListByCap(int32_t netId, const NetCapabilities &netCapability);
    void DeleteServiceFromListByNet(const Network &network);
    bool IsServiceInList(int32_t netId, const NetCapabilities &netCapability) const;
    int32_t ReConnectService();
    void ThreadExitTask();
    int32_t NotifyNetConnStateChanged(const sptr<NetConnCallbackInfo> &info);

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    bool registerToService_;
    ServiceRunningState state_;
    sptr<NetService> defaultNetService_ = nullptr;

    NET_SERVICE_LIST netServices_;
    NET_NETWORK_LIST networks_;
    NET_SUPPLIER_LIST netSupplier_;

    Timer reConnectTimer_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_CONN_SERVICE_H
