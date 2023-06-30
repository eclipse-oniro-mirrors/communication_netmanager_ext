/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_IPC_INTERFACE_CODE_H
#define ETHERNET_IPC_INTERFACE_CODE_H

/* SAID:1157 */
namespace OHOS {
namespace NetManagerStandard {
enum class EthernetInterfaceCode {
    CMD_SET_IF_CFG,
    CMD_GET_IF_CFG,
    CMD_IS_ACTIVATE,
    CMD_GET_ACTIVATE_INTERFACE,
    CMD_RESET_FACTORY,
    CMD_REGISTER_INTERFACE_CB,
    CMD_UNREGISTER_INTERFACE_CB,
    CMD_SET_INTERFACE_UP,
    CMD_SET_INTERFACE_DOWN,
    CMD_GET_INTERFACE_CONFIG,
    CMD_SET_INTERFACE_CONFIG,
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif //ETHERNET_IPC_INTERFACE_CODE_H
