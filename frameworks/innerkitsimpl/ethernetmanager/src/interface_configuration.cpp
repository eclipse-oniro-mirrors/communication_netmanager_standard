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

#include "interface_configuration.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool InterfaceConfiguration::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(mode_))) {
        return false;
    }
    if (!ipStatic_.Marshalling(parcel)) {
        NETMGR_LOGE("write ipStatic_ to parcel failed");
        return false;
    }
    return true;
}

sptr<InterfaceConfiguration> InterfaceConfiguration::Unmarshalling(Parcel &parcel)
{
    sptr<InterfaceConfiguration> ptr = (std::make_unique<InterfaceConfiguration>()).release();
    if (ptr == nullptr) {
        return nullptr;
    }
    int32_t mode = 0;
    if (!parcel.ReadInt32(mode)) {
        return nullptr;
    }
    ptr->mode_ = static_cast<IPSetMode>(mode);
    sptr<StaticConfiguration> sc = StaticConfiguration::Unmarshalling(parcel);
    if (sc == nullptr) {
        NETMGR_LOGE("sc is null");
        return nullptr;
    }
    ptr->ipStatic_ = *sc;
    return ptr;
}
} // namespace NetManagerStandard
} // namespace OHOS