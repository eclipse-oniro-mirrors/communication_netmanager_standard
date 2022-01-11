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
#ifndef DNS_SERVICE_IFACE_H
#define DNS_SERVICE_IFACE_H

#include "dns_base_service.h"

namespace OHOS {
namespace NetManagerStandard {
class DnsServiceIface : public DnsBaseService {
public:
    int32_t GetAddressesByName(const std::string &hostName, uint16_t netId,
        std::vector<INetAddr> &addrInfo) override;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // DNS_SERVICE_IFACE_H