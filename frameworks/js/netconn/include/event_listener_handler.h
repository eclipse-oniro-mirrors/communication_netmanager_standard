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

#ifndef EVENT_LISTENER_HANDLER_H
#define EVENT_LISTENER_HANDLER_H

#include <list>
#include <uv.h> // support libuv
#include "event_handler.h"
#include "event_runner.h"
#include "napi_common.h"
#include "napi_net_conn_observer.h"

namespace OHOS {
namespace NetManagerStandard {
class EventListenerHandler : public AppExecFwk::EventHandler {
public:
    EventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    ~EventListenerHandler() = default;
    int32_t AddEventListener(const EventListener &eventListener);
    int32_t RemoveEventListener(const EventListener &eventListener);
    void NetAvailableStateUpdated(EventListener &listen, std::unique_ptr<int32_t> &ev);
    void NetCapabilitiesStateUpdated(EventListener &listen, std::unique_ptr<CapabilitiesEvent> &ev);
    void NetConnectionStateUpdated(EventListener &listen, std::unique_ptr<int32_t> &ev);
    void NetLostStateUpdated(EventListener &listen, std::unique_ptr<ConnectionEvent> &ev);
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
private:
    std::list<EventListener> listenerList;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EVENT_LISTENER_HANDLER_H
