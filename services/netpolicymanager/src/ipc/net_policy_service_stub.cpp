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
#include "net_policy_service_stub.h"

#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyServiceStub::NetPolicyServiceStub()
{
    memberFuncMap_[CMD_NSM_SET_UID_POLICY] = &NetPolicyServiceStub::OnSetUidPolicy;
    memberFuncMap_[CMD_NSM_GET_UID_POLICY] = &NetPolicyServiceStub::OnGetUidPolicy;
    memberFuncMap_[CMD_NSM_GET_UIDS] = &NetPolicyServiceStub::OnGetUids;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_METERED] = &NetPolicyServiceStub::OnIsUidNetAccessMetered;
    memberFuncMap_[CMD_NSM_IS_NET_ACCESS_IFACENAME] = &NetPolicyServiceStub::OnIsUidNetAccessIfaceName;
}

NetPolicyServiceStub::~NetPolicyServiceStub() {}

int32_t NetPolicyServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescripter = NetPolicyServiceStub::GetDescriptor();
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

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetPolicyServiceStub::OnSetUidPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t netPolicy;
    if (!data.ReadUint32(netPolicy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(SetUidPolicy(uid, static_cast<NetUidPolicy>(netPolicy))))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetUidPolicy(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(GetUidPolicy(uid)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnGetUids(MessageParcel &data, MessageParcel &reply)
{
    uint32_t policy;
    if (!data.ReadUint32(policy)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!reply.WriteUInt32Vector(GetUids(static_cast<NetUidPolicy>(policy)))) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnIsUidNetAccessMetered(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    bool metered = false;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadBool(metered)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool ret = IsUidNetAccess(uid, metered);
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}

int32_t NetPolicyServiceStub::OnIsUidNetAccessIfaceName(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uid = 0;
    std::string ifaceName;
    if (!data.ReadUint32(uid)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.ReadString(ifaceName)) {
        return ERR_FLATTEN_OBJECT;
    }

    bool ret = IsUidNetAccess(uid, ifaceName);
    if (!reply.WriteBool(ret)) {
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_NONE;
}
} // namespace NetManagerStandard
} // namespace OHOS
