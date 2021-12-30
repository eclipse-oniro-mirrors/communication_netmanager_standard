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

#include "napi_common.h"

namespace OHOS {
namespace NetManagerStandard {
napi_value NapiCommon::CreateCodeMessage(napi_env env, const std::string &msg, int32_t code)
{
    napi_value messageCodeInfo = nullptr;
    napi_value messageInfo = nullptr;
    napi_value codeInfo = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, msg.c_str(), msg.length(), &messageInfo));
    NAPI_CALL(env, napi_create_string_utf8(
        env, std::to_string(code).c_str(), std::to_string(code).length(), &codeInfo));
    NAPI_CALL(env, napi_create_error(env, codeInfo, messageInfo, &messageCodeInfo));
    return messageCodeInfo;
}

void NapiCommon::SetPropertyInt32(napi_env env, napi_value object, const std::string &propertyName, int32_t property)
{
    napi_value propertyDest = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, property, &propertyDest));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, object, propertyName.c_str(), propertyDest));
}

void NapiCommon::SetPropertyString(
    napi_env env, napi_value object, const std::string &propertyName, const std::string &property)
{
    napi_value propertyDest = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, property.c_str(), property.length(), &propertyDest));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, object, propertyName.c_str(), propertyDest));
}

void NapiCommon::GetPropertyString(
    napi_env env, napi_value object, const std::string &propertyName, std::string &property)
{
    napi_value value = nullptr;
    char propertyBuffer[PROPERTY_MAX_BYTE] = {0};
    size_t realByte = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, object, propertyName.c_str(), &value));
    NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, value, propertyBuffer, PROPERTY_MAX_BYTE, &realByte));
    property = propertyBuffer;
}

void NapiCommon::GetPropertyInt32(napi_env env, napi_value object, const std::string &propertyName, int32_t &property)
{
    napi_value value = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, object, propertyName.c_str(), &value));
    NAPI_CALL_RETURN_VOID(env, napi_get_value_int32(env, value, &property));
}

napi_value NapiCommon::NapiValueByInt32(napi_env env, int32_t property)
{
    napi_value value = nullptr;
    NAPI_CALL(env, napi_create_int32(env, property, &value));
    return value;
}
} // namespace NetManagerStandard
} // namespace OHOS
