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
#include "net_policy_file.h"

#include <json/json.h>
#include <fcntl.h>

#include "net_policy_define.h"
#include "net_mgr_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {
bool NetPolicyFile::FileExists(const std::string &fileName)
{
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == static_cast<int32_t>(NetPolicyResultCode::ERR_NONE));
}

bool NetPolicyFile::CreateFile(const std::string& fileName)
{
    if (fileName.empty() || FileExists(fileName)) {
        NETMGR_LOGE("fileName empty or file not exists.");
        return false;
    }

    int32_t fd = open(fileName.c_str(), O_CREAT | O_WRONLY, CHOWN_RWX_USR_GRP);
    if (fd < 0) {
        NETMGR_LOGE("open file error.");
        return false;
    }
    close(fd);

    return true;
}

bool NetPolicyFile::Json2Obj(const std::string &content, NetPolicy &netPolicy)
{
    if (content.empty()) {
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder buidler;
    std::unique_ptr<Json::CharReader> reader(buidler.newCharReader());
    JSONCPP_STRING errs;

    bool isSuccess = reader->parse(content.c_str(), content.c_str() + content.length(), &root, &errs);
    if (isSuccess && errs.size() == 0) {
        netPolicy.hosVersion = root[CONFIG_HOS_VERSION].asString();
        if (netPolicy.hosVersion.empty()) {
            netPolicy.hosVersion = HOS_VERSION;
        }
        const Json::Value arrayUidPolicy = root[CONFIG_UID_POLICY];
        uint32_t size = arrayUidPolicy.size();
        UidPolicy uidPolicy;
        for (uint32_t i = 0; i < size; i++) {
            uidPolicy.uid= arrayUidPolicy[i][CONFIG_UID].asString();
            uidPolicy.policy = arrayUidPolicy[i][CONFIG_POLICY].asString();
            netPolicy.uidPolicys.push_back(uidPolicy);
        }
    }

    return true;
}

bool NetPolicyFile::ReadFile(const std::string &fileName, std::string &fileContent)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (fileName.empty()) {
        NETMGR_LOGE("fileName empty.");
        return false;
    }

    if (!FileExists(fileName)) {
        NETMGR_LOGE("[%{public}s] not exist.", fileName.c_str());
        return false;
    }

    std::fstream file(fileName.c_str(), std::fstream::in);
    if (file.is_open() == false) {
        NETMGR_LOGE("fstream failed.");
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    fileContent = buffer.str();
    file.close();

    return true;
}

bool NetPolicyFile::WriteFile(const std::string &fileName)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (fileName.empty()) {
        NETMGR_LOGE("fileName is empty.");
        return false;
    }

    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> streamWriter(builder.newStreamWriter());
    std::fstream file(fileName.c_str(), std::fstream::out);
    if (file.is_open() == false) {
        NETMGR_LOGE("fstream failed.");
        return false;
    }

    root[CONFIG_HOS_VERSION] = Json::Value(netPolicy_.hosVersion);
    uint32_t size = netPolicy_.uidPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        Json::Value uidPolicy;
        uidPolicy[CONFIG_UID] = netPolicy_.uidPolicys[i].uid;
        uidPolicy[CONFIG_POLICY] = netPolicy_.uidPolicys[i].policy;
        std::string policy = std::to_string(static_cast<uint32_t>(NetUidPolicy::NET_POLICY_TEMPORARY_ALLOW_METERED));
        /* Temporary permission, no need to write files */
        if (netPolicy_.uidPolicys[i].policy == policy) {
            continue;
        }
        root[CONFIG_UID_POLICY].append(uidPolicy);
    }
    std::ostringstream out;
    streamWriter->write(root, &out);
    file << out.str().c_str();
    file.close();

    return true;
}

bool NetPolicyFile::WriteFile(const NetUidPolicyOpType netUidPolicyOpType, uint32_t uid, NetUidPolicy policy)
{
    if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_UPDATE) {
        for (auto &uidPolicy : netPolicy_.uidPolicys) {
            if (uidPolicy.uid == std::to_string(uid)) {
                uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
                break;
            }
        }
    } else if (netUidPolicyOpType == NetUidPolicyOpType::NET_POLICY_UID_OP_TYPE_DELETE) {
        for (auto iter = netPolicy_.uidPolicys.begin(); iter != netPolicy_.uidPolicys.end(); ++iter) {
            if (iter->uid == std::to_string(uid)) {
                netPolicy_.uidPolicys.erase(iter);
                break;
            }
        }
    } else {
        UidPolicy uidPolicy;
        uidPolicy.uid = std::to_string(uid);
        uidPolicy.policy = std::to_string(static_cast<uint32_t>(policy));
        netPolicy_.uidPolicys.push_back(uidPolicy);
    }

    if (!WriteFile(POLICY_FILE_NAME)) {
        NETMGR_LOGE("WriteFile failed");
        return false;
    }

    return true;
}

bool NetPolicyFile::IsUidPolicyExist(uint32_t uid)
{
    uint32_t size = netPolicy_.uidPolicys.size();
    for (uint32_t i = 0; i < size; i++) {
        if (static_cast<uint32_t>(std::stol(netPolicy_.uidPolicys[i].uid)) == uid) {
            return true;
        }
    }

    return false;
}

NetUidPolicy NetPolicyFile::GetUidPolicy(uint32_t uid)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.uid == std::to_string(uid)) {
            return static_cast<NetUidPolicy>(std::stol(uidPolicy.policy));
        }
    }

    return NetUidPolicy::NET_POLICY_NONE;
}

bool NetPolicyFile::GetUids(NetUidPolicy policy, std::vector<uint32_t> &uids)
{
    for (auto &uidPolicy : netPolicy_.uidPolicys) {
        if (uidPolicy.policy == std::to_string(static_cast<uint32_t>(policy))) {
            uint32_t uid = static_cast<uint32_t>(std::stol(uidPolicy.uid));
            uids.push_back(uid);
        }
    }

    return true;
}

bool NetPolicyFile::InitPolicy()
{
    std::string content;
    if (!ReadFile(POLICY_FILE_NAME, content)) {
        if (!CreateFile(POLICY_FILE_NAME)) {
            NETMGR_LOGE("CreateFile [%{public}s] failed", POLICY_FILE_NAME);
            return false;
        }
    }
    if (!content.empty() && !Json2Obj(content, netPolicy_)) {
        NETMGR_LOGE("Analysis fileconfig failed");
        return false;
    }
    return true;
}
} // namespace NetManagerStandard
} // namespace OHOS