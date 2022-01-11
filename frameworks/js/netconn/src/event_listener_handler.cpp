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

#include "event_listener_handler.h"
#include "inner_event.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
EventListenerHandler::EventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    NETMGR_LOG_D("EventListenerHandler");
}

int32_t EventListenerHandler::AddEventListener(const EventListener &eventListener)
{
    listenerList.push_back(eventListener);
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        NETMGR_LOG_D("eventId = [%{public}d]", it->eventId);
    }
    NETMGR_LOG_D("listenerList.size = [%{public}zd]", listenerList.size());
    return 0;
}

int32_t EventListenerHandler::RemoveEventListener(const EventListener &eventListener)
{
    for (std::list<EventListener>::iterator it = listenerList.begin(); it != listenerList.end(); it++) {
        if (it->env != nullptr && it->callbackRef != nullptr) {
            napi_delete_reference(it->env, it->callbackRef);
        }
    }
    listenerList.erase(listenerList.begin(), listenerList.end());
    NETMGR_LOG_D("listenerList.size = [%{puvlic}zd]", listenerList.size());
    return 0;
}

void EventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    int32_t eventId = event->GetInnerEventId();
    NETMGR_LOG_D("eventId = %{public}d", eventId);
}

void EventListenerHandler::NetAvailableStateUpdated(EventListener &listen, std::unique_ptr<int32_t> &ev)
{
}

void EventListenerHandler::NetCapabilitiesStateUpdated(EventListener &listen,
    std::unique_ptr<CapabilitiesEvent> &ev)
{
}

void EventListenerHandler::NetConnectionStateUpdated(EventListener &listen, std::unique_ptr<int32_t> &ev)
{
}

void EventListenerHandler::NetLostStateUpdated(EventListener &listen, std::unique_ptr<ConnectionEvent> &ev)
{
}
} // namespace NetManagerStandard
} // namespace OHOS
