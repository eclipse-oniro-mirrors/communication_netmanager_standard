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

#include "event_listener_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "net_activate.h"
#include "net_conn_client.h"
#include "net_specifier.h"
#include "napi_net_conn_observer.h"
#include "napi_net_conn.h"

namespace OHOS {
namespace NetManagerStandard {
static sptr<INetConnCallback> activeCallback = std::make_unique<NapiNetConnObserver>().release();

EventListenerManager& EventListenerManager::GetInstance()
{
    NETMGR_LOG_D("EventListenerManager::GetInstance");
    static EventListenerManager instance;
    return instance;
}

EventListenerManager::EventListenerManager()
{
    std::lock_guard<std::mutex> lck(mtx_);
    NETMGR_LOG_D("EventListenerManager");
    if (eventStateRun_ == STATE_RUNNING) {
        NETMGR_LOG_D("eventListenerHandler is running");
        return;
    }
    eventLoop = AppExecFwk::EventRunner::Create("EventListenerHandler");
    if (eventLoop.get() == nullptr) {
        NETMGR_LOG_E("failed to create EventRunner");
        return;
    }
    eventListenerHandler = std::make_shared<EventListenerHandler>(eventLoop);
    if (eventListenerHandler == nullptr) {
        NETMGR_LOG_E("failed to create new eventListenerHandler");
        return;
    }
    eventLoop->Run();
    NETMGR_LOG_D("eventLoop  is running");
    eventStateRun_ = STATE_RUNNING;
}

int32_t EventListenerManager::AddEventListener(EventListener &eventListener)
{
    std::lock_guard<std::mutex> lck(mtx_);
    int32_t result = 0;
    eventListenerHandler->AddEventListener(eventListener);
    if ((eventListener.eventId == EVENT_NET_AVAILABLE_CHANGE)||
        (eventListener.eventId == EVENT_NET_CAPABILITIES_CHANGE)||
        (eventListener.eventId == EVENT_NET_CONNECTION_CHANGE)||
        (eventListener.eventId == EVENT_NET_LOST_CHANGE)) {
        NETMGR_LOG_I("AddEventListener");
    }
    return result;
}

int32_t EventListenerManager::RemoveEventListener(EventListener &eventListener)
{
    std::lock_guard<std::mutex> lck(mtx_);
    NETMGR_LOG_D("RemoveEventListener");
    int32_t result = 0;
    eventListenerHandler->RemoveEventListener(eventListener);
    if ((eventListener.eventId == EVENT_NET_AVAILABLE_CHANGE)||
        (eventListener.eventId == EVENT_NET_CAPABILITIES_CHANGE)||
        (eventListener.eventId == EVENT_NET_CONNECTION_CHANGE)||
        (eventListener.eventId == EVENT_NET_LOST_CHANGE)) {
        NETMGR_LOG_I("RemoveEventListener");
    }
    return result;
}
} // namespace NetManagerStandard
} // namespace OHOS
