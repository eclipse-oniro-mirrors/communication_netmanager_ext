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

#include "eth_eap_cert_store.h"

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <securec.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/x509.h>

#include "cert_manager_api.h"
#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

static constexpr const char *CERT_DIR = "/data/service/el1/public/eth/eap_certs/";
static constexpr const char *CA_FILE = "/data/service/el1/public/eth/eap_certs/ca.pem";
static constexpr const char *CLIENT_FILE = "/data/service/el1/public/eth/eap_certs/client.p12";
static constexpr const char *KEY_URI_PREFIX = "oh:";
static constexpr size_t PEM_PREFIX_LEN = 10; // "-----BEGIN"

static bool IsValidPkcs12(const std::vector<uint8_t> &entry)
{
    const unsigned char *p = entry.data();
    PKCS12 *p12 = d2i_PKCS12(nullptr, &p, static_cast<long>(entry.size()));
    if (p12 == nullptr) {
        return false;
    }
    PKCS12_free(p12);
    return true;
}

static bool DerToPem(const uint8_t *data, uint32_t size, std::string &pem)
{
    const unsigned char *p = data;
    X509 *cert = d2i_X509(nullptr, &p, static_cast<long>(size));
    if (cert == nullptr) {
        return false;
    }
    BIO *bio = BIO_new(BIO_s_mem());
    bool ok = bio != nullptr && PEM_write_bio_X509(bio, cert) == 1;
    if (ok) {
        char *buf = nullptr;
        long len = BIO_get_mem_data(bio, &buf);
        if (len > 0) {
            pem.assign(buf, static_cast<size_t>(len));
        } else {
            ok = false;
        }
    }
    BIO_free(bio);
    X509_free(cert);
    return ok;
}

EthEapCertStore &EthEapCertStore::GetInstance()
{
    static EthEapCertStore instance;
    return instance;
}

int32_t EthEapCertStore::AtomicWriteFile(const std::string &path, const uint8_t *data, size_t len)
{
    struct stat pathStat;
    if (lstat(path.c_str(), &pathStat) == 0 && S_ISLNK(pathStat.st_mode)) {
        NETMGR_EXT_LOG_E("AtomicWriteFile: target is a symlink: %{public}s", path.c_str());
        return EAP_ERRCODE_CERT_PROCESS_FAILED;
    }
    struct stat dirStat;
    if (stat(CERT_DIR, &dirStat) != 0) {
        if (mkdir(CERT_DIR, S_IRWXU) != 0 && errno != EEXIST) {
            NETMGR_EXT_LOG_E("AtomicWriteFile: mkdir failed, errno: %{public}d", errno);
            return EAP_ERRCODE_CERT_PROCESS_FAILED;
        }
    }
    std::string tmpPath = path + ".tmp";
    (void)unlink(tmpPath.c_str());
    int32_t fd = open(tmpPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        NETMGR_EXT_LOG_E("AtomicWriteFile: open failed, errno: %{public}d", errno);
        return EAP_ERRCODE_CERT_PROCESS_FAILED;
    }
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, data + written, len - written);
        if (n <= 0) {
            NETMGR_EXT_LOG_E("AtomicWriteFile: write failed, errno: %{public}d", errno);
            (void)close(fd);
            (void)unlink(tmpPath.c_str());
            return EAP_ERRCODE_CERT_PROCESS_FAILED;
        }
        written += static_cast<size_t>(n);
    }
    bool syncOk = fsync(fd) == 0;
    (void)close(fd);
    if (!syncOk || rename(tmpPath.c_str(), path.c_str()) != 0) {
        NETMGR_EXT_LOG_E("AtomicWriteFile: fsync/rename failed, errno: %{public}d", errno);
        (void)unlink(tmpPath.c_str());
        return EAP_ERRCODE_CERT_PROCESS_FAILED;
    }
    (void)chmod(path.c_str(), S_IRUSR | S_IWUSR);
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapCertStore::StoreClientCert(const std::vector<uint8_t> &entry, std::string &outPath)
{
    if (entry.empty() || !IsValidPkcs12(entry)) {
        NETMGR_EXT_LOG_E("StoreClientCert: invalid pkcs12 entry, size: %{public}zu", entry.size());
        return EAP_ERRCODE_CERT_PROCESS_FAILED;
    }
    int32_t ret = AtomicWriteFile(CLIENT_FILE, entry.data(), entry.size());
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return EAP_ERRCODE_CERT_PROCESS_FAILED;
    }
    outPath = CLIENT_FILE;
    return NETMANAGER_EXT_SUCCESS;
}

bool EthEapCertStore::ResolveCaCert(const std::string &keyUri, std::string &outPath)
{
    // 尽力而为：用户受信 CA 存储的 keyUri → CmGetUserCertInfo → PEM 落盘 ca.pem。
    // 任一步失败返回 false（调用方降级为不校验服务器证书），不抛错。
    struct CmBlob uriBlob;
    uriBlob.size = static_cast<uint32_t>(keyUri.size() + 1); // 含结尾 '\0'，对齐证书管理器用法
    uriBlob.data = reinterpret_cast<uint8_t *>(const_cast<char *>(keyUri.c_str()));

    struct CertInfo certInfo;
    (void)memset_s(&certInfo, sizeof(certInfo), 0, sizeof(certInfo));
    uint8_t certData[MAX_LEN_CERTIFICATE] = {0};
    certInfo.certInfo.data = certData;
    certInfo.certInfo.size = MAX_LEN_CERTIFICATE;

    int32_t ret = CmGetUserCertInfo(&uriBlob, CM_USER_TRUSTED_STORE, &certInfo);
    if (ret != CM_SUCCESS) {
        NETMGR_EXT_LOG_W("ResolveCaCert: CmGetUserCertInfo failed: %{public}d", ret);
        return false;
    }
    if (certInfo.certInfo.data == nullptr || certInfo.certInfo.size == 0) {
        NETMGR_EXT_LOG_W("ResolveCaCert: empty cert data");
        return false;
    }
    // 用户 CA 安装时经 CmX509ToPEM 归一为 PEM 存储；防御性支持 DER 输入
    const uint8_t *data = certInfo.certInfo.data;
    uint32_t size = certInfo.certInfo.size;
    std::string pem;
    if (size > PEM_PREFIX_LEN && memcmp(data, "-----BEGIN", PEM_PREFIX_LEN) == 0) {
        pem.assign(reinterpret_cast<const char *>(data), size);
    } else if (!DerToPem(data, size, pem)) {
        NETMGR_EXT_LOG_W("ResolveCaCert: cert is neither PEM nor DER, size: %{public}u", size);
        return false;
    }
    if (AtomicWriteFile(CA_FILE, reinterpret_cast<const uint8_t *>(pem.data()), pem.size()) != NETMANAGER_EXT_SUCCESS) {
        return false;
    }
    outPath = CA_FILE;
    return true;
}

int32_t EthEapCertStore::ProcessCertFields(EthEapProfile &profile)
{
    // 客户端凭据：keyUri（clientCertAliases）直接透传 wpa_supplicant
    // （client_cert/private_key 支持 oh: 解析，与 WiFi EAP 同机制）；
    // 文件方式（企业 API certEntry）：P12 内容校验后落盘，clientCertAliases 回填落盘路径。
    if (!profile.certEntry.empty()) {
        std::string clientFile;
        int32_t ret = StoreClientCert(profile.certEntry, clientFile);
        if (ret != NETMANAGER_EXT_SUCCESS) {
            return ret;
        }
        profile.clientCertAliases = clientFile;
    }
    // CA：keyUri wpa 不支持（ca_cert 无 oh: 解析），尽力解析落盘 ca.pem 回填 caPath
    // （成功→完整校验服务器证书）；失败仅告警降级（wpa 跳过 ca_cert），不报错给上层。
    // caPath 已显式指向非本类管理路径（企业 API 直传/系统证书目录）时不覆盖。
    if (profile.caCertAliases.rfind(KEY_URI_PREFIX, 0) == 0 &&
        (profile.caPath.empty() || profile.caPath == CA_FILE)) {
        std::string caFile;
        if (ResolveCaCert(profile.caCertAliases, caFile)) {
            profile.caPath = caFile;
        } else if (profile.caPath == CA_FILE) {
            NETMGR_EXT_LOG_W("ProcessCertFields: CA resolve failed, clear stale caPath");
            profile.caPath.clear();
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t EthEapCertStore::EnsureCertFiles(EthEapProfile &profile)
{
    if (!profile.certEntry.empty() && access(CLIENT_FILE, F_OK) != 0) {
        std::string clientFile;
        if (StoreClientCert(profile.certEntry, clientFile) == NETMANAGER_EXT_SUCCESS) {
            profile.clientCertAliases = clientFile;
        } else {
            NETMGR_EXT_LOG_E("EnsureCertFiles: rebuild client cert failed");
        }
    }
    // CA 自愈：CA 来自 keyUri 但 ca.pem 缺失（保存时降级或文件丢失）时重解析，
    // 仍失败则保持/清空 caPath 降级，避免 wpa 加载不存在路径
    if (profile.caCertAliases.rfind(KEY_URI_PREFIX, 0) == 0 &&
        (profile.caPath.empty() || profile.caPath == CA_FILE) && access(CA_FILE, F_OK) != 0) {
        std::string caFile;
        if (ResolveCaCert(profile.caCertAliases, caFile)) {
            profile.caPath = caFile;
        } else if (profile.caPath == CA_FILE) {
            NETMGR_EXT_LOG_W("EnsureCertFiles: CA resolve failed, clear stale caPath");
            profile.caPath.clear();
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

void EthEapCertStore::Clear()
{
    if (unlink(CA_FILE) != 0 && errno != ENOENT) {
        NETMGR_EXT_LOG_E("Clear: remove ca cert failed, errno: %{public}d", errno);
    }
    if (unlink(CLIENT_FILE) != 0 && errno != ENOENT) {
        NETMGR_EXT_LOG_E("Clear: remove client cert failed, errno: %{public}d", errno);
    }
}

} // namespace NetManagerStandard
} // namespace OHOS

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
