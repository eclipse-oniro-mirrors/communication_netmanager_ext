/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "wearable_distributed_net_supplier_info.h"

namespace OHOS {
namespace NetManagerStandard {
void WearableDistributedNetSupplierInfo::SetAvailable(NetSupplierInfo &supplierInfo)
{
    supplierInfo.isAvailable_ = CONSTANTS::AVALIABLE;
}

void WearableDistributedNetSupplierInfo::SetRoamingStatus(NetSupplierInfo &supplierInfo)
{
    supplierInfo.isRoaming_ = CONSTANTS::ROAMING_STATUS;
}

void WearableDistributedNetSupplierInfo::SetLinkUpBandwidthKbps(NetSupplierInfo &supplierInfo)
{
    supplierInfo.linkUpBandwidthKbps_ = CONSTANTS::LINKUP_BAND_WIDTH_KBPS;
}

void WearableDistributedNetSupplierInfo::SetLinkDownBandwidthKbps(NetSupplierInfo &supplierInfo)
{
    supplierInfo.linkDownBandwidthKbps_ = CONSTANTS::LINKDOWN_BAND_WIDTH_KBPS;
}

void SetNetSupplierInfo(NetSupplierInfo &supplierInfo)
{
    WearableDistributedNetSupplierInfo info;
    info.SetAvailable(supplierInfo);
    info.SetRoamingStatus(supplierInfo);
    info.SetLinkUpBandwidthKbps(supplierInfo);
    info.SetLinkDownBandwidthKbps(supplierInfo);
}
} // namespace NetManagerStandard
} // namespace OHOS