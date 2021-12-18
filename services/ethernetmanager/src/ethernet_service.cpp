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

#include "ethernet_service.h"

#include "net_mgr_log_wrapper.h"
#include "ethernet_constants.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_ETH = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<EthernetService>::GetInstance().get());

EthernetService::EthernetService()
    : SystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, true)
{
    ethManagement_ = std::make_unique<EthernetManagement>();
}

EthernetService::~EthernetService() {}

void EthernetService::OnStart()
{
    NETMGR_LOGI("EthernetService::OnStart");
    if (state_ == STATE_RUNNING) {
        NETMGR_LOGI("EthernetService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_LOGE("EthernetService init failed");
        return;
    }
    nlkRtnl_.Init();
    if (ethManagement_ != nullptr) {
        ethManagement_->Init();
        ethManagement_->RegisterNlk(nlkRtnl_);
    }
    state_ = STATE_RUNNING;
}

void EthernetService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool EthernetService::Init()
{
    if (!REGISTER_LOCAL_RESULT_ETH) {
        NETMGR_LOGE("EthernetService Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<EthernetService>::GetInstance().get())) {
            NETMGR_LOGE("EthernetService Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    NETMGR_LOGI("GetEthernetServer suc");
    return true;
}

int32_t EthernetService::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    NETMGR_LOGI("EthernetService SetIfaceConfig processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->UpdateDevInterfaceState(iface, ic);
    } else {
        return ETHERNET_ERROR;
    }
}

sptr<InterfaceConfiguration> EthernetService::GetIfaceConfig(const std::string &iface)
{
    NETMGR_LOGI("EthernetService GetIfaceConfig processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->GetDevInterfaceCfg(iface);
    } else {
        return nullptr;
    }
}

int32_t EthernetService::IsActivate(const std::string &iface)
{
    if (ethManagement_ != nullptr) {
        return ethManagement_->IsActivate(iface);
    } else {
        return ETHERNET_ERROR;
    }
}

std::vector<std::string> EthernetService::GetActivateInterfaces()
{
    if (ethManagement_ != nullptr) {
        return ethManagement_->GetActivateInterfaces();
    } else {
        return {};
    }
}
} // namespace NetManagerStandard
} // namespace OHOS