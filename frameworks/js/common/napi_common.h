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

#ifndef NAPI_COMMON_H
#define NAPI_COMMON_H

#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace NetManagerStandard {
// define callback ref count
constexpr uint32_t CALLBACK_REF_CNT = 1;
// define property max bytes
constexpr uint32_t PROPERTY_MAX_BYTE = 128;
// define constant of 64 bytes
constexpr uint32_t BUFFER_BYTE = 64;

enum class JS_CALLBACK_ARGV : int32_t {
    CALLBACK_ARGV_INDEX_0 = 0,
    CALLBACK_ARGV_INDEX_1,
    CALLBACK_ARGV_CNT,
};

enum class JS_ARGV_NUM : int32_t {
    ARGV_NUM_0 = 0,
    ARGV_NUM_1,
    ARGV_NUM_2,
    ARGV_NUM_3,
    ARGV_NUM_4,
    ARGV_NUM_5,
};

enum class JS_ARGV_INDEX : int32_t {
    ARGV_INDEX_0 = 0,
    ARGV_INDEX_1,
    ARGV_INDEX_2,
    ARGV_INDEX_3,
    ARGV_INDEX_4,
};

class NapiCommon {
public:
    static napi_value CreateCodeMessage(napi_env env, const std::string &msg, int32_t code);
    static napi_value NapiValueByInt32(napi_env env, int32_t property);
    static void SetPropertyInt32(napi_env env, napi_value object, const std::string &propertyName, int32_t property);
    static void SetPropertyString(napi_env env, napi_value object, const std::string &propertyName,
        const std::string &property);
    static void GetPropertyString(napi_env env, napi_value object, const std::string &propertyName,
        std::string &property);
    static void GetPropertyInt32(napi_env env, napi_value object, const std::string &propertyName, int32_t &property);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NAPI_COMMON_H

