/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "napi_utils.h"
#include "netmgr_ext_log_wrapper.h"

#include "constant.h"
#include "mdns_base_context.h"

namespace OHOS::NetManagerStandard {
MDnsBaseContext::MDnsBaseContext(napi_env env, EventManager *manager) : BaseContext(env, manager) {}

void MDnsBaseContext::ParseAddressObj(napi_env env, napi_value obj)
{
    if (NapiUtils::GetValueType(env, obj) == napi_object) {
        if (!NapiUtils::HasNamedProperty(env, obj, SERVICEINFO_ADDRESS)) {
            serviceInfo_.addr = "";
        } else {
            serviceInfo_.addr = NapiUtils::GetStringPropertyUtf8(env, obj, SERVICEINFO_ADDRESS);
        }

        if (!NapiUtils::HasNamedProperty(env, obj, SERVICEINFO_FAMILY)) {
            serviceInfo_.family = MDnsServiceInfo::UNKNOWN;
            return;
        }
        int32_t family = NapiUtils::GetInt32Property(env, obj, SERVICEINFO_FAMILY);
        if (family == 0) {
            serviceInfo_.family = MDnsServiceInfo::IPV6;
        } else if (family == 1) {
            serviceInfo_.family = MDnsServiceInfo::IPV4;
        }
    } else {
        NETMGR_EXT_LOG_E("host is not napi_object");
    }
}

bool MDnsBaseContext::GetAttributeObj(napi_env env, napi_value obj, uint32_t &len)
{
    if (!NapiUtils::IsArray(env, obj)) {
        return false;
    }
    len = NapiUtils::GetArrayLength(env, obj);
    return true;
}

void MDnsBaseContext::ParseAttributeObj(napi_env env, napi_value obj, TxtRecord &attrMap)
{
    uint32_t arrLen = 0;
    if (GetAttributeObj(env, obj, arrLen)) {
        return;
    }

    for (size_t i = 0; i < arrLen; i++) {
        napi_value svrAttr = NapiUtils::GetArrayElement(env, obj, i);
        std::string key;
        if (NapiUtils::GetValueType(env, svrAttr) != napi_object) {
            continue;
        }
        if (NapiUtils::HasNamedProperty(env, svrAttr, SERVICEINFO_ATTR_KEY)) {
            key = NapiUtils::GetStringPropertyUtf8(env, obj, SERVICEINFO_TYPE);
        }
        if (!NapiUtils::HasNamedProperty(env, svrAttr, SERVICEINFO_ATTR_VALUE)) {
            continue;
        }
        napi_value valueObj = NapiUtils::GetNamedProperty(env, svrAttr, SERVICEINFO_ATTR);
        uint32_t valArrLen = 0;
        if (GetAttributeObj(env, valueObj, valArrLen)) {
            continue;
        }
        std::vector<uint8_t> typeArray;
        for (size_t j = 0; j < valArrLen; j++) {
            napi_value valAttr = NapiUtils::GetArrayElement(env, svrAttr, j);
            if (NapiUtils::GetValueType(env, valAttr) != napi_number) {
                continue;
            }
            typeArray.push_back(static_cast<uint8_t>(NapiUtils::GetInt32FromValue(env, valAttr)));
        }
        if (typeArray.size() > 0) {
            attrMap[key] = typeArray;
        }
    }
}

void MDnsBaseContext::ParseServiceInfo(napi_env env, napi_value value)
{
    if (NapiUtils::HasNamedProperty(env, value, SERVICEINFO_TYPE)) {
        serviceInfo_.type = NapiUtils::GetStringPropertyUtf8(env, value, SERVICEINFO_TYPE);
    }
    if (NapiUtils::HasNamedProperty(env, value, SERVICEINFO_NAME)) {
        serviceInfo_.name = NapiUtils::GetStringPropertyUtf8(env, value, SERVICEINFO_NAME);
    }
    if (NapiUtils::HasNamedProperty(env, value, SERVICEINFO_PORT)) {
        serviceInfo_.port = NapiUtils::GetInt32Property(env, value, SERVICEINFO_PORT);
    }

    if (!NapiUtils::HasNamedProperty(env, value, SERVICEINFO_HOST)) {
        NETMGR_EXT_LOG_E("host infomation does not exit");
        return;
    }
    napi_value hostObj = NapiUtils::GetNamedProperty(env, value, SERVICEINFO_HOST);
    ParseAddressObj(env, hostObj);

    if (!NapiUtils::HasNamedProperty(env, value, SERVICEINFO_ATTR)) {
        NETMGR_EXT_LOG_E("serviceAttribute infomation does not exit");
        return;
    }
    napi_value attrObj = NapiUtils::GetNamedProperty(env, value, SERVICEINFO_ATTR);
    TxtRecord attrMap;
    ParseAttributeObj(env, attrObj, attrMap);
    serviceInfo_.SetAttrMap(attrMap);
}
} // namespace OHOS::NetManagerStandard
