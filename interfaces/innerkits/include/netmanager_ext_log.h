/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef COMMUNICATIONNETMANAGER_EXT_NETMANAGER_EXT_LOG
#define COMMUNICATIONNETMANAGER_EXT_NETMANAGER_EXT_LOG

#include "netmanager_base_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef LOG_TAG
#ifndef NETMGR_LOG_TAG
#define LOG_TAG "NETMANAGER_EXT"
#else
#define LOG_TAG NETMGR_LOG_TAG
#endif
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD0015B0

#define NETMANAGER_EXT_LOGE(fmt, ...) NETMANAGER_LOG(ERROR, fmt, ##__VA_ARGS__)
#define NETMANAGER_EXT_LOGI(fmt, ...) NETMANAGER_LOG(INFO, fmt, ##__VA_ARGS__)

#define NETMGR_EXT_LOG_D(fmt, ...) NETMANAGER_LOG(DEBUG, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_E(fmt, ...) NETMANAGER_LOG(ERROR, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_W(fmt, ...) NETMANAGER_LOG(WARN, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_I(fmt, ...) NETMANAGER_LOG(INFO, fmt, ##__VA_ARGS__)
#define NETMGR_EXT_LOG_F(fmt, ...) NETMANAGER_LOG(FATAL, fmt, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATIONNETMANAGER_EXT_NETMANAGER_EXT_LOG */
