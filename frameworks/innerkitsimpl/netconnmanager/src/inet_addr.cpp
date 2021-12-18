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

#include "inet_addr.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool INetAddr::operator==(const INetAddr &obj) const
{
    bool out = true;
    out = out && (type_ == obj.type_);
    out = out && (family_ == obj.family_);
    out = out && (prefixlen_ == obj.prefixlen_);
    out = out && (address_ == obj.address_);
    out = out && (netMask_ == obj.netMask_);
    out = out && (hostName_ == obj.hostName_);
    return out;
}

bool INetAddr::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint8(type_)) {
        return false;
    }
    if (!parcel.WriteUint8(family_)) {
        return false;
    }
    if (!parcel.WriteUint8(prefixlen_)) {
        return false;
    }
    if (!parcel.WriteString(address_)) {
        return false;
    }
    if (!parcel.WriteString(netMask_)) {
        return false;
    }
    if (!parcel.WriteString(hostName_)) {
        return false;
    }
    return true;
}

sptr<INetAddr> INetAddr::Unmarshalling(Parcel &parcel)
{
    sptr<INetAddr> ptr = (std::make_unique<INetAddr>()).release();
    if (ptr == nullptr) {
        NETMGR_LOGE("create INetAddr failed");
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->type_)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->family_)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->prefixlen_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->address_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->netMask_)) {
        return nullptr;
    }
    if (!parcel.ReadString(ptr->hostName_)) {
        return nullptr;
    }
    return ptr;
}

bool INetAddr::Marshalling(Parcel &parcel, const sptr<INetAddr> &object)
{
    if (object == nullptr) {
        NETMGR_LOGE("INetAddr object ptr is nullptr");
        return false;
    }
    if (!parcel.WriteUint8(object->type_)) {
        return false;
    }
    if (!parcel.WriteUint8(object->family_)) {
        return false;
    }
    if (!parcel.WriteUint8(object->prefixlen_)) {
        return false;
    }
    if (!parcel.WriteString(object->address_)) {
        return false;
    }
    if (!parcel.WriteString(object->netMask_)) {
        return false;
    }
    if (!parcel.WriteString(object->hostName_)) {
        return false;
    }
    return true;
}

std::string INetAddr::ToString(const std::string tab) const
{
    std::string familyStr = std::to_string(family_);
    std::string prefixlenStr = std::to_string(prefixlen_);
    std::string jsonStr = "{";
    jsonStr += "\"family_\"";
    jsonStr += ":";
    jsonStr += "\"" + familyStr + "\",";

    jsonStr += "\"prefixlen_\"";
    jsonStr += ":";
    jsonStr += "\"" + prefixlenStr + "\",";

    jsonStr += "\"address_\"";
    jsonStr += ":";
    jsonStr += "\"" + address_ + "\",";

    jsonStr += "\"netMask_\"";
    jsonStr += ":";
    jsonStr += "\"" + netMask_ + "\"";
    jsonStr += "}";
    return jsonStr;
}
} // namespace NetManagerStandard
} // namespace OHOS
