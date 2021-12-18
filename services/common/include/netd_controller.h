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

#ifndef NETD_CONTROLLER_H
#define NETD_CONTROLLER_H

#include <string>
#include <mutex>
#ifdef NATIVE_NETD_FEATURE
#include "dnsresolv_service.h"
#include "fwmark_server.h"
#include "net_manager_native.h"
#include "netlink_manager.h"
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/route.h>
#endif

#include "net_mgr_log_wrapper.h"
#include "route.h"

namespace OHOS {
namespace NetManagerStandard {
class NetdController {
public:
    NetdController();
    ~NetdController();
    void Init();

    static NetdController *GetInstance();
    /**
     * @brief Create a physical network
     *
     * @param netId
     * @param permission Permission to create a physical network
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkCreatePhysical(int32_t netId, int32_t permission);

    /**
     * @brief Destroy the network
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkDestroy(int32_t netId);

    /**
     * @brief Add network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkAddInterface(int32_t netId, const std::string &iface);

    /**
     * @brief Delete network port device
     *
     * @param netId
     * @param iface Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkRemoveInterface(int32_t netId, const std::string &iface);

    /**
     * @brief Add route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkAddRoute(int32_t netId, const std::string &ifName, const std::string &destination,
        const std::string &nextHop);

    /**
     * @brief Remove route
     *
     * @param netId
     * @param ifName Network port device name
     * @param destination Target host ip
     * @param nextHop Next hop address
     * @return Return the return value of the netd interface call
     */
    int32_t NetworkRemoveRoute(int32_t netId, const std::string &ifName, const std::string &destination,
        const std::string &nextHop);

    /**
     * @brief Turn off the device
     *
     * @param iface Network port device name
     */
    void SetInterfaceDown(const std::string &iface);

    /**
     * @brief Turn on the device
     *
     * @param iface Network port device name
     */
    void SetInterfaceUp(const std::string &iface);

    /**
     * @brief Clear the network interface ip address
     *
     * @param ifName Network port device name
     */
    void InterfaceClearAddrs(const std::string &ifName);

    /**
     * @brief Obtain mtu from the network interface device
     *
     * @param ifName Network port device name
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceGetMtu(const std::string &ifName);

    /**
     * @brief Set mtu to network interface device
     *
     * @param ifName Network port device name
     * @param mtu
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceSetMtu(const std::string &ifName, int32_t mtu);

    /**
     * @brief Add ip address
     *
     * @param ifName Network port device name
     * @param ipAddr    ip address
     * @param prefixLength  subnet mask
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceAddAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * @brief Delete ip address
     *
     * @param ifName Network port device name
     * @param ipAddr ip address
     * @param prefixLength subnet mask
     * @return Return the return value of the netd interface call
     */
    int32_t InterfaceDelAddress(const std::string &ifName, const std::string &ipAddr, int32_t prefixLength);

    /**
     * @brief Set dns
     *
     * @param netId
     * @param baseTimeoutMsec
     * @param retryCount
     * @param servers
     * @param domains
     * @return Return the return value of the netd interface call
     */
    int32_t SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
        const std::vector<std::string> &servers, const std::vector<std::string> &domains);
    /**
     * @brief Get dns server param info
     *
     * @param netId
     * @param servers
     * @param domains
     * @param baseTimeoutMsec
     * @param retryCount
     * @return Return the return value of the netd interface call
     */
    int32_t GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
                            std::vector<std::string> &domains, uint16_t &baseTimeoutMsec, uint8_t &retryCount);

    /**
     * @brief Create dns cache before set dns
     *
     * @param netId
     * @return Return the return value for status of call
     */
    int CreateNetworkCache(uint16_t netId);

    /**
     * @brief Destory dns cache
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int DestoryNetworkCache(uint16_t netId);

    /**
     * @brief Flush dns cache
     *
     * @param netId
     * @return Return the return value of the netd interface call
     */
    int FlushNetworkCache(uint16_t netId);

    /**
     * @brief Domain name resolution Obtains the domain name address
     *
     * @param hostName
     * @param serverName
     * @param hints
     * @param res
     * @return Return the return value of the netd interface call
     */
    int GetAddrInfo(const std::string &hostName, const std::string &serverName,
        const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId);

#ifdef NATIVE_NETD_FEATURE
#else
    int AddRoute(const std::string &ip, const std::string &mask,
                    const std::string &gateWay, const std::string &devName);
#endif
private:
#ifdef NATIVE_NETD_FEATURE
    std::unique_ptr<OHOS::nmd::NetManagerNative> netdService_ = nullptr;
    std::unique_ptr<OHOS::nmd::netlink_manager> manager_ = nullptr;
    std::unique_ptr<OHOS::nmd::fwmark_server> fwmarkServer_ = nullptr;
    std::unique_ptr<OHOS::nmd::dnsresolv_service> dnsResolvService_ = nullptr;
#endif
    static NetdController *singleInstance_;
    static std::mutex mutex_;
    bool initFlag_ = false;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETD_CONTROLLER_H
