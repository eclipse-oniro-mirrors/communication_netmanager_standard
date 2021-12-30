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

#ifndef NET_POLICY_FILE_H
#define NET_POLICY_FILE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <climits>

#include "refbase.h"

#include "net_policy_constants.h"
#include "net_policy_define.h"

namespace OHOS {
namespace NetManagerStandard {
enum class NetUidPolicyOpType {
    NET_POLICY_UID_OP_TYPE_ADD = 1,
    NET_POLICY_UID_OP_TYPE_DELETE = 2,
    NET_POLICY_UID_OP_TYPE_UPDATE = 3,
};

class NetPolicyFile : public virtual RefBase {
public:
    bool InitPolicy();
    bool IsUidPolicyExist(uint32_t uid);
    bool ReadFile(const std::string& fileName, std::string& content);
    bool Json2Obj(const std::string& content, NetPolicy& netPolicy);
    bool WriteFile(const std::string& fileName);
    bool WriteFile(const NetUidPolicyOpType netUidPolicyOpType, uint32_t uid, NetUidPolicy policy);
    NetUidPolicy GetUidPolicy(uint32_t uid);
    bool GetUids(NetUidPolicy policy, std::vector<uint32_t> &uids);

private:
    bool FileExists(const std::string& fileName);
    bool CreateFile(const std::string& fileName);

private:
    NetPolicy netPolicy_;
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_POLICY_FILE_H

