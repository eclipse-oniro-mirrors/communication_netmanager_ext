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

#include "ethernet_service_stub.h"

#include "i_ethernet_service.h"
#include "interface_configuration.h"
#include "ipc_object_stub.h"
#include "message_parcel.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"
#include "interface_type.h"

namespace OHOS {
namespace NetManagerStandard {
EthernetServiceStub::EthernetServiceStub()
{
    memberFuncMap_[CMD_SET_IF_CFG] = &EthernetServiceStub::OnSetIfaceConfig;
    memberFuncMap_[CMD_GET_IF_CFG] = &EthernetServiceStub::OnGetIfaceConfig;
    memberFuncMap_[CMD_IS_ACTIVATE] = &EthernetServiceStub::OnIsIfaceActive;
    memberFuncMap_[CMD_GET_ACTIVATE_INTERFACE] = &EthernetServiceStub::OnGetAllActiveIfaces;
    memberFuncMap_[CMD_RESET_FACTORY] = &EthernetServiceStub::OnResetFactory;
    memberFuncMap_[CMD_SET_INTERFACE_UP] = &EthernetServiceStub::OnSetInterfaceUp;
    memberFuncMap_[CMD_SET_INTERFACE_DOWN] = &EthernetServiceStub::OnSetInterfaceDown;
    memberFuncMap_[CMD_GET_INTERFACE_CONFIG] = &EthernetServiceStub::OnGetInterfaceConfig;
}

EthernetServiceStub::~EthernetServiceStub() {}

int32_t EthernetServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NETMGR_EXT_LOG_D("stub call start, code = [%{public}d]", code);

    std::u16string myDescripter = EthernetServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        NETMGR_EXT_LOG_E("descriptor checked fail");
        return NETMANAGER_EXT_ERR_DESCRIPTOR_MISMATCH;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    NETMGR_EXT_LOG_D("stub default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t EthernetServiceStub::OnSetIfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    sptr<InterfaceConfiguration> ic = InterfaceConfiguration::Unmarshalling(data);
    if (ic == nullptr) {
        return NETMANAGER_EXT_ERR_LOCAL_PTR_NULL;
    }
    int32_t ret = SetIfaceConfig(iface, ic);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnGetIfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    sptr<InterfaceConfiguration> ic = GetIfaceConfig(iface);
    if (ic != nullptr) {
        if (!reply.WriteInt32(GET_CFG_SUC)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
        if (!ic->Marshalling(reply)) {
            NETMGR_EXT_LOG_E("proxy Marshalling failed");
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    } else {
        if (!reply.WriteInt32(0)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnIsIfaceActive(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t ret = IsIfaceActive(iface);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnGetAllActiveIfaces(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> ifaces = GetAllActiveIfaces();
    if (!reply.WriteInt32(ifaces.size())) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    for (auto it = ifaces.begin(); it != ifaces.end(); ++it) {
        if (!reply.WriteString(*it)) {
            return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnResetFactory(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = ResetFactory();
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnSetInterfaceUp(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t ret = SetInterfaceUp(iface);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnSetInterfaceDown(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    int32_t ret = SetInterfaceDown(iface);
    if (!reply.WriteInt32(ret)) {
        return NETMANAGER_EXT_ERR_WRITE_REPLY_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthernetServiceStub::OnGetInterfaceConfig(MessageParcel &data, MessageParcel &reply)
{
    std::string iface;
    if (!data.ReadString(iface)) {
        return NETMANAGER_EXT_ERR_READ_DATA_FAIL;
    }
    OHOS::nmd::InterfaceConfigurationParcel cfg;
    bool result = GetInterfaceConfig(iface, cfg);
    if (!result) {
        NETMGR_EXT_LOG_E("GetInterfaceConfig is error");
        return NETMANAGER_EXT_ERROR;
    }
    reply.WriteString(cfg.ifName);
    reply.WriteString(cfg.hwAddr);
    reply.WriteString(cfg.ipv4Addr);
    reply.WriteInt32(cfg.prefixLength);
    int32_t vsize = static_cast<int32_t>(cfg.flags.size());
    reply.WriteInt32(vsize);
    std::vector<std::string>::iterator iter;
    for (iter = cfg.flags.begin(); iter != cfg.flags.end(); ++iter) {
        reply.WriteString(*iter);
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS