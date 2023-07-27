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
#include <cstddef>
#include <iostream>
#include <random>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "mdns_manager.h"
#include "mdns_packet_parser.h"
#include "net_conn_client.h"
#include "netmgr_ext_log_wrapper.h"

#include "securec.h"

namespace OHOS {
namespace NetManagerStandard {

constexpr uint32_t DEFAULT_INTEVAL_MS = 2000;
constexpr uint32_t DEFAULT_LOST_MS = 10000;
constexpr uint32_t DEFAULT_TTL = 120;
constexpr uint16_t MDNS_FLUSH_CACHE_BIT = 0x8000;

constexpr int PHASE_PTR = 1;
constexpr int PHASE_SRV = 2;
constexpr int PHASE_DOMAIN = 3;

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
    }
    return std::string(buf);
}

int64_t MilliSecondsSinceEpoch()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

MDnsProtocolImpl::MDnsProtocolImpl()
{
    Init();
}

void MDnsProtocolImpl::Init()
{
    NETMGR_EXT_LOG_D("mdns_log MDnsProtocolImpl init");
    listener_.Stop();
    listener_.CloseAllSocket();

    if (config_.configAllIface) {
        listener_.OpenSocketForEachIface(config_.ipv6Support, config_.configLo);
    } else {
        listener_.OpenSocketForDefault(config_.ipv6Support);
    }
    listener_.SetReceiveHandler(
        [this](int sock, const MDnsPayload &payload) { return this->ReceivePacket(sock, payload); });
    listener_.SetFinishedHandler([this](int sock) {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        RunTaskQueue(taskQueue_);
    });
    listener_.Start();

    taskQueue_.clear();
    taskOnChange_.clear();
    AddTask([this]() { return Browse(); }, false);
}

bool MDnsProtocolImpl::Browse()
{
    if (lastRunTime != -1 && MilliSecondsSinceEpoch() - lastRunTime < DEFAULT_INTEVAL_MS) {
        return false;
    }
    lastRunTime = MilliSecondsSinceEpoch();
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    for (auto &&[key, res] : browserMap_) {
        NETMGR_EXT_LOG_D("mdns_log Browse browserMap_ key[%{public}s] res.size[%{public}zu]", key.c_str(), res.size());
        if (nameCbMap_.find(key) != nameCbMap_.end() &&
            !MDnsManager::GetInstance().IsAvailableCallback(nameCbMap_[key])) {
            continue;
        }
        handleOfflineService(key, res);
        MDnsPayloadParser parser;
        MDnsMessage msg{};
        msg.questions.emplace_back(DNSProto::Question{
            .name = key,
            .qtype = DNSProto::RRTYPE_PTR,
            .qclass = DNSProto::RRCLASS_IN,
        });
        listener_.MulticastAll(parser.ToBytes(msg));
    }
    return false;
}

int32_t MDnsProtocolImpl::ConnectControl(int32_t sockfd, sockaddr* serverAddr)
{
    uint32_t flags = static_cast<uint32_t>(fcntl(sockfd, F_GETFL, 0));
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    int32_t ret = connect(sockfd, serverAddr, sizeof(sockaddr));
    if ((ret < 0) && (errno != EINPROGRESS)) {
        NETMGR_EXT_LOG_E("connect error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (ret == 0) {
        fcntl(sockfd, F_SETFL, flags); /* restore file status flags */
        NETMGR_EXT_LOG_I("connect success.");
        return NETMANAGER_EXT_SUCCESS;
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    fd_set wset = rset;
    timeval tval {1, 0};
    ret = select(sockfd + 1, &rset, &wset, NULL, &tval);
    if (ret < 0) { // select error.
        NETMGR_EXT_LOG_E("select error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (ret == 0) { // timeout
        NETMGR_EXT_LOG_E("connect timeout...");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (!FD_ISSET(sockfd, &rset) && !FD_ISSET(sockfd, &wset)) {
        NETMGR_EXT_LOG_E("select error: sockfd not set");
        return NETMANAGER_EXT_ERR_INTERNAL;
    }

    int32_t result = NETMANAGER_EXT_ERR_INTERNAL;
    socklen_t len = sizeof(result);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &result, &len) < 0) {
        NETMGR_EXT_LOG_E("getsockopt error: %{public}d", errno);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    if (result != 0) { // connect failed.
        NETMGR_EXT_LOG_E("connect failed. error: %{public}d", result);
        return NETMANAGER_EXT_ERR_INTERNAL;
    }
    fcntl(sockfd, F_SETFL, flags); /* restore file status flags */
    NETMGR_EXT_LOG_I("lost but connect success.");
    return NETMANAGER_EXT_SUCCESS;
}

bool MDnsProtocolImpl::IsConnectivity(const std::string &ip, int32_t port)
{
    if (ip.empty()) {
        NETMGR_EXT_LOG_E("ip is empty");
        return false;
    }

    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        NETMGR_EXT_LOG_E("create socket error: %{public}d", errno);
        return false;
    }

    struct sockaddr_in serverAddr;
    if (memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr)) != EOK) {
        NETMGR_EXT_LOG_E("memset_s serverAddr failed!");
        close(sockfd);
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddr.sin_port = htons(port);
    if (ConnectControl(sockfd, (struct sockaddr*)&serverAddr) != NETMANAGER_EXT_SUCCESS) {
        NETMGR_EXT_LOG_I("connect error: %{public}d", errno);
        close(sockfd);
        return false;
    }

    close(sockfd);
    return true;
}

void MDnsProtocolImpl::handleOfflineService(const std::string &key, std::vector<Result> &res)
{
    NETMGR_EXT_LOG_D("mdns_log handleOfflineService key:[%{public}s]", key.c_str());
    for (auto it = res.begin(); it != res.end();) {
        if (lastRunTime - it->refrehTime > DEFAULT_LOST_MS && it->state == State::LIVE) {
            std::string fullName = Decorated(it->serviceName + MDNS_DOMAIN_SPLITER_STR + it->serviceType);
            if ((cacheMap_.find(fullName) != cacheMap_.end()) &&
                IsConnectivity(cacheMap_[fullName].addr, cacheMap_[fullName].port)) {
                it++;
                continue;
            }

            it->state = State::DEAD;
            if (nameCbMap_.find(key) != nameCbMap_.end() && nameCbMap_[key] != nullptr) {
                NETMGR_EXT_LOG_W("mdns_log HandleServiceLost");
                nameCbMap_[key]->HandleServiceLost(ConvertResultToInfo(*it), NETMANAGER_EXT_SUCCESS);
            }
            it = res.erase(it);
            cacheMap_.erase(fullName);
        } else {
            it++;
        }
    }
}

void MDnsProtocolImpl::SetConfig(const MDnsConfig &config)
{
    config_ = config;
}

const MDnsConfig &MDnsProtocolImpl::GetConfig() const
{
    return config_;
}

std::string MDnsProtocolImpl::Decorated(const std::string &name) const
{
    return name + config_.topDomain;
}

int32_t MDnsProtocolImpl::Register(const Result &info)
{
    NETMGR_EXT_LOG_D("mdns_log Register");
    if (!(IsNameValid(info.serviceName) && IsTypeValid(info.serviceType) && IsPortValid(info.port))) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = Decorated(info.serviceName + MDNS_DOMAIN_SPLITER_STR + info.serviceType);
    if (!IsDomainValid(name)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        if (srvMap_.find(name) != srvMap_.end()) {
            return NET_MDNS_ERR_SERVICE_INSTANCE_DUPLICATE;
        }
        srvMap_.emplace(name, info);
    }
    return Announce(info, false);
}

int32_t MDnsProtocolImpl::UnRegister(const std::string &key)
{
    NETMGR_EXT_LOG_D("mdns_log UnRegister");
    std::string name = Decorated(key);
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (srvMap_.find(name) != srvMap_.end()) {
        Announce(srvMap_[name], true);
        srvMap_.erase(name);
        return NETMANAGER_EXT_SUCCESS;
    }
    return NET_MDNS_ERR_SERVICE_INSTANCE_NOT_FOUND;
}

bool MDnsProtocolImpl::DiscoveryFromCache(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log DiscoveryFromCache");
    std::string name = Decorated(serviceType);
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (!IsBrowserAvailable(name)) {
        return false;
    }

    if (browserMap_.find(name) == browserMap_.end()) {
        NETMGR_EXT_LOG_D("mdns_log DiscoveryFromCache browserMap_ not find name");
        return false;
    }

    for (auto &res : browserMap_[name]) {
        if (res.state == State::REMOVE || res.state == State::DEAD) {
            continue;
        }
        AddTask([cb, info = ConvertResultToInfo(res)]() {
            NETMGR_EXT_LOG_W("mdns_log DiscoveryFromCache ConvertResultToInfo HandleServiceFound");
            if (MDnsManager::GetInstance().IsAvailableCallback(cb)) {
                cb->HandleServiceFound(info, NETMANAGER_EXT_SUCCESS);
            }
            return true;
        });
    }
    return true;
}

bool MDnsProtocolImpl::DiscoveryFromNet(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log DiscoveryFromNet");
    std::string name = Decorated(serviceType);
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    browserMap_.insert({name, std::vector<Result>{}});
    nameCbMap_[name] = cb;
    // key is serviceTYpe
    AddEvent(name, [this, name, cb]() {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        if (!IsBrowserAvailable(name)) {
            return false;
        }
        if (!MDnsManager::GetInstance().IsAvailableCallback(cb)) {
            return true;
        }
        for (auto &res : browserMap_[name]) {
            std::string fullName = Decorated(res.serviceName + MDNS_DOMAIN_SPLITER_STR + res.serviceType);
            NETMGR_EXT_LOG_W("mdns_log DiscoveryFromNet name:[%{public}s] fullName:[%{public}s]", name.c_str(),
                             fullName.c_str());
            if (cacheMap_.find(fullName) == cacheMap_.end() ||
                (res.state == State::ADD || res.state == State::REFRESH)) {
                NETMGR_EXT_LOG_W("mdns_log HandleServiceFound");
                cb->HandleServiceFound(ConvertResultToInfo(res), NETMANAGER_EXT_SUCCESS);
                res.state = State::LIVE;
            }
            if (res.state == State::REMOVE) {
                res.state = State::DEAD;
                NETMGR_EXT_LOG_D("mdns_log HandleServiceLost");
                cb->HandleServiceLost(ConvertResultToInfo(res), NETMANAGER_EXT_SUCCESS);
                if (cacheMap_.find(fullName) != cacheMap_.end()) {
                    res.state = State::ADD;
                    cacheMap_.erase(fullName);
                }
            }
        }
        return false;
    });

    AddTask([=]() {
            MDnsPayloadParser parser;
            MDnsMessage msg{};
            msg.questions.emplace_back(DNSProto::Question{
                .name = name,
                .qtype = DNSProto::RRTYPE_PTR,
                .qclass = DNSProto::RRCLASS_IN,
            });
            listener_.MulticastAll(parser.ToBytes(msg));
            return true;
        }, false);
    return true;
}

int32_t MDnsProtocolImpl::Discovery(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log Discovery");
    DiscoveryFromCache(serviceType, cb);
    DiscoveryFromNet(serviceType, cb);
    return NETMANAGER_EXT_SUCCESS;
}

bool MDnsProtocolImpl::ResolveInstanceFromCache(const std::string &name, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log ResolveInstanceFromCache");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (!IsInstanceCacheAvailable(name)) {
        NETMGR_EXT_LOG_W("mdns_log ResolveInstanceFromCache cacheMap_ has no element [%{public}s]", name.c_str());
        return false;
    }

    NETMGR_EXT_LOG_I("mdns_log rr.name : [%{public}s]", name.c_str());
    Result r = cacheMap_[name];
    if (IsDomainCacheAvailable(r.domain)) {
        r.ipv6 = cacheMap_[r.domain].ipv6;
        r.addr = cacheMap_[r.domain].addr;

        NETMGR_EXT_LOG_D("mdns_log Add Task DomainCache Available, [%{public}s]", r.domain.c_str());
        AddTask([cb, info = ConvertResultToInfo(r)]() {
            if (nullptr != cb) {
                cb->HandleResolveResult(info, NETMANAGER_EXT_SUCCESS);
            }
            return true;
        });
    } else {
        ResolveFromNet(r.domain, nullptr);
        NETMGR_EXT_LOG_D("mdns_log Add Event DomainCache UnAvailable, [%{public}s]", r.domain.c_str());
        AddEvent(r.domain, [this, cb, r]() mutable {
            if (!IsDomainCacheAvailable(r.domain)) {
                return false;
            }
            r.ipv6 = cacheMap_[r.domain].ipv6;
            r.addr = cacheMap_[r.domain].addr;
            if (nullptr != cb) {
                cb->HandleResolveResult(ConvertResultToInfo(r), NETMANAGER_EXT_SUCCESS);
            }
            return true;
        });
    }
    return true;
}

bool MDnsProtocolImpl::ResolveInstanceFromNet(const std::string &name, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log ResolveInstanceFromNet");
    {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        cacheMap_[name].state = State::ADD;
        ExtractNameAndType(name, cacheMap_[name].serviceName, cacheMap_[name].serviceType);
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
    AddEvent(name, [this, name, cb]() { return ResolveInstanceFromCache(name, cb); });
    ssize_t size = listener_.MulticastAll(parser.ToBytes(msg));
    return size > 0;
}

bool MDnsProtocolImpl::ResolveFromCache(const std::string &domain, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log ResolveFromCache");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    if (!IsDomainCacheAvailable(domain)) {
        return false;
    }
    AddTask([this, cb, info = ConvertResultToInfo(cacheMap_[domain])]() {
        if (nullptr != cb) {
            cb->HandleResolveResult(info, NETMANAGER_EXT_SUCCESS);
        }
        return true;
    });
    return true;
}

bool MDnsProtocolImpl::ResolveFromNet(const std::string &domain, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log ResolveFromNet");
    {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        cacheMap_[domain];
        cacheMap_[domain].domain = domain;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg{};
    msg.questions.emplace_back(DNSProto::Question{
        .name = domain,
        .qtype = DNSProto::RRTYPE_A,
        .qclass = DNSProto::RRCLASS_IN,
    });
    msg.questions.emplace_back(DNSProto::Question{
        .name = domain,
        .qtype = DNSProto::RRTYPE_AAAA,
        .qclass = DNSProto::RRCLASS_IN,
    });
    // key is serviceName
    AddEvent(domain, [this, cb, domain]() { return ResolveFromCache(domain, cb); });
    ssize_t size = listener_.MulticastAll(parser.ToBytes(msg));
    return size > 0;
}

int32_t MDnsProtocolImpl::ResolveInstance(const std::string &instance, const sptr<IResolveCallback> &cb)
{
    NETMGR_EXT_LOG_D("mdns_log execute ResolveInstance");
    if (!IsInstanceValid(instance)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    std::string name = Decorated(instance);
    if (!IsDomainValid(name)) {
        return NET_MDNS_ERR_ILLEGAL_ARGUMENT;
    }
    if (ResolveInstanceFromCache(name, cb)) {
        return NETMANAGER_EXT_SUCCESS;
    }
    return ResolveInstanceFromNet(name, cb) ? NETMANAGER_EXT_SUCCESS : NET_MDNS_ERR_SEND;
}

int32_t MDnsProtocolImpl::Announce(const Result &info, bool off)
{
    NETMGR_EXT_LOG_I("mdns_log Announce message");
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
    return size > 0 ? NETMANAGER_EXT_SUCCESS : NET_MDNS_ERR_SEND;
}

void MDnsProtocolImpl::ReceivePacket(int sock, const MDnsPayload &payload)
{
    if (payload.size() == 0) {
        return;
    }
    MDnsPayloadParser parser;
    MDnsMessage msg = parser.FromBytes(payload);
    if (parser.GetError() != 0) {
        NETMGR_EXT_LOG_E("parser payload failed");
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
        NETMGR_EXT_LOG_W("mdns_log ProcessQuestion saddrIf is null");
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
    if (phase < PHASE_DOMAIN) {
        AppendRecord(response.additional, anyAddrType, GetHostDomain(), anyAddr);
    }

    if (phase != 0 && response.answers.size() > 0) {
        listener_.Multicast(sock, MDnsPayloadParser().ToBytes(response));
    }
}

void MDnsProtocolImpl::ProcessQuestionRecord(const std::any &anyAddr, const DNSProto::RRType &anyAddrType,
                                             const DNSProto::Question &qu, int &phase, MDnsMessage &response)
{
    NETMGR_EXT_LOG_D("mdns_log ProcessQuestionRecord");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    std::string name = qu.name;
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_PTR) {
        std::for_each(srvMap_.begin(), srvMap_.end(), [&](const auto &elem) -> void {
            if (EndsWith(elem.first, name)) {
                AppendRecord(response.answers, DNSProto::RRTYPE_PTR, name, elem.first);
                AppendRecord(response.additional, DNSProto::RRTYPE_SRV, elem.first,
                             DNSProto::RDataSrv{
                                 .priority = 0,
                                 .weight = 0,
                                 .port = static_cast<uint16_t>(elem.second.port),
                                 .name = GetHostDomain(),
                             });
                AppendRecord(response.additional, DNSProto::RRTYPE_TXT, elem.first, elem.second.txt);
            }
        });
        phase = std::max(phase, PHASE_PTR);
    }
    if (qu.qtype == DNSProto::RRTYPE_ANY || qu.qtype == DNSProto::RRTYPE_SRV) {
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
    std::set<std::string> changed;
    for (const auto &answer : msg.answers) {
        ProcessAnswerRecord(v6, answer, changed);
    }
    for (const auto &i : msg.additional) {
        ProcessAnswerRecord(v6, i, changed);
    }
    for (const auto &i : changed) {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        RunTaskQueue(taskOnChange_[i]);
        KillCache(i);
    }
}

void MDnsProtocolImpl::UpdatePtr(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed)
{
    const std::string *data = std::any_cast<std::string>(&rr.rdata);
    if (data == nullptr) {
        return;
    }

    std::string name = rr.name;
    if (browserMap_.find(name) == browserMap_.end()) {
        return;
    }
    auto &results = browserMap_[name];
    std::string srvName;
    std::string srvType;
    ExtractNameAndType(*data, srvName, srvType);
    if (srvName.empty() || srvType.empty()) {
        return;
    }
    auto res =
        std::find_if(results.begin(), results.end(), [&](const auto &elem) { return elem.serviceName == srvName; });
    if (res == results.end()) {
        results.emplace_back(Result{
            .serviceName = srvName,
            .serviceType = srvType,
            .state = State::ADD,
        });
    }
    res = std::find_if(results.begin(), results.end(), [&](const auto &elem) { return elem.serviceName == srvName; });
    if (res->serviceName != srvName || res->state == State::DEAD) {
        res->state = State::REFRESH;
        res->serviceName = srvName;
    }
    if (rr.ttl == 0) {
        res->state = State::REMOVE;
    }
    if (res->state != State::LIVE && res->state != State::DEAD) {
        changed.emplace(name);
    }
    res->ttl = rr.ttl;
    res->refrehTime = MilliSecondsSinceEpoch();
}

void MDnsProtocolImpl::UpdateSrv(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed)
{
    const DNSProto::RDataSrv *srv = std::any_cast<DNSProto::RDataSrv>(&rr.rdata);
    if (srv == nullptr) {
        return;
    }
    std::string name = rr.name;
    if (cacheMap_.find(name) == cacheMap_.end()) {
        ExtractNameAndType(name, cacheMap_[name].serviceName, cacheMap_[name].serviceType);
        cacheMap_[name].state = State::ADD;
        cacheMap_[name].domain = srv->name;
        cacheMap_[name].port = srv->port;
    }
    Result &result = cacheMap_[name];
    if (result.domain != srv->name || result.port != srv->port || result.state == State::DEAD) {
        if (result.state != State::ADD) {
            result.state = State::REFRESH;
        }
        result.domain = srv->name;
        result.port = srv->port;
    }
    if (rr.ttl == 0) {
        result.state = State::REMOVE;
    }
    if (result.state != State::LIVE && result.state != State::DEAD) {
        changed.emplace(name);
    }
    result.ttl = rr.ttl;
    result.refrehTime = MilliSecondsSinceEpoch();
}

void MDnsProtocolImpl::UpdateTxt(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed)
{
    const TxtRecordEncoded *txt = std::any_cast<TxtRecordEncoded>(&rr.rdata);
    if (txt == nullptr) {
        return;
    }
    std::string name = rr.name;
    if (cacheMap_.find(name) == cacheMap_.end()) {
        ExtractNameAndType(name, cacheMap_[name].serviceName, cacheMap_[name].serviceType);
        cacheMap_[name].state = State::ADD;
        cacheMap_[name].txt = *txt;
    }
    Result &result = cacheMap_[name];
    if (result.txt != *txt || result.state == State::DEAD) {
        if (result.state != State::ADD) {
            result.state = State::REFRESH;
        }
        result.txt = *txt;
    }
    if (rr.ttl == 0) {
        result.state = State::REMOVE;
    }
    if (result.state != State::LIVE && result.state != State::DEAD) {
        changed.emplace(name);
    }
    result.ttl = rr.ttl;
    result.refrehTime = MilliSecondsSinceEpoch();
}

void MDnsProtocolImpl::UpdateAddr(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed)
{
    if (v6 != (rr.rtype == DNSProto::RRTYPE_AAAA)) {
        return;
    }
    const std::string addr = AddrToString(rr.rdata);
    bool v6rr = (rr.rtype == DNSProto::RRTYPE_AAAA);
    if (addr.empty()) {
        return;
    }
    std::string name = rr.name;
    if (cacheMap_.find(name) == cacheMap_.end()) {
        ExtractNameAndType(name, cacheMap_[name].serviceName, cacheMap_[name].serviceType);
        cacheMap_[name].state = State::ADD;
        cacheMap_[name].ipv6 = v6rr;
        cacheMap_[name].addr = addr;
    }
    Result &result = cacheMap_[name];
    if (result.addr != addr || result.ipv6 != v6rr || result.state == State::DEAD) {
        result.state = State::REFRESH;
        result.addr = addr;
        result.ipv6 = v6rr;
    }
    if (rr.ttl == 0) {
        result.state = State::REMOVE;
    }
    if (result.state != State::LIVE && result.state != State::DEAD) {
        changed.emplace(name);
    }
    result.ttl = rr.ttl;
    result.refrehTime = MilliSecondsSinceEpoch();
}

void MDnsProtocolImpl::ProcessAnswerRecord(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed)
{
    NETMGR_EXT_LOG_D("mdns_log ProcessAnswerRecord, type=[%{public}d]", rr.rtype);
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    std::string name = rr.name;
    if (cacheMap_.find(name) == cacheMap_.end() && browserMap_.find(name) == browserMap_.end() &&
        srvMap_.find(name) != srvMap_.end()) {
        return;
    }
    if (rr.rtype == DNSProto::RRTYPE_PTR) {
        UpdatePtr(v6, rr, changed);
    } else if (rr.rtype == DNSProto::RRTYPE_SRV) {
        UpdateSrv(v6, rr, changed);
    } else if (rr.rtype == DNSProto::RRTYPE_TXT) {
        UpdateTxt(v6, rr, changed);
    } else if (rr.rtype == DNSProto::RRTYPE_A || rr.rtype == DNSProto::RRTYPE_AAAA) {
        UpdateAddr(v6, rr, changed);
    } else {
        NETMGR_EXT_LOG_D("mdns_log Unknown packet received, type=[%{public}d]", rr.rtype);
    }
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

void MDnsProtocolImpl::AddTask(const Task &task, bool atonce)
{
    {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        taskQueue_.emplace_back(task);
    }
    if (atonce) {
        listener_.TriggerRefresh();
    }
}

MDnsServiceInfo MDnsProtocolImpl::ConvertResultToInfo(const MDnsProtocolImpl::Result &result)
{
    MDnsServiceInfo info;
    info.name = result.serviceName;
    info.type = result.serviceType;
    if (!result.addr.empty()) {
        info.family = result.ipv6 ? MDnsServiceInfo::IPV6 : MDnsServiceInfo::IPV4;
    }
    info.addr = result.addr;
    info.port = result.port;
    info.txtRecord = result.txt;
    return info;
}

bool MDnsProtocolImpl::IsCacheAvailable(const std::string &key)
{
    constexpr int64_t ms2S = 1000LL;
    NETMGR_EXT_LOG_D("mdns_log IsCacheAvailable, ttl=[%{public}u]", cacheMap_[key].ttl);
    return cacheMap_.find(key) != cacheMap_.end() &&
           (ms2S * cacheMap_[key].ttl) > static_cast<uint32_t>(MilliSecondsSinceEpoch() - cacheMap_[key].refrehTime);
}

bool MDnsProtocolImpl::IsDomainCacheAvailable(const std::string &key)
{
    return IsCacheAvailable(key) && !cacheMap_[key].addr.empty();
}

bool MDnsProtocolImpl::IsInstanceCacheAvailable(const std::string &key)
{
    return IsCacheAvailable(key) && !cacheMap_[key].domain.empty();
}

bool MDnsProtocolImpl::IsBrowserAvailable(const std::string &key)
{
    return browserMap_.find(key) != browserMap_.end() && !browserMap_[key].empty();
}

void MDnsProtocolImpl::AddEvent(const std::string &key, const Task &task)
{
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    taskOnChange_[key].emplace_back(task);
}

void MDnsProtocolImpl::RunTaskQueue(std::list<Task> &queue)
{
    std::list<Task> tmp;
    for (auto &&func : queue) {
        if (!func()) {
            tmp.emplace_back(func);
        }
    }
    tmp.swap(queue);
}

void MDnsProtocolImpl::KillCache(const std::string &key)
{
    NETMGR_EXT_LOG_D("mdns_log KillCache");
    if (IsBrowserAvailable(key) && browserMap_.find(key) != browserMap_.end()) {
        for (auto it = browserMap_[key].begin(); it != browserMap_[key].end();) {
            KillBrowseCache(key, it);
        }
    }
    if (IsCacheAvailable(key)) {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        auto &elem = cacheMap_[key];
        if (elem.state == State::REMOVE) {
            elem.state = State::DEAD;
            cacheMap_.erase(key);
        } else if (elem.state == State::ADD || elem.state == State::REFRESH) {
            elem.state = State::LIVE;
        }
    }
}

void MDnsProtocolImpl::KillBrowseCache(const std::string &key, std::vector<Result>::iterator &it)
{
    NETMGR_EXT_LOG_D("mdns_log KillBrowseCache");
    if (it->state == State::REMOVE) {
        it->state = State::DEAD;
        if (nameCbMap_.find(key) != nameCbMap_.end()) {
            NETMGR_EXT_LOG_D("mdns_log HandleServiceLost");
            nameCbMap_[key]->HandleServiceLost(ConvertResultToInfo(*it), NETMANAGER_EXT_SUCCESS);
        }
        std::string fullName = Decorated(it->serviceName + MDNS_DOMAIN_SPLITER_STR + it->serviceType);
        cacheMap_.erase(fullName);
        it = browserMap_[key].erase(it);
    } else if (it->state == State::ADD || it->state == State::REFRESH) {
        it->state = State::LIVE;
        it++;
    } else {
        it++;
    }
}

int32_t MDnsProtocolImpl::StopCbMap(const std::string &serviceType)
{
    NETMGR_EXT_LOG_D("mdns_log StopCbMap");
    std::lock_guard<std::recursive_mutex> guard(mutex_);
    std::string name = Decorated(serviceType);
    sptr<IDiscoveryCallback> cb = nullptr;
    if (nameCbMap_.find(name) != nameCbMap_.end()) {
        cb = nameCbMap_[name];
        nameCbMap_.erase(name);
    }
    taskOnChange_.erase(name);
    auto it = browserMap_.find(name);
    if (it != browserMap_.end()) {
        if (cb != nullptr) {
            NETMGR_EXT_LOG_I("mdns_log StopCbMap res size:[%{public}zu]", it->second.size());
            for (auto &&res : it->second) {
                NETMGR_EXT_LOG_W("mdns_log HandleServiceLost");
                cb->HandleServiceLost(ConvertResultToInfo(res), NETMANAGER_EXT_SUCCESS);
            }
        }
        browserMap_.erase(name);
    }
    return NETMANAGER_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
