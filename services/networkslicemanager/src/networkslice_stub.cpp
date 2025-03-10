/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <sys/socket.h>

#include "errors.h"
#include "hilog/log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "netmgr_ext_log_wrapper.h"
#include "networkslice_ipc_interface_code.h"
#include "networkslice_service.h"
#include "net_manager_constants.h"
#include "networkslice_stub.h"

namespace OHOS {
namespace NetManagerStandard {

NetworkSliceStub::NetworkSliceStub()
{
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::SET_NETWORKSLICE_UEPOLICY)] =
        &NetworkSliceStub::OnSetNetworkSlicePolicy;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_INIT_UEPOLICY)] =
        &NetworkSliceStub::OnNetworkSliceInitUePolicy;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_ALLOWEDNSSAI_RPT)] =
        &NetworkSliceStub::OnNetworkSliceAllowedNssaiRpt;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_EHPLMN_RPT)] =
        &NetworkSliceStub::OnNetworkSliceEhplmnRpt;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYDNN)] =
        &NetworkSliceStub::OnGetRouteSelectionDescriptorByDNN;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_GETRSDBYNETCAP)] =
        &NetworkSliceStub::OnGetRSDByNetCap;
    memberFuncMap_[static_cast<uint32_t>(NetworkSliceInterfaceCode::NETWORKSLICE_SETSASTATE)] =
        &NetworkSliceStub::OnSetSaState;
}

int32_t NetworkSliceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string myDescripter = NetworkSliceStub::GetDescriptor();
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
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t NetworkSliceStub::OnSetNetworkSlicePolicy(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnSetNetworkSlicePolicy");
    int32_t buffersize = data.ReadInt32();
    std::vector<uint8_t> buffer;
    for (int32_t i = 0; i < buffersize; ++i) {
        buffer.push_back(data.ReadUint8());
    }
    int32_t ret = NetworkSliceService::GetInstance().SetNetworkSliceUePolicy(buffer);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceStub::OnNetworkSliceAllowedNssaiRpt(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnNetworkSliceAllowedNssaiRpt");
    int32_t buffersize = data.ReadInt32();
    std::vector<uint8_t> buffer;
    for (int i = 0; i < buffersize; ++i) {
        buffer.push_back(data.ReadUint8());
    }
    int32_t ret = NetworkSliceService::GetInstance().NetworkSliceAllowedNssaiRpt(buffer);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceStub::OnNetworkSliceEhplmnRpt(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnNetworkSliceEhplmnRpt");
    int32_t buffersize = data.ReadInt32();
    std::vector<uint8_t> buffer;
    for (int i = 0; i < buffersize; ++i) {
        buffer.push_back(data.ReadUint8());
    }
    int32_t ret = NetworkSliceService::GetInstance().NetworkSliceEhplmnRpt(buffer);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceStub::OnNetworkSliceInitUePolicy(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnNetworkSliceInitUePolicy");
    int32_t ret = NetworkSliceService::GetInstance().NetworkSliceInitUePolicy();
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceStub::OnGetRouteSelectionDescriptorByDNN(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnGetRouteSelectionDescriptorByDNN");
    std::string dnn = data.ReadString();
    std::string snssai;
    uint8_t sscmode;
    int32_t ret = NetworkSliceService::GetInstance().GetRouteSelectionDescriptorByDNN(dnn, snssai, sscmode);
    reply.WriteString(snssai);
    reply.WriteUint8(sscmode);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t NetworkSliceStub::OnGetRSDByNetCap(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnGetRSDByNetCap");
    int32_t netcap = data.ReadInt32();
    std::map<std::string, std::string> networkSliceParas;
    int32_t ret = NetworkSliceService::GetInstance().GetRSDByNetCap(netcap, networkSliceParas);
    if (networkSliceParas.find("dnn") != networkSliceParas.end() && !networkSliceParas["dnn"].empty()) {
        reply.WriteString(networkSliceParas["dnn"]);
    } else {
        reply.WriteString("Invalid");
    }
    if (networkSliceParas.find("snssai") != networkSliceParas.end() && !networkSliceParas["snssai"].empty()) {
        reply.WriteString(networkSliceParas["snssai"]);
    } else {
        reply.WriteString("Invalid");
    }
    if (networkSliceParas.find("sscmode") != networkSliceParas.end() && !networkSliceParas["sscmode"].empty()) {
        reply.WriteString(networkSliceParas["sscmode"]);
    } else {
        reply.WriteString("0");
    }
    if (networkSliceParas.find("pdusessiontype") != networkSliceParas.end() &&
        !networkSliceParas["pdusessiontype"].empty()) {
        reply.WriteString(networkSliceParas["pdusessiontype"]);
    } else {
        reply.WriteString("0");
    }
    if (networkSliceParas.find("routebitmap") != networkSliceParas.end() && !networkSliceParas["routebitmap"].empty()) {
        reply.WriteString(networkSliceParas["routebitmap"]);
    } else {
        reply.WriteString("0");
    }
    return ret;
}

int32_t NetworkSliceStub::OnSetSaState(MessageParcel &data, MessageParcel &reply)
{
    NETMGR_EXT_LOG_I("NetworkSliceStub::OnSetSaState");
    bool isSaState = data.ReadBool();
    int32_t ret = NetworkSliceService::GetInstance().SetSaState(isSaState);
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
