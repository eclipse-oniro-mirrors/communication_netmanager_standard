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

#include "net_supplier_info.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetSupplierInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isAvailable_)) {
        return false;
    }
    if (!parcel.WriteBool(isRoaming_)) {
        return false;
    }
    if (!parcel.WriteUint8(strength_)) {
        return false;
    }
    if (!parcel.WriteUint32(frequency_)) {
        return false;
    }
    return true;
}

sptr<NetSupplierInfo> NetSupplierInfo::Unmarshalling(Parcel &parcel)
{
    sptr<NetSupplierInfo> ptr = (std::make_unique<NetSupplierInfo>()).release();
    if (ptr == nullptr) {
        NETMGR_LOGE("make_unique<NetSupplierInfo>() failed");
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isAvailable_)) {
        return nullptr;
    }
    if (!parcel.ReadBool(ptr->isRoaming_)) {
        return nullptr;
    }
    if (!parcel.ReadUint8(ptr->strength_)) {
        NETMGR_LOGE("read strength_ from parcel failed");
        return nullptr;
    }
    if (!parcel.ReadUint32(ptr->frequency_)) {
        return nullptr;
    }
    return ptr;
}

bool NetSupplierInfo::Marshalling(Parcel &parcel, const sptr<NetSupplierInfo> &object)
{
    if (object == nullptr) {
        NETMGR_LOGE("NetSupplierInfo object ptr is nullptr");
        return false;
    }
    if (!parcel.WriteBool(object->isAvailable_)) {
        return false;
    }
    if (!parcel.WriteBool(object->isRoaming_)) {
        return false;
    }
    if (!parcel.WriteUint8(object->strength_)) {
        return false;
    }
    if (!parcel.WriteUint32(object->frequency_)) {
        return false;
    }
    return true;
}

std::string NetSupplierInfo::ToString(const std::string &tab) const
{
    std::string str;
    str.append("\n");
    str.append(tab);
    str.append("[NetSupplierInfo]");

    str.append("\n");
    str.append(tab);
    str.append("isAvailable_ = ");
    str.append(std::to_string(isAvailable_));

    str.append("\n");
    str.append(tab);
    str.append("isRoaming_ = ");
    str.append(std::to_string(isRoaming_));

    str.append("\n");
    str.append(tab);
    str.append("strength_ = ");
    str.append(std::to_string(strength_));

    str.append("\n");
    str.append(tab);
    str.append("frequency_ = ");
    str.append(std::to_string(frequency_));

    return str;
}
} // namespace NetManagerStandard
} // namespace OHOS