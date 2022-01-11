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

#include "net_handler.h"

#include "net_conn_constants.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "net_mgr_log_wrapper.h"
#include "net_conn_client.h"

namespace OHOS {
namespace NetManagerStandard {
int32_t NetHandler::GetAddressesByName(const std::string& host, std::vector<INetAddr>& addrList)
{
    if (host.empty()) {
        NETMGR_LOG_E("host is empty");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    return DelayedSingleton<NetConnClient>::GetInstance()->GetAddressesByName(host, netId_, addrList);
}

int32_t NetHandler::GetAddressByName(const std::string &host, INetAddr &addr)
{
    if (host.empty()) {
        NETMGR_LOG_E("host is empty");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    return DelayedSingleton<NetConnClient>::GetInstance()->GetAddressByName(host, netId_, addr);
}

int32_t NetHandler::BindSocket(int32_t socket_fd)
{
    if (socket_fd < 0) {
        NETMGR_LOG_E("socket_fd is invalid");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    return DelayedSingleton<NetConnClient>::GetInstance()->BindSocket(socket_fd, netId_);
}
} // namespace OHOS
} // namespace NetManagerStandard