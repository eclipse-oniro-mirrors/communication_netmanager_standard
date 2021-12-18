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
#include "net_policy_service_proxy.h"

#include "net_policy_constants.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
NetPolicyServiceProxy::NetPolicyServiceProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetPolicyService>(impl)
{}

NetPolicyServiceProxy::~NetPolicyServiceProxy() {}

NetPolicyResultCode NetPolicyServiceProxy::SetUidPolicy(uint32_t uid, NetUidPolicy policy)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteUint32(uid)) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(policy))) {
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    int32_t error = remote->SendRequest(CMD_NSM_SET_UID_POLICY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetPolicyResultCode::ERR_INTERNAL_ERROR;
    }

    return static_cast<NetPolicyResultCode>(reply.ReadInt32());
}

NetUidPolicy NetPolicyServiceProxy::GetUidPolicy(uint32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    if (!data.WriteUint32(uid)) {
        return NetUidPolicy::NET_POLICY_NONE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return NetUidPolicy::NET_POLICY_NONE;
    }

    int32_t error = remote->SendRequest(CMD_NSM_GET_UID_POLICY, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return NetUidPolicy::NET_POLICY_NONE;
    }

    return static_cast<NetUidPolicy>(reply.ReadInt32());
}

std::vector<uint32_t> NetPolicyServiceProxy::GetUids(NetUidPolicy policy)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<uint32_t> uids;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return uids;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(policy))) {
        return uids;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return uids;
    }

    int32_t error = remote->SendRequest(CMD_NSM_GET_UIDS, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return uids;
    }

    bool errorRet = reply.ReadUInt32Vector(&uids);
    if (errorRet == false) {
        NETMGR_LOGE("proxy SendRequest Readuint32Vector failed");
    }

    return uids;
}

bool NetPolicyServiceProxy::IsUidNetAccess(uint32_t uid, bool metered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<uint32_t> uids;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(uid)) {
        return false;
    }

    if (!data.WriteBool(metered)) {
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return false;
    }

    int32_t error = remote->SendRequest(CMD_NSM_IS_NET_ACCESS_METERED, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return false;
    }

    return reply.ReadBool();
}

bool NetPolicyServiceProxy::IsUidNetAccess(uint32_t uid, const std::string &ifaceName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<uint32_t> uids;
    if (!WriteInterfaceToken(data)) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return false;
    }

    if (!data.WriteUint32(uid)) {
        return false;
    }

    if (!data.WriteString(ifaceName)) {
        return false;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        NETMGR_LOGE("Remote is null");
        return false;
    }

    int32_t error = remote->SendRequest(CMD_NSM_IS_NET_ACCESS_IFACENAME, data, reply, option);
    if (error != ERR_NONE) {
        NETMGR_LOGE("proxy SendRequest failed, error code: [%{public}d]", error);
        return false;
    }

    return reply.ReadBool();
}

bool NetPolicyServiceProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetPolicyServiceProxy::GetDescriptor())) {
        NETMGR_LOGE("WriteInterfaceToken failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS
