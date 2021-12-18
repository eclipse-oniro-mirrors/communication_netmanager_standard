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
#include "net_conn_service_stub.h"

#include "net_conn_types.h"
#include "net_conn_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetConnServiceStub::NetConnServiceStub()
{
    memberFuncMap_[CMD_NM_SYSTEM_READY]                 = &NetConnServiceStub::OnSystemReady;
    memberFuncMap_[CMD_NM_REGISTER_NET_CONN_CALLBACK]   = &NetConnServiceStub::OnRegisterNetConnCallback;
    memberFuncMap_[CMD_NM_REGISTER_NET_CONN_CALLBACK_BY_SPECIFIER] =
        &NetConnServiceStub::OnRegisterNetConnCallbackBySpecifier;
    memberFuncMap_[CMD_NM_UNREGISTER_NET_CONN_CALLBACK] = &NetConnServiceStub::OnUnregisterNetConnCallback;
        memberFuncMap_[CMD_NM_UNREGISTER_NET_CONN_CALLBACK_BY_SPECIFIER] =
        &NetConnServiceStub::OnUnregisterNetConnCallbackBySpecifier;
    memberFuncMap_[CMD_NM_REG_NET_SUPPLIER]             = &NetConnServiceStub::OnRegisterNetSupplier;
    memberFuncMap_[CMD_NM_UNREG_NETWORK]                = &NetConnServiceStub::OnUnregisterNetSupplier;
    memberFuncMap_[CMD_NM_SET_NET_SUPPLIER_INFO]        = &NetConnServiceStub::OnUpdateNetSupplierInfo;
    memberFuncMap_[CMD_NM_SET_NET_CAPABILTITES]         = &NetConnServiceStub::OnUpdateNetCapabilities;
    memberFuncMap_[CMD_NM_SET_NET_LINK_INFO]            = &NetConnServiceStub::OnUpdateNetLinkInfo;
}

NetConnServiceStub::~NetConnServiceStub() {}

int32_t NetConnServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_LOGI("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = NetConnServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_LOGE("descriptor checked fail");
        return ERR_FLATTEN_OBJECT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETMGR_LOGI("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetConnServiceStub::OnSystemReady(MessageParcel &data, MessageParcel &reply)
{
    SystemReady();
    return ERR_NONE;
}

int32_t NetConnServiceStub::OnRegisterNetSupplier(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_LOGI("stub processing");
    uint32_t netType;
    std::string ident;
    uint64_t netCapabilities;
    if (!data.ReadUint32(netType)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadString(ident)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadUint64(netCapabilities)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = RegisterNetSupplier(netType, ident, netCapabilities);
    if (ret == ERR_NO_NETWORK || ret == ERR_NO_SUPPLIER) {
        NETMGR_LOGE("Register network supplier failed, error code:[%{public}d].", ret);
        ret = NET_CONN_ERR_INVALID_SUPPLIER_ID;
    }
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetConnServiceStub::OnUnregisterNetSupplier(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;
    if (!data.ReadUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret = UnregisterNetSupplier(supplierId);
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetConnServiceStub::OnRegisterNetConnCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = NET_CONN_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOGE("Callback ptr is nullptr.");
        result = NET_CONN_ERR_GET_REMOTE_OBJECT_FAILED;
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NET_CONN_ERR_INPUT_NULL_PTR;
        reply.WriteInt32(result);
        return result;
    }

    result = ConvertCode(RegisterNetConnCallback(callback));
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnRegisterNetConnCallbackBySpecifier(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetSpecifier> netSpecifier = NetSpecifier::Unmarshalling(data);

    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOGE("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NET_CONN_ERR_INPUT_NULL_PTR;
        reply.WriteInt32(result);
        return result;
    }

    result = ConvertCode(RegisterNetConnCallback(netSpecifier, callback));
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUnregisterNetConnCallback(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOGE("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NET_CONN_ERR_INPUT_NULL_PTR;
        reply.WriteInt32(result);
        return result;
    }

    result = ConvertCode(UnregisterNetConnCallback(callback));
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUnregisterNetConnCallbackBySpecifier(MessageParcel &data, MessageParcel &reply)
{
    sptr<NetSpecifier> netSpecifier = NetSpecifier::Unmarshalling(data);

    int32_t result = ERR_FLATTEN_OBJECT;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        NETMGR_LOGE("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }

    sptr<INetConnCallback> callback = iface_cast<INetConnCallback>(remote);
    if (callback == nullptr) {
        result = NET_CONN_ERR_INPUT_NULL_PTR;
        reply.WriteInt32(result);
        return result;
    }

    result = ConvertCode(UnregisterNetConnCallback(netSpecifier, callback));
    reply.WriteInt32(result);
    return result;
}

int32_t NetConnServiceStub::OnUpdateNetSupplierInfo(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;
    if (!data.ReadUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }

    sptr<NetSupplierInfo> netSupplierInfo = NetSupplierInfo::Unmarshalling(data);
    int32_t ret = UpdateNetSupplierInfo(supplierId, netSupplierInfo);
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetConnServiceStub::OnUpdateNetCapabilities(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;
    uint64_t netCapabilities;

    if (!data.ReadUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadUint64(netCapabilities)) {
        return ERR_FLATTEN_OBJECT;
    }

    NETMGR_LOGI("stub execute UpdateNetCapabilities");
    int32_t ret = UpdateNetCapabilities(supplierId, netCapabilities);
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetConnServiceStub::OnUpdateNetLinkInfo(MessageParcel &data, MessageParcel &reply)
{
    uint32_t supplierId;

    if (!data.ReadUint32(supplierId)) {
        return ERR_FLATTEN_OBJECT;
    }

    sptr<NetLinkInfo> netLinkInfo = NetLinkInfo::Unmarshalling(data);

    int32_t ret = UpdateNetLinkInfo(supplierId, netLinkInfo);
    if (!reply.WriteInt32(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetConnServiceStub::ConvertCode(int32_t internalCode)
{
    switch (internalCode) {
        case static_cast<int32_t>(ERR_NONE):
            return static_cast<int32_t>(NET_CONN_SUCCESS);
        case static_cast<int32_t>(ERR_SERVICE_NULL_PTR):
            return static_cast<int32_t>(NET_CONN_ERR_INPUT_NULL_PTR);
        case static_cast<int32_t>(ERR_NET_TYPE_NOT_FOUND):
            return static_cast<int32_t>(NET_CONN_ERR_NET_TYPE_NOT_FOUND);
        case static_cast<int32_t>(ERR_NO_ANY_NET_TYPE):
            return static_cast<int32_t>(NET_CONN_ERR_NO_ANY_NET_TYPE);
        case static_cast<int32_t>(ERR_NO_REGISTERED):
            return static_cast<int32_t>(NET_CONN_ERR_NO_REGISTERED);
        default:
            break;
    }

    return static_cast<int32_t>(NET_CONN_ERR_INTERNAL_ERROR);
}
} // namespace NetManagerStandard
} // namespace OHOS
