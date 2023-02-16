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

#ifndef MDNS_MANAGER_H
#define MDNS_MANAGER_H

#include <any>
#include <list>
#include <string>

#include "mdns_common.h"
#include "mdns_packet_parser.h"
#include "mdns_socket_listener.h"

namespace OHOS {
namespace NetManagerStandard {

struct MDnsConfig {
    bool ipv6Support = true;
    int configAllIface = true;
    std::string topDomain = MDNS_TOP_DOMAIN_DEFAULT;
    std::string hostname;
};

class MDnsProtocolImpl {
public:
    MDnsProtocolImpl();
    ~MDnsProtocolImpl() = default;

    enum ResultType {
        UNKNOWN,
        SERVICE_STARTED,
        SERVICE_STOPED,
        SERVICE_FOUND,
        SERVICE_LOST,
        INSTANCE_RESOLVED,
        DOMAIN_RESOLVED
    };

    using TxtRecord = std::map<std::string, std::vector<uint8_t>>;

    struct Result {
        ResultType type;
        std::string serviceName;
        std::string serviceType;
        std::string domain;
        int port = -1;
        bool ipv6 = false;
        std::string addr;
        TxtRecordEncoded txt;
        std::string iface;
    };

    using Handler = std::function<void(const Result &, int32_t)>;

    void SetConfig(const MDnsConfig &config);
    const MDnsConfig &GetConfig() const;

    void SetHandler(const Handler &handler);
    int32_t Register(const Result &info);
    int32_t Discovery(const std::string &serviceType);
    int32_t ResolveInstance(const std::string &instance);
    int32_t Resolve(const std::string &domain);

    int32_t UnRegister(const std::string &key);
    int32_t StopDiscovery(const std::string &key);
    int32_t StopResolveInstance(const std::string &key);
    int32_t StopResolve(const std::string &key);
    int32_t Stop(const std::string &key);

    bool IsNameValid(const std::string &name);
    bool IsTypeValid(const std::string &type);
    bool IsPortValid(int port);
    bool IsInstanceValid(const std::string &instance);
    bool IsDomainValid(const std::string &domain);
    void ExtractNameAndType(const std::string &instance, std::string &name, std::string type);

private:
    void Init();
    int32_t Announce(const Result &info, bool off);
    void ReceivePacket(int sock, const MDnsPayload &payload);
    void ProcessQuestion(int sock, const MDnsMessage &msg);
    void ProcessQuestionRecord(const std::any &anyAddr, const DNSProto::RRType &anyAddrType,
                               const DNSProto::Question &qu, int &phase, MDnsMessage &response);
    void ProcessAnswer(int sock, const MDnsMessage &msg);
    void ProcessAnswerRecord(bool v6, const DNSProto::ResourceRecord &rr, std::vector<Result> &matches,
                             std::map<std::string, Result> &results, std::map<std::string, std::string> &needMore);
    void ExtractNameAndType(const std::string &instance, Result &result);
    void AppendRecord(std::vector<DNSProto::ResourceRecord> &rrlist, DNSProto::RRType type, const std::string &name,
                      const std::any &rdata);

    std::string ExtractInstance(const Result &info) const;
    std::string Decorated(const std::string &name) const;
    std::string UnDecorated(const std::string &name) const;
    std::string Dotted(const std::string &name) const;
    std::string UnDotted(const std::string &name) const;
    std::string GetHostDomain();

    MDnsConfig config_;
    MDnsSocketListener listener_;
    Handler handler_;
    std::map<std::string, Result> srvMap_;
    std::map<std::string, int> reqMap_;
    std::atomic_int reqCount_ = 0;
    std::mutex mutex_;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif
