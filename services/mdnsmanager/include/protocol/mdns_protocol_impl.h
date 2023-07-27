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
#include <set>
#include <string>

#include "i_mdns_event.h"
#include "mdns_common.h"
#include "mdns_packet_parser.h"
#include "mdns_socket_listener.h"

namespace OHOS {
namespace NetManagerStandard {

struct MDnsConfig {
    bool ipv6Support = false;
    int configAllIface = true;
    int configLo = true;
    std::string topDomain = MDNS_TOP_DOMAIN_DEFAULT;
    std::string hostname;
};

class MDnsProtocolImpl {
public:
    MDnsProtocolImpl();
    ~MDnsProtocolImpl() = default;

    struct Result;
    using TxtRecord = std::map<std::string, std::vector<uint8_t>>;
    using Task = std::function<bool()>;

    enum class State {
        DEAD,
        ADD,
        LIVE,
        REFRESH,
        REMOVE,
    };

    struct Result {
        std::string serviceName;
        std::string serviceType;
        std::string domain;
        int port = -1;
        bool ipv6 = false;
        std::string addr;
        TxtRecordEncoded txt;
        State state = State::DEAD;
        uint32_t ttl = 0;
        int64_t refrehTime = -1;
        int32_t err = NETMANAGER_EXT_SUCCESS;
    };

    void SetConfig(const MDnsConfig &config);
    bool Browse();
    const MDnsConfig &GetConfig() const;

    int32_t Register(const Result &info);
    int32_t Discovery(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);
    int32_t ResolveInstance(const std::string &instance, const sptr<IResolveCallback> &cb);

    int32_t UnRegister(const std::string &key);
    int32_t StopCbMap(const std::string &key);

    void AddTask(const Task &task, bool atonce = true);
    void AddEvent(const std::string &key, const Task &task);

    void Init();
    int32_t Announce(const Result &info, bool off);
    void ReceivePacket(int sock, const MDnsPayload &payload);
    void RunTaskQueue(std::list<Task> &queue);
    void ProcessQuestion(int sock, const MDnsMessage &msg);
    void ProcessQuestionRecord(const std::any &anyAddr, const DNSProto::RRType &anyAddrType,
                               const DNSProto::Question &qu, int &phase, MDnsMessage &response);
    void ProcessAnswer(int sock, const MDnsMessage &msg);
    void ProcessAnswerRecord(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed);
    void UpdatePtr(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed);
    void UpdateSrv(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed);
    void UpdateTxt(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed);
    void UpdateAddr(bool v6, const DNSProto::ResourceRecord &rr, std::set<std::string> &changed);
    void AppendRecord(std::vector<DNSProto::ResourceRecord> &rrlist, DNSProto::RRType type, const std::string &name,
                      const std::any &rdata);

    bool ResolveInstanceFromCache(const std::string &name, const sptr<IResolveCallback> &cb);
    bool ResolveInstanceFromNet(const std::string &name, const sptr<IResolveCallback> &cb);
    bool ResolveFromCache(const std::string &domain, const sptr<IResolveCallback> &cb);
    bool ResolveFromNet(const std::string &domain, const sptr<IResolveCallback> &cb);
    bool DiscoveryFromCache(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);
    bool DiscoveryFromNet(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);
    bool IsCacheAvailable(const std::string &key);
    bool IsDomainCacheAvailable(const std::string &key);
    bool IsInstanceCacheAvailable(const std::string &key);
    bool IsBrowserAvailable(const std::string &key);
    void KillCache(const std::string &key);
    MDnsServiceInfo ConvertResultToInfo(const Result &result);

    std::string Decorated(const std::string &name) const;
    std::string Dotted(const std::string &name) const;
    std::string UnDotted(const std::string &name) const;
    std::string GetHostDomain();

private:
    void handleOfflineService(const std::string &key, std::vector<Result> &res);
    void KillBrowseCache(const std::string &key, std::vector<Result>::iterator &it);

    int32_t ConnectControl(int32_t sockfd, sockaddr* serverAddr);
    bool IsConnectivity(const std::string &ip, int32_t port);

public:
    std::map<std::string, Result> srvMap_;

private:
    int64_t lastRunTime = {-1};
    MDnsConfig config_;
    MDnsSocketListener listener_;
    std::map<std::string, std::vector<Result>> browserMap_;
    std::map<std::string, Result> cacheMap_;
    std::recursive_mutex mutex_;
    std::list<Task> taskQueue_;
    std::map<std::string, std::list<Task>> taskOnChange_;
    std::map<std::string, sptr<IDiscoveryCallback>> nameCbMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif
