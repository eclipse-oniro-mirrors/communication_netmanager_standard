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

#ifndef NET_SUPPLIER_H
#define NET_SUPPLIER_H

#include <string>

#include "i_net_controller.h"

namespace OHOS {
namespace NetManagerStandard {
class Network;
class NetService;
class NetSupplier : public virtual RefBase {
public:
    NetSupplier();
    NetSupplier(NetworkType netSupplierType, const std::string &netSupplierIdent);
    ~NetSupplier() = default;
    bool operator==(const NetSupplier &netSupplier) const;
    NetworkType GetNetSupplierType() const;
    std::string GetNetSupplierIdent() const;
    bool SupplierConnection(NetCapabilities netCapabilities);
    bool SupplierDisconnection(NetCapabilities netCapabilities);
    void UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo);
    uint32_t GetSupplierId() const;
    bool GetConnected() const;
    bool GetAvailable() const;
    bool GetRoaming() const;
    bool GetStrength() const;
    bool GetFrequency() const;

private:
    sptr<INetController> netController_;
    NetworkType netSupplierType_;
    std::string netSupplierIdent_;
    uint32_t supplierId_ = 0;
    uint16_t frequency_ = 0x00;
    uint8_t strength_ = 0x00;
    bool connected_ = false;
    bool isAvailable_ = false; // whether the network is available
    bool isRoaming_ = false;
    const int32_t REG_OK = 1;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_SUPPLIER_H
