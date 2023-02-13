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

#include "mdns_protocol_impl.h"

#include <arpa/inet.h>
#include <iostream>
#include <numeric>
#include <random>
#include <unistd.h>

#include "mdns_packet_parser.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {

constexpr uint32_t DEFAULT_TTL = 120;
constexpr uint16_t MDNS_FLUSH_CACHE_BIT = 0x8000;

constexpr const char *MDNS_TYPE_TCP = "_tcp";
constexpr const char *MDNS_TYPE_UDP = "_udp";
constexpr const char *MDNS_TYPE_PREFIX = "_";
constexpr size_t MDNS_TYPE_SEGMENT = 2;
constexpr size_t MDNS_INSTANCE_SEGMENT = 3;
constexpr size_t MDNS_FULL_SEGMENT = 4;
constexpr size_t MDNS_NAME_IDX = 0;
constexpr size_t MDNS_TYPE1_IDX = 1;
constexpr size_t MDNS_TYPE2_IDX = 2;

constexpr int PHASE_PTR = 1;
constexpr int PHASE_SRV = 2;
constexpr int PHASE_DOMAIN = 3;

bool EndsWith(const std::string_view &str, const std::string_view &pat)
{
    return std::mismatch(pat.rbegin(), pat.rend(), str.rbegin()).first == pat.rend();
}

bool StartsWith(const std::string_view &str, const std::string_view &pat)
{
    return std::mismatch(pat.begin(), pat.end(), str.begin()).first == pat.end();
}

std::string AddrToString(const std::any &addr)
{
    char buf[INET6_ADDRSTRLEN] = {0};
    if (std::any_cast<in_addr>(&addr)) {
        if (inet_ntop(AF_INET, std::any_cast<in_addr>(&addr), buf, sizeof(buf)) == nullptr) {
            return std::string{};
        }
    } else if (std::any_cast<in6_addr>(&addr)) {
        if (inet_ntop(AF_INET6, std::any_cast<in6_addr>(&addr), buf, sizeof(buf)) == nullptr) {
            return std::string{};
        }
    } else {
        return std::string{};
    }
    return std::string(buf);
}

std::vector<std::string_view> Split(const std::string_view &s, char seperator)
{
    std::vector<std::string_view> output;
    std::string::size_type prev = 0;
    std::string::size_type pos = 0;
    while ((pos = s.find(seperator, pos)) != std::string::npos) {
        if (pos - prev > 0) {
            output.push_back(s.substr(prev, pos - prev));
        }
        prev = ++pos;
    }
    if (prev < s.size()) {
        output.push_back(s.substr(prev));
    }
    return output;
}

} // namespace

MDnsProtocolImpl::MDnsProtocolImpl()
{
    Init();
}

void MDnsProtocolImpl::Init()
{
    listener_.CloseAllSocket();
    if (config_.configAllIface) {
        listener_.OpenSocketForEachIface(config_.ipv6Support);
    } else {
        listener_.OpenSocketForDefault(config_.ipv6Support);
    }
    listener_.SetReciver([this](int sock, const MDnsPayload &payload) { return this->ReceivePacket(sock, payload); });
}

void MDnsProtocolImpl::SetConfig(const MDnsConfig &config)
{
    config_ = config;
}

const MDnsConfig &MDnsProtocolImpl::GetConfig() const
{
    return config_;
}

void MDnsProtocolImpl::SetHandler(const Handler &handler)
{
    handler_ = handler;
}

std::string MDnsProtocolImpl::Decorated(const std::string &name) const
{
    return name + config_.topDomain + MDNS_DOMAIN_SPLITER_STR;
}

std::string MDnsProtocolImpl::UnDecorated(const std::string &name) const
{
    return name.substr(0, name.size() - (config_.topDomain.size() + 1)); // Not check suffix, carefully
}

std::string MDnsProtocolImpl::Dotted(const std::string &name) const
{
    return EndsWith(name, MDNS_DOMAIN_SPLITER_STR) ? name : name + MDNS_DOMAIN_SPLITER_STR;
}

std::string MDnsProtocolImpl::UnDotted(const std::string &name) const
{
    return EndsWith(name, MDNS_DOMAIN_SPLITER_STR) ? name.substr(0, name.size() - 1) : name;
}

std::string MDnsProtocolImpl::ExtractInstance(const Result &info) const
{
    return Decorated(info.serviceName + MDNS_DOMAIN_SPLITER_STR + info.serviceType);
}

int32_t MDnsProtocolImpl::Register(const Result &info)
{
    if (!(IsNameValid(info.serviceName) && IsTypeValid(info.serviceType) && IsPortValid(info.port))) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }

    std::string name = ExtractInstance(info);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (srvMap_.find(name) != srvMap_.end()) {
            return NET_MDNS_ERR_SERVICE_INSTANCE_DUPLICATE;
        }
        srvMap_.emplace(name, info);
    }

    listener_.Start();
    return Announce(info, false);
}

int32_t MDnsProtocolImpl::Discovery(const std::string &serviceType)
{
    if (!IsTypeValid(serviceType)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = Decorated(serviceType);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        ++reqMap_[name];
        ++reqCount_;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg{};
    msg.questions.emplace_back(DNSProto::Question{
        .name = name,
        .qtype = DNSProto::RRTYPE_PTR,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.header.qdcount = msg.questions.size();

    ssize_t size = listener_.MulticastAll(parser.ToBytes(msg));
    listener_.Start();

    return size == -1 ? NET_MDNS_ERR_SEND : NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsProtocolImpl::ResolveInstance(const std::string &instance)
{
    if (!IsInstanceValid(instance)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = Decorated(instance);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        ++reqMap_[name];
        ++reqCount_;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg{};
    msg.questions.emplace_back(DNSProto::Question{
        .name = name,
        .qtype = DNSProto::RRTYPE_SRV,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.questions.emplace_back(DNSProto::Question{
        .name = name,
        .qtype = DNSProto::RRTYPE_TXT,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.header.qdcount = msg.questions.size();

    ssize_t size = listener_.MulticastAll(parser.ToBytes(msg));
    listener_.Start();

    return size == -1 ? NET_MDNS_ERR_SEND : NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsProtocolImpl::Resolve(const std::string &domain)
{
    if (!IsDomainValid(domain)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = Dotted(domain);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        ++reqMap_[name];
        ++reqCount_;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg{};
    msg.questions.emplace_back(DNSProto::Question{
        .name = name,
        .qtype = DNSProto::RRTYPE_A,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.questions.emplace_back(DNSProto::Question{
        .name = name,
        .qtype = DNSProto::RRTYPE_AAAA,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.header.qdcount = msg.questions.size();

    ssize_t size = listener_.MulticastAll(parser.ToBytes(msg));
    listener_.Start();

    return size == -1 ? NET_MDNS_ERR_SEND : NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsProtocolImpl::UnRegister(const std::string &key)
{
    std::string name = Decorated(key);
    std::lock_guard<std::mutex> guard(mutex_);
    if (srvMap_.find(name) != srvMap_.end()) {
        Announce(srvMap_[name], true);
        srvMap_.erase(name);
        return NETMANAGER_EXT_SUCCESS;
    }
    return NET_MDNS_ERR_SERVICE_INSTANCE_NOT_FOUND;
}

int32_t MDnsProtocolImpl::StopDiscovery(const std::string &key)
{
    return Stop(Decorated(key));
}

int32_t MDnsProtocolImpl::StopResolveInstance(const std::string &key)
{
    return Stop(Decorated(key));
}

int32_t MDnsProtocolImpl::StopResolve(const std::string &key)
{
    return Stop(Dotted(key));
}

int32_t MDnsProtocolImpl::Stop(const std::string &key)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (reqMap_.find(key) != reqMap_.end() && reqMap_[key] > 0) {
        --reqMap_[key];
        --reqCount_;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t MDnsProtocolImpl::Announce(const Result &info, bool off)
{
    MDnsMessage response{};
    response.header.flags = DNSProto::MDNS_ANSWER_FLAGS;
    std::string name = Decorated(info.serviceName + MDNS_DOMAIN_SPLITER_STR + info.serviceType);
    response.answers.emplace_back(DNSProto::ResourceRecord{.name = Decorated(info.serviceType),
                                                           .rtype = static_cast<uint16_t>(DNSProto::RRTYPE_PTR),
                                                           .rclass = DNSProto::RRCLASS_IN | MDNS_FLUSH_CACHE_BIT,
                                                           .ttl = off ? 0U : DEFAULT_TTL,
                                                           .rdata = name});
    response.answers.emplace_back(DNSProto::ResourceRecord{.name = name,
                                                           .rtype = static_cast<uint16_t>(DNSProto::RRTYPE_SRV),
                                                           .rclass = DNSProto::RRCLASS_IN | MDNS_FLUSH_CACHE_BIT,
                                                           .ttl = off ? 0U : DEFAULT_TTL,
                                                           .rdata = DNSProto::RDataSrv{
                                                               .priority = 0,
                                                               .weight = 0,
                                                               .port = static_cast<uint16_t>(info.port),
                                                               .name = GetHostDomain(),
                                                           }});
    response.answers.emplace_back(DNSProto::ResourceRecord{.name = name,
                                                           .rtype = static_cast<uint16_t>(DNSProto::RRTYPE_TXT),
                                                           .rclass = DNSProto::RRCLASS_IN | MDNS_FLUSH_CACHE_BIT,
                                                           .ttl = off ? 0U : DEFAULT_TTL,
                                                           .rdata = info.txt});
    MDnsPayloadParser parser;
    ssize_t size = listener_.MulticastAll(parser.ToBytes(response));
    return size == -1 ? NET_MDNS_ERR_SEND : NETMANAGER_EXT_SUCCESS;
}

void MDnsProtocolImpl::ReceivePacket(int sock, const MDnsPayload &payload)
{
    if (payload.size() == 0) {
        NETMGR_EXT_LOG_W("empty payload received");
        return;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg = parser.FromBytes(payload);
    if (parser.GetError() != 0) {
        NETMGR_EXT_LOG_W("payload parse failed");
        return;
    }
    if ((msg.header.flags & DNSProto::HEADER_FLAGS_QR_MASK) == 0) {
        ProcessQuestion(sock, msg);
    } else {
        ProcessAnswer(sock, msg);
    }
}

void MDnsProtocolImpl::AppendRecord(std::vector<DNSProto::ResourceRecord> &rrlist, DNSProto::RRType type,
                                    const std::string &name, const std::any &rdata)
{
    rrlist.emplace_back(DNSProto::ResourceRecord{.name = name,
                                                 .rtype = static_cast<uint16_t>(type),
                                                 .rclass = DNSProto::RRCLASS_IN | MDNS_FLUSH_CACHE_BIT,
                                                 .ttl = DEFAULT_TTL,
                                                 .rdata = rdata});
}

void MDnsProtocolImpl::ProcessQuestion(int sock, const MDnsMessage &msg)
{
    const sockaddr *saddrIf = listener_.GetSockAddr(sock);
    if (saddrIf == nullptr) {
        return;
    }
    std::any anyAddr;
    DNSProto::RRType anyAddrType;
    if (saddrIf->sa_family == AF_INET6) {
        anyAddr = reinterpret_cast<const sockaddr_in6 *>(saddrIf)->sin6_addr;
        anyAddrType = DNSProto::RRTYPE_AAAA;
    } else {
        anyAddr = reinterpret_cast<const sockaddr_in *>(saddrIf)->sin_addr;
        anyAddrType = DNSProto::RRTYPE_A;
    }
    int phase = 0;
    MDnsMessage response{};
    response.header.flags = DNSProto::MDNS_ANSWER_FLAGS;
    for (size_t i = 0; i < msg.header.qdcount; ++i) {
        ProcessQuestionRecord(anyAddr, anyAddrType, msg.questions[i], phase, response);
    }
    if (phase == PHASE_SRV) {
        AppendRecord(response.additional, anyAddrType, GetHostDomain(), anyAddr);
    }
    if (phase != 0 && response.answers.size() > 0) {
        listener_.Multicast(sock, MDnsPayloadParser().ToBytes(response));
    }
}

void MDnsProtocolImpl::ProcessQuestionRecord(const std::any &anyAddr, const DNSProto::RRType &anyAddrType,
                                             const DNSProto::Question &qu, int &phase, MDnsMessage &response)
{
    std::string name = qu.name;
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_PTR) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::for_each(srvMap_.begin(), srvMap_.end(), [&](const auto &elem) -> void {
            if (EndsWith(elem.first, name)) {
                AppendRecord(response.answers, DNSProto::RRTYPE_PTR, name, elem.first);
            }
        });
        phase = std::max(phase, PHASE_PTR);
    }
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_SRV) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter = srvMap_.find(name);
        if (iter == srvMap_.end()) {
            return;
        }
        AppendRecord(response.answers, DNSProto::RRTYPE_SRV, name,
                     DNSProto::RDataSrv{
                         .priority = 0,
                         .weight = 0,
                         .port = static_cast<uint16_t>(iter->second.port),
                         .name = GetHostDomain(),
                     });
        phase = std::max(phase, PHASE_SRV);
    }
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_TXT) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto iter = srvMap_.find(name);
        if (iter == srvMap_.end()) {
            return;
        }
        AppendRecord(response.answers, DNSProto::RRTYPE_TXT, name, iter->second.txt);
        phase = std::max(phase, PHASE_SRV);
    }
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_A || qu.qtype == DNSProto::RRTYPE_AAAA) {
        if (name != GetHostDomain() || (qu.qtype != DNSProto::RRTYPE_ANY && anyAddrType != qu.qtype)) {
            return;
        }
        AppendRecord(response.answers, anyAddrType, name, anyAddr);
        phase = std::max(phase, PHASE_DOMAIN);
    }
}

void MDnsProtocolImpl::ProcessAnswer(int sock, const MDnsMessage &msg)
{
    const sockaddr *saddrIf = listener_.GetSockAddr(sock);
    if (saddrIf == nullptr) {
        return;
    }
    bool v6 = (saddrIf->sa_family == AF_INET6);

    std::vector<Result> matches;
    std::map<std::string, Result> results;
    std::map<std::string, std::string> needMore;
    for (size_t i = 0; i < msg.answers.size(); ++i) {
        ProcessAnswerRecord(v6, msg.answers[i], matches, results, needMore);
    }

    for (size_t i = 0; i < msg.additional.size() && !needMore.empty(); ++i) {
        std::string name = msg.additional[i].name;
        if (needMore.find(name) == needMore.end()) {
            continue;
        }
        if (msg.additional[i].rtype == DNSProto::RRTYPE_A || msg.additional[i].rtype == DNSProto::RRTYPE_AAAA) {
            if (v6 != (msg.additional[i].rtype == DNSProto::RRTYPE_AAAA)) {
                continue;
            }
            Result &result = results[needMore[name]];
            result.domain = UnDotted(name);
            result.ipv6 = (msg.additional[i].rtype == DNSProto::RRTYPE_AAAA);
            result.addr = AddrToString(msg.additional[i].rdata);
        }
    }

    for (auto i = matches.begin(); i != matches.end() && handler_ != nullptr; ++i) {
        handler_(*i, NETMANAGER_EXT_SUCCESS);
    }
    for (auto i = results.begin(); i != results.end() && handler_ != nullptr; ++i) {
        i->second.iface = listener_.GetIface(sock);
        i->second.ipv6 = v6;
        handler_(i->second, NETMANAGER_EXT_SUCCESS);
    }
}

void MDnsProtocolImpl::ProcessAnswerRecord(bool v6, const DNSProto::ResourceRecord &rr, std::vector<Result> &matches,
                                           std::map<std::string, Result> &results,
                                           std::map<std::string, std::string> &needMore)
{
    std::string name = rr.name;
    mutex_.lock();
    if (reqMap_[name] <= 0) {
        return mutex_.unlock();
    }
    mutex_.unlock();
    if (rr.rtype == DNSProto::RRTYPE_PTR) {
        const std::string *data = std::any_cast<std::string>(&rr.rdata);
        if (data == nullptr) {
            return;
        }
        Result result;
        result.type = (rr.ttl == 0) ? SERVICE_LOST : SERVICE_FOUND;
        ExtractNameAndType(*data, result);
        if (std::find_if(matches.begin(), matches.end(), [&](const auto &elem) {
                return elem.serviceName == result.serviceName && elem.serviceType == result.serviceType;
            }) == matches.end()) {
            matches.emplace_back(std::move(result));
        }
    } else if (rr.rtype == DNSProto::RRTYPE_SRV) {
        const DNSProto::RDataSrv *srv = std::any_cast<DNSProto::RDataSrv>(&rr.rdata);
        if (rr.ttl == 0 || srv == nullptr) {
            return;
        }
        Result &result = results[name];
        result.type = INSTANCE_RESOLVED;
        ExtractNameAndType(name, result);
        result.domain = UnDotted(srv->name);
        result.port = srv->port;
        needMore[srv->name] = name;
    } else if (rr.rtype == DNSProto::RRTYPE_TXT) {
        const TxtRecordEncoded *txt = std::any_cast<TxtRecordEncoded>(&rr.rdata);
        if (rr.ttl == 0 || txt == nullptr) {
            return;
        }
        Result &result = results[name];
        result.txt = *txt;
    } else if (rr.rtype == DNSProto::RRTYPE_A || rr.rtype == DNSProto::RRTYPE_AAAA) {
        if (rr.ttl == 0 || v6 != (rr.rtype == DNSProto::RRTYPE_AAAA)) {
            return;
        }
        Result &result = results[name];
        result.type = DOMAIN_RESOLVED;
        result.domain = UnDotted(name);
        result.ipv6 = (rr.rtype == DNSProto::RRTYPE_AAAA);
        result.addr = AddrToString(rr.rdata);
    } else {
        NETMGR_EXT_LOG_D("Unknown packet received, type=[%{public}d]", rr.rtype);
    }
}

void MDnsProtocolImpl::ExtractNameAndType(const std::string &instance, Result &result)
{
    auto views = Split(instance, MDNS_DOMAIN_SPLITER);
    if (views.size() != MDNS_FULL_SEGMENT) {
        return;
    }
    result.serviceName = std::string(views[MDNS_NAME_IDX].begin(), views[MDNS_NAME_IDX].end());
    result.serviceType = std::string(views[MDNS_TYPE1_IDX].begin(), views[MDNS_TYPE2_IDX].end());
}

void MDnsProtocolImpl::ExtractNameAndType(const std::string &instance, std::string &name, std::string type)
{
    auto views = Split(instance, MDNS_DOMAIN_SPLITER);
    if (views.size() != MDNS_INSTANCE_SEGMENT) {
        return;
    }
    name = std::string(views[MDNS_NAME_IDX].begin(), views[MDNS_NAME_IDX].end());
    type = std::string(views[MDNS_TYPE1_IDX].begin(), views[MDNS_TYPE2_IDX].end());
}

std::string MDnsProtocolImpl::GetHostDomain()
{
    if (config_.hostname.empty()) {
        char buffer[MDNS_MAX_DOMAIN_LABEL];
        if (gethostname(buffer, sizeof(buffer)) == 0) {
            config_.hostname = buffer;
            static auto uid = []() {
                std::random_device rd;
                return rd();
            }();
            config_.hostname += std::to_string(uid);
        }
    }
    return Decorated(config_.hostname);
}

bool MDnsProtocolImpl::IsNameValid(const std::string &name)
{
    return 0 < name.size() && name.size() <= MDNS_MAX_DOMAIN_LABEL &&
           name.find(MDNS_DOMAIN_SPLITER) == std::string::npos;
}

bool MDnsProtocolImpl::IsTypeValid(const std::string &type)
{
    auto views = Split(type, MDNS_DOMAIN_SPLITER);
    return views.size() == MDNS_TYPE_SEGMENT && views[0].size() <= MDNS_MAX_DOMAIN_LABEL &&
           StartsWith(views[0], MDNS_TYPE_PREFIX) && (views[1] == MDNS_TYPE_UDP || views[1] == MDNS_TYPE_TCP);
}

bool MDnsProtocolImpl::IsPortValid(int port)
{
    return 0 <= port && port <= UINT16_MAX;
}

bool MDnsProtocolImpl::IsInstanceValid(const std::string &instance)
{
    auto views = Split(instance, MDNS_DOMAIN_SPLITER);
    return views.size() == MDNS_INSTANCE_SEGMENT && views[MDNS_NAME_IDX].size() <= MDNS_MAX_DOMAIN_LABEL &&
           views[MDNS_TYPE1_IDX].size() <= MDNS_MAX_DOMAIN_LABEL &&
           StartsWith(views[MDNS_TYPE1_IDX], MDNS_TYPE_PREFIX) &&
           (views[MDNS_TYPE2_IDX] == MDNS_TYPE_UDP || views[MDNS_TYPE2_IDX] == MDNS_TYPE_TCP);
}

bool MDnsProtocolImpl::IsDomainValid(const std::string &domain)
{
    return true;
}

} // namespace NetManagerStandard
} // namespace OHOS
