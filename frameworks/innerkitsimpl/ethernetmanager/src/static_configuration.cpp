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

#include "static_configuration.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool StaticConfiguration::Marshalling(Parcel &parcel) const
{
    if (!ipAddr_.Marshalling(parcel)) {
        NETMGR_LOGE("write ipAddr_ to parcel failed");
        return false;
    }
    if (!route_.Marshalling(parcel)) {
        NETMGR_LOGE("write route_ to parcel failed");
        return false;
    }
    if (!gate_.Marshalling(parcel)) {
        NETMGR_LOGE("write gate_ to parcel failed");
        return false;
    }
    if (!netMask_.Marshalling(parcel)) {
        NETMGR_LOGE("write netMask_ to parcel failed");
        return false;
    }
    if (!parcel.WriteUint32(dnsServers_.size())) {
        NETMGR_LOGE("write dnsServers_ size to parcel failed");
        return false;
    }
    for (auto it = dnsServers_.begin(); it != dnsServers_.end(); ++it) {
        if (!it->Marshalling(parcel)) {
            NETMGR_LOGE("write dnsServers_ to parcel failed");
            return false;
        }
    }
    if (!parcel.WriteString(domain_)) {
        NETMGR_LOGE("write domain_ to parcel failed");
        return false;
    }
    return true;
}

sptr<StaticConfiguration> StaticConfiguration::Unmarshalling(Parcel &parcel)
{
    sptr<StaticConfiguration> ptr = (std::make_unique<StaticConfiguration>()).release();
    if (ptr == nullptr) {
        return nullptr;
    }
    sptr<INetAddr> ipAddr = INetAddr::Unmarshalling(parcel);
    if (ipAddr == nullptr) {
        NETMGR_LOGE("ipAddr_ is null");
        return nullptr;
    }
    ptr->ipAddr_ = *ipAddr;
    sptr<INetAddr> route = INetAddr::Unmarshalling(parcel);
    if (route == nullptr) {
        NETMGR_LOGE("route_ is null");
        return nullptr;
    }
    ptr->route_ = *route;
    sptr<INetAddr> gate = INetAddr::Unmarshalling(parcel);
    if (gate == nullptr) {
        NETMGR_LOGE("gate_ is null");
        return nullptr;
    }
    ptr->gate_ = *gate;
    sptr<INetAddr> netMask = INetAddr::Unmarshalling(parcel);
    if (netMask == nullptr) {
        NETMGR_LOGE("netMask_ is null");
        return nullptr;
    }
    ptr->netMask_ = *netMask;
    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return nullptr;
    }
    for (uint32_t i = 0; i < size; i++) {
        sptr<INetAddr> netAddr = INetAddr::Unmarshalling(parcel);
        if (netAddr == nullptr) {
            NETMGR_LOGE("netAddr is null");
            return nullptr;
        }
        ptr->dnsServers_.push_back(*netAddr);
    }
    if (!parcel.ReadString(ptr->domain_)) {
        return nullptr;
    }
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS