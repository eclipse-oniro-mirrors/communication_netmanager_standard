/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "napi_net_conn_observer.h"
#include "net_mgr_log_wrapper.h"
#include "event_listener_manager.h"
#include "napi_net_conn.h"
#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NapiNetConnObserver::NetAvailable(sptr<NetHandler> &netHandler)
{
    NETMGR_LOG_D("NapiNetConnObserver NetAvailable netId [%{public}d]", netHandler->GetNetId());
    std::unique_ptr<int> id = std::make_unique<int>(netHandler->GetNetId());
    bool result = EventListenerManager::GetInstance().SendEvent(EVENT_NET_AVAILABLE_CHANGE, id);
    NETMGR_LOG_D("NetAvailable result = [%{public}d]", result);
    return 0;
}

int32_t NapiNetConnObserver::NetCapabilitiesChange(sptr<NetHandler> &netHandler, const uint64_t &netCap)
{
    NETMGR_LOG_D("NapiNetConnObserver NetCapabilitiesChange netId [%{public}d], netcap [%{public}" PRIu64 "]",
        netHandler->GetNetId(), netCap);
    std::unique_ptr<CapabilitiesEvent> capabilities = std::make_unique<CapabilitiesEvent>(netHandler->GetNetId(),
        netCap);
    bool result = EventListenerManager::GetInstance().SendEvent(EVENT_NET_LOST_CHANGE, capabilities);
    NETMGR_LOG_D("NetLost result = [%{public}d]", result);
    return 0;
}

int32_t NapiNetConnObserver::NetConnectionPropertiesChange(sptr<NetHandler> &netHandler,
    const sptr<NetLinkInfo> &info)
{
    NETMGR_LOG_D("NapiNetConnObserver NetConnectionPropertiesChange netId [%{public}d], info is [%{public}s]",
        netHandler->GetNetId(), info == nullptr ? "nullptr" : "not nullptr");
    std::unique_ptr<ConnectionEvent> connection = std::make_unique<ConnectionEvent>(netHandler->GetNetId(), info);
    bool result = EventListenerManager::GetInstance().SendEvent(EVENT_NET_CONNECTION_CHANGE, connection);
    NETMGR_LOG_D("NetConnectionPropertiesChange result = [%{public}d]", result);
    return 0;
}

int32_t NapiNetConnObserver::NetLost(sptr<NetHandler> &netHandler)
{
    NETMGR_LOG_D("NapiNetConnObserver NetLost netId [%{public}d]", netHandler->GetNetId());
    std::unique_ptr<int> id = std::make_unique<int>(netHandler->GetNetId());
    bool result = EventListenerManager::GetInstance().SendEvent(EVENT_NET_LOST_CHANGE, id);
    NETMGR_LOG_D("NetLost result = [%{public}d]", result);
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
