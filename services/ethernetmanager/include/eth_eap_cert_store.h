/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ETH_EAP_CERT_STORE_H
#define ETH_EAP_CERT_STORE_H

#include <string>
#include <vector>

#include "eth_eap_config.h"

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

namespace OHOS {
namespace NetManagerStandard {

// 802.1X 证书落盘管理：客户端凭据 keyUri 直接透传 wpa（私钥不出证书管理器）；
// CA keyUri 由本服务解析为 PEM 落盘 ca.pem 回填 caPath，失败仅降级不校验服务器证书；
// 企业 API 的 certEntry（P12 内容）在本类校验落盘。
// 企业 API 的客户端 P12 文件内容（certEntry）在本类校验落盘。
class EthEapCertStore {
public:
    static EthEapCertStore &GetInstance();

    // SetEthEapConfig 挂接：certEntry 校验落盘 client.p12 回填路径；caCertAliases 尽力解析
    // CA 落盘 ca.pem（失败仅告警降级）。返回 SUCCESS 或 EAP_ERRCODE_CERT_PROCESS_FAILED(33200013)
    int32_t ProcessCertFields(EthEapProfile &profile);

    // 删除配置时清理落盘证书
    void Clear();

    // 认证前自愈：落盘文件缺失时用持久化数据重建（失败仅告警，不阻断认证）
    int32_t EnsureCertFiles(EthEapProfile &profile);

private:
    EthEapCertStore() = default;
    ~EthEapCertStore() = default;

    // P12 内容校验 → 落盘 client.p12
    int32_t StoreClientCert(const std::vector<uint8_t> &entry, std::string &outPath);
    // CA keyUri → CmGetUserCertInfo（用户受信存储）→ PEM 落盘 ca.pem，outPath 回填落盘路径。
    // 尽力而为：任一步失败返回 false（调用方降级为不校验服务器证书），不抛错
    bool ResolveCaCert(const std::string &keyUri, std::string &outPath);
    // tmp + fsync + rename 原子写；目录不存在则创建；lstat 拒绝符号链接；0600 权限
    int32_t AtomicWriteFile(const std::string &path, const uint8_t *data, size_t len);
};

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#endif // ETH_EAP_CERT_STORE_H
