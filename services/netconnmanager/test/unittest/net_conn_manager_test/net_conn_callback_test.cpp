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

#include "net_conn_callback_test.h"

#include <iostream>

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnCallbackTest::NetConnCallbackTest() {}

NetConnCallbackTest::~NetConnCallbackTest() {}

void NetConnCallbackTest::NotifyAll()
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.notify_all();
}

void NetConnCallbackTest::WaitFor(int timeoutSecond)
{
    std::unique_lock<std::mutex> callbackLock(callbackMutex_);
    cv_.wait_for(callbackLock, std::chrono::seconds(timeoutSecond));
}

int32_t NetConnCallbackTest::NetConnStateChanged(const sptr<NetConnCallbackInfo> &info)
{
    if (info == nullptr) {
        NETMGR_LOGI("NetConnCallbackTest::NetConnStateChanged(), info is nullptr");
        return -1;
    }

    NETMGR_LOGI("NetConnCallbackTest::NetConnStateChanged(), netState_:[%{public}d], netType_:[%{public}d]",
        info->netState_, info->netType_);

    netState_ = info->netState_;
    NotifyAll();

    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS
