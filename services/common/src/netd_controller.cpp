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
#include "netd_controller.h"

#include "securec.h"
#ifdef NATIVE_NETD_FEATURE
#include <signal.h>
#include "net_conn_types.h"
#else
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

namespace OHOS {
namespace NetManagerStandard {
NetdController *NetdController::singleInstance_ = nullptr;
std::mutex NetdController::mutex_;

NetdController::NetdController()
{
    Init();
}

NetdController::~NetdController() {}

void NetdController::Init()
{
    NETMGR_LOGI("netd Init");
    if (initFlag_) {
        NETMGR_LOGI("netd initialization is complete");
        return;
    }
    initFlag_ = true;
#ifdef NATIVE_NETD_FEATURE
    netdService_ = std::make_unique<nmd::NetManagerNative>();
    netdService_->init();

    int32_t pid = getpid();
    manager_ = std::make_unique<OHOS::nmd::netlink_manager>(pid);
    std::thread nlManager([&] { manager_->start(); });

    fwmarkServer_ = std::make_unique<OHOS::nmd::fwmark_server>();
    std::thread fwserve([&] { fwmarkServer_->start(); });

    dnsResolvService_ = std::make_unique<OHOS::nmd::dnsresolv_service>();
    std::thread dnsresolvServe([&] { dnsResolvService_->start(); });

    nlManager.detach();
    fwserve.detach();
    dnsresolvServe.detach();
#else
    return;
#endif
}

NetdController *NetdController::GetInstance()
{
    if (singleInstance_ == nullptr) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (singleInstance_ == nullptr) {
            singleInstance_ = std::make_unique<NetdController>().release();
        }
    }

    return singleInstance_;
}

int32_t NetdController::NetworkCreatePhysical(int32_t netId, int32_t permission)
{
    NETMGR_LOGI("Create Physical network: netId[%{public}d], permission[%{public}d]", netId, permission);
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkCreatePhysical(netId, permission);
#else
    return 0;
#endif
}

int32_t NetdController::NetworkDestroy(int32_t netId)
{
    NETMGR_LOGI("Destroy network: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkDestroy(netId);
#else
    return 0;
#endif
}

int32_t NetdController::NetworkAddInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOGI("Add network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkAddInterface(netId, iface);
#else
    return 0;
#endif
}

int32_t NetdController::NetworkRemoveInterface(int32_t netId, const std::string &iface)
{
    NETMGR_LOGI("Remove network interface: netId[%{public}d], iface[%{public}s]", netId, iface.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkRemoveInterface(netId, iface);
#else
    return 0;
#endif
}

int32_t NetdController::NetworkAddRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOGI("Add Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkAddRoute(netId, ifName, destination, nextHop);
#else
    std::string mask = "0.0.0.0";
    return AddRoute(destination, mask, nextHop, ifName);
#endif
}

int32_t NetdController::NetworkRemoveRoute(int32_t netId, const std::string &ifName,
    const std::string &destination, const std::string &nextHop)
{
    NETMGR_LOGI("Remove Route: netId[%{public}d], ifName[%{public}s], destination[%{public}s], nextHop[%{public}s]",
        netId, ifName.c_str(), destination.c_str(), nextHop.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->networkRemoveRoute(netId, ifName, destination, nextHop);
#else
    return 0;
#endif
}

void NetdController::SetInterfaceDown(const std::string &iface)
{
    NETMGR_LOGI("Set interface down: iface[%{public}s]", iface.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return;
    }
    auto interfaceConfig = netdService_->interfaceGetConfig(iface);
    auto fit = std::find(interfaceConfig.flags.begin(), interfaceConfig.flags.end(), "up");
    if (fit != interfaceConfig.flags.end()) {
        interfaceConfig.flags.erase(fit);
    }
    interfaceConfig.flags.push_back("down");
    netdService_->interfaceSetConfig(interfaceConfig);
#else
    return;
#endif
}

void NetdController::SetInterfaceUp(const std::string &iface)
{
    NETMGR_LOGI("Set interface up: iface[%{public}s]", iface.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return;
    }
    auto interfaceConfig = netdService_->interfaceGetConfig(iface);
    auto fit = std::find(interfaceConfig.flags.begin(), interfaceConfig.flags.end(), "down");
    if (fit != interfaceConfig.flags.end()) {
        interfaceConfig.flags.erase(fit);
    }
    interfaceConfig.flags.push_back("up");
    netdService_->interfaceSetConfig(interfaceConfig);
#else
    return;
#endif
}

void NetdController::InterfaceClearAddrs(const std::string &ifName)
{
    NETMGR_LOGI("Clear addrs: ifName[%{public}s]", ifName.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return;
    }
    return netdService_->interfaceClearAddrs(ifName);
#else
    return;
#endif
}

int32_t NetdController::InterfaceGetMtu(const std::string &ifName)
{
    NETMGR_LOGI("Get mtu: ifName[%{public}s]", ifName.c_str());
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->interfaceGetMtu(ifName);
#else
    return 0;
#endif
}

int32_t NetdController::InterfaceSetMtu(const std::string &ifName, int32_t mtu)
{
    NETMGR_LOGI("Set mtu: ifName[%{public}s], mtu[%{public}d]", ifName.c_str(), mtu);
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->interfaceSetMtu(ifName, mtu);
#else
    return 0;
#endif
}

int32_t NetdController::InterfaceAddAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOGI("Add address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->interfaceAddAddress(ifName, ipAddr, prefixLength);
#else
    return 0;
#endif
}

int32_t NetdController::InterfaceDelAddress(const std::string &ifName,
    const std::string &ipAddr, int32_t prefixLength)
{
    NETMGR_LOGI("Delete address: ifName[%{public}s], ipAddr[%{public}s], prefixLength[%{public}d]",
        ifName.c_str(), ipAddr.c_str(), prefixLength);
#ifdef NATIVE_NETD_FEATURE
    if (netdService_ == nullptr) {
        NETMGR_LOGE("netdService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return netdService_->interfaceDelAddress(ifName, ipAddr, prefixLength);
#else
    return 0;
#endif
}

int32_t NetdController::SetResolverConfig(uint16_t netId, uint16_t baseTimeoutMsec, uint8_t retryCount,
    const std::vector<std::string> &servers, const std::vector<std::string> &domains)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("Set resolver config: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    const nmd::dnsresolver_params params = {netId, baseTimeoutMsec, retryCount, servers, domains};
    return dnsResolvService_->setResolverConfig(params);
#else
    return 0;
#endif
}

int32_t NetdController::GetResolverInfo(uint16_t netId, std::vector<std::string> &servers,
                                        std::vector<std::string> &domains,
                                        uint16_t &baseTimeoutMsec, uint8_t &retryCount)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("Get resolver config: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    nmd::dns_res_params getParam;
    int ret = dnsResolvService_->getResolverInfo(netId, servers, domains, getParam);
    baseTimeoutMsec = getParam.baseTimeoutMsec;
    retryCount = getParam.retryCount;
    return ret;
#else
    return 0;
#endif
}

int32_t NetdController::CreateNetworkCache(uint16_t netId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("create Network cache: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return dnsResolvService_->createNetworkCache(netId);
#else
    return 0;
#endif
}

int NetdController::DestoryNetworkCache(uint16_t netId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("Destory dns cache: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return dnsResolvService_->destoryNetworkCache(netId);
#else
    return 0;
#endif
}

int NetdController::FlushNetworkCache(uint16_t netId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("Destory Flush dns cache: netId[%{public}d]", netId);
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    return dnsResolvService_->flushNetworkCache(netId);
#else
    return 0;
#endif
}

int NetdController::GetAddrInfo(const std::string &hostName,
    const std::string &serverName, const struct addrinfo &hints, std::unique_ptr<addrinfo> &res, uint16_t netId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    NETMGR_LOGI("NetdController GetAddrInfo");
#ifdef NATIVE_NETD_FEATURE
    if (dnsResolvService_ == nullptr) {
        NETMGR_LOGE("NetdController GetAddrInfo dnsResolvService_ is null");
        return ERR_SERVICE_UPDATE_NET_LINK_INFO_FAIL;
    }
    struct addrinfo *addrRes = nullptr;
    int ret = dnsResolvService_->getaddrinfo(hostName.c_str(), nullptr, &hints, &addrRes, netId);
    res.reset(addrRes);
    return ret;
#else
    return 0;
#endif
}

#ifndef NATIVE_NETD_FEATURE
int NetdController::AddRoute(const std::string &ip, const std::string &mask,
    const std::string &gateWay, const std::string &devName)
{
    int fd = 0;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        NETMGR_LOGE("NetdController create socket fd[%{public}d]", fd);
        return -1;
    }
    struct sockaddr_in _sin;
    struct sockaddr_in *sin = &_sin;
    struct rtentry  rt;
    bzero(&rt, sizeof(struct rtentry));
    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if (inet_aton(gateWay.c_str(), &sin->sin_addr) < 0) {
        NETMGR_LOGE("NetdController inet_aton gateWay[%{public}s]", gateWay.c_str());
        return -1;
    }
    memcpy_s(&rt.rt_gateway, sizeof(rt.rt_gateway), sin, sizeof(struct sockaddr_in));
    (reinterpret_cast<struct sockaddr_in*>(&rt.rt_dst))->sin_family=AF_INET;
    if (inet_aton(ip.c_str(), &((struct sockaddr_in*)&rt.rt_dst)->sin_addr) < 0) {
        NETMGR_LOGE("NetdController inet_aton ip[%{public}s]", ip.c_str());
        return -1;
    }
    (reinterpret_cast<struct sockaddr_in*>(&rt.rt_genmask))->sin_family=AF_INET;
    if (inet_aton(mask.c_str(), &(reinterpret_cast<struct sockaddr_in*>(&rt.rt_genmask))->sin_addr) < 0) {
        NETMGR_LOGE("NetdController inet_aton mask[%{public}s]", mask.c_str());
        return -1;
    }
    if (!devName.empty()) {
        rt.rt_dev = (char*)devName.c_str();
    }
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt) < 0) {
        NETMGR_LOGE("NetdController ioctl error");
        return -1;
    }
    close(fd);
    return 0;
}
#endif
} // namespace NetManagerStandard
} // namespace OHOS