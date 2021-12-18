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
#include "net_conn_callback_proxy.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnCallbackProxy::NetConnCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetConnCallback>(impl)
{}

NetConnCallbackProxy::~NetConnCallbackProxy() {}

int32_t NetConnCallbackProxy::NetConnStateChanged(const sptr<NetConnCallbackInfo> &info)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!info->Marshalling(data)) {
        NETMGR_LOGE("Proxy Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(NET_CONN_STATE_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        NETMGR_LOGE("Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

bool NetConnCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetConnCallbackProxy::GetDescriptor())) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
