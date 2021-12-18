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
#include "net_supplier.h"

#include <atomic>

#include "net_controller_factory.h"
#include "net_service.h"
#include "net_mgr_log_wrapper.h"
#include "telephony_controller.h"

namespace OHOS {
namespace NetManagerStandard {
static std::atomic<uint32_t> g_nextNetSupplierId = 0x03EB;

NetSupplier::NetSupplier(NetworkType netSupplierType, const std::string &netSupplierIdent)
{
    sptr<INetController> netController =
        DelayedSingleton<NetControllerFactory>::GetInstance().get()->MakeNetController(netSupplierType);
    if (netController != nullptr) {
        netController_ = netController;
    }
    netSupplierType_ = netSupplierType;
    netSupplierIdent_ = netSupplierIdent;
    supplierId_ = g_nextNetSupplierId++;
}

bool NetSupplier::operator==(const NetSupplier &netSupplier) const
{
    return supplierId_ == netSupplier.supplierId_ &&
            netSupplierType_ == netSupplier.netSupplierType_ &&
            netSupplierIdent_ == netSupplier.netSupplierIdent_;
}

NetworkType NetSupplier::GetNetSupplierType() const
{
    return netSupplierType_;
}

std::string NetSupplier::GetNetSupplierIdent() const
{
    return netSupplierIdent_;
}

bool NetSupplier::SupplierConnection(NetCapabilities netCapabilities)
{
    NETMGR_LOGI("param ident[%{public}s] netCapabilities[%{public}lld]", netSupplierIdent_.c_str(),
        static_cast<uint64_t>(netCapabilities));
    if (netController_ == nullptr) {
        NETMGR_LOGE("netController_ is nullptr");
        return false;
    }
    NETMGR_LOGI("execute RequestNetwork");
    int32_t errCode = netController_->RequestNetwork(netSupplierIdent_, netCapabilities);
    NETMGR_LOGI("RequestNetwork errCode[%{public}d]", errCode);
    if (errCode == REG_OK) {
        connected_ = true;
        return true;
    }

    return false;
}

bool NetSupplier::SupplierDisconnection(NetCapabilities netCapabilities)
{
    NETMGR_LOGI("param ident_[%{public}s] netCapabilities[%{public}lld]", netSupplierIdent_.c_str(),
        static_cast<uint64_t>(netCapabilities));
    if (netController_ == nullptr) {
        NETMGR_LOGE("netController_ is nullptr");
        return false;
    }
    NETMGR_LOGI("execute ReleaseNetwork");
    int32_t errCode = netController_->ReleaseNetwork(netSupplierIdent_, netCapabilities);
    NETMGR_LOGI("ReleaseNetwork errCode[%{public}d]", errCode);
    if (errCode == REG_OK) {
        connected_ = false;
        return true;
    }
    return false;
}

void NetSupplier::UpdateNetSupplierInfo(const NetSupplierInfo &netSupplierInfo)
{
    isAvailable_ = netSupplierInfo.isAvailable_;
    isRoaming_ = netSupplierInfo.isRoaming_;
    strength_ = netSupplierInfo.strength_;
    frequency_ = netSupplierInfo.frequency_;
    NETMGR_LOGI(
        "isAvailable_[%{public}d] isRoaming_[%{public}d] strength_[%{public}d] "
        "frequency_[%{public}d]",
        isAvailable_, isRoaming_, strength_, frequency_);
}

uint32_t NetSupplier::GetSupplierId() const
{
    return supplierId_;
}

bool NetSupplier::GetConnected() const
{
    return connected_;
}

bool NetSupplier::GetAvailable() const
{
    return isAvailable_;
}

bool NetSupplier::GetRoaming() const
{
    return isRoaming_;
}

bool NetSupplier::GetStrength() const
{
    return strength_;
}

bool NetSupplier::GetFrequency() const
{
    return frequency_;
}
} // namespace NetManagerStandard
} // namespace OHOS