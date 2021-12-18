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
#include "net_conn_service_proxy.h"

#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnServiceProxy::NetConnServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetConnService>(impl)
{}

NetConnServiceProxy::~NetConnServiceProxy() {}

int32_t NetConnServiceProxy::SystemReady()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SYSTEM_READY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
    }
    return error;
}

int32_t NetConnServiceProxy::RegisterNetSupplier(
    uint32_t netType, const std::string &ident, uint64_t netCapabilities)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    NETMGR_LOGI("proxy netType[%{public}d], ident[%{public}s], netCapabilities[%{public}lld]",
                netType, ident.c_str(), netCapabilities);
    if (!data.WriteUint32(netType)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    if (!data.WriteString(ident)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    if (!data.WriteUint64(netCapabilities)) {
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    int32_t error = remote->SendRequest(CMD_NM_REG_NET_SUPPLIER, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::UnregisterNetSupplier(uint32_t supplierId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    NETMGR_LOGI("proxy supplierId[%{public}d]", supplierId);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_UNREG_NETWORK, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_REGISTER_NET_CONN_CALLBACK, dataParcel, replyParcel, option);
    NETMGR_LOGI("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOGE("The parameter of netSpecifier or callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(
        CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER, dataParcel, replyParcel, option);
    NETMGR_LOGI("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    if (callback == nullptr) {
        NETMGR_LOGE("The parameter of callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(CMD_NM_UNREGISTER_NET_CONN_CALLBACK, dataParcel, replyParcel, option);
    NETMGR_LOGI("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UnregisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback)
{
    if (netSpecifier == nullptr || callback == nullptr) {
        NETMGR_LOGE("The parameter of netSpecifier or callback is nullptr");
        return NET_CONN_ERR_INPUT_NULL_PTR;
    }

    MessageParcel dataParcel;
    if (!WriteInterfaceToken(dataParcel)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NET_CONN_ERR_INVALID_PARAMETER;
    }
    netSpecifier->Marshalling(dataParcel);
    dataParcel.WriteRemoteObject(callback->AsObject().GetRefPtr());

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
    }

    MessageOption option;
    MessageParcel replyParcel;
    int32_t retCode = remote->SendRequest(
        CMD_NM_UNREGISTER_NET_CONN_CALLBACK_BY_SPECIFIER, dataParcel, replyParcel, option);
    NETMGR_LOGI("SendRequest retCode:[%{public}d]", retCode);
    if (retCode != NET_CONN_SUCCESS) {
        return retCode;
    }
    return replyParcel.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetSupplierInfo(uint32_t supplierId,
    const sptr<NetSupplierInfo> &netSupplierInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    NETMGR_LOGI("proxy supplierId[%{public}d]", supplierId);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOGI("proxy supplierId[%{public}d] Marshalling success", supplierId);
    if (!netSupplierInfo->Marshalling(data)) {
        NETMGR_LOGE("proxy Marshalling failed");
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOGI("proxy Marshalling success");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_SUPPLIER_INFO, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetCapabilities(uint32_t supplierId, uint64_t netCapabilities)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    NETMGR_LOGI("proxy supplierId[%{public}d], netCapabilities[%{public}lld]", supplierId, netCapabilities);
    if (!data.WriteUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteUint64(netCapabilities)) {
        return ERR_FLATTEN_OBJECT;
    }
    NETMGR_LOGI("proxy Marshalling success");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_CAPABILTITES, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t NetConnServiceProxy::UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    if (!data.WriteUint32(supplierId)) {
        return IPC_PROXY_ERR;
    }

    if (!netLinkInfo->Marshalling(data)) {
        NETMGR_LOGE("proxy Marshalling failed");
        return IPC_PROXY_ERR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return ERR_NULL_OBJECT;
    }
    int32_t error = remote->SendRequest(CMD_NM_SET_NET_LINK_INFO, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return error;
    }

    return reply.ReadInt32();
}

bool NetConnServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetConnServiceProxy::GetDescriptor())) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
