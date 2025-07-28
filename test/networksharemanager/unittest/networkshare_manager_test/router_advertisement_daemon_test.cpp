/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <net/if.h>
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "net_manager_constants.h"
#include "networkshare_tracker.h"
#include "router_advertisement_daemon.h"
#include "sharing_event_callback_stub.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

// size of per option in RA package
constexpr size_t RA_HEADER_SIZE = 16;
constexpr size_t RA_SLLA_SIZE = 8;
constexpr size_t RA_PIO_HEADER_SIZE = 16;
constexpr uint32_t SHIFT_8 = 8;
constexpr uint32_t SHIFT_16 = 16;
constexpr uint32_t SHIFT_24 = 24;
constexpr uint32_t TEST_BUF = 1024;
} // namespace

class RouterAdvertisementDaemonTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RouterAdvertisementDaemonTest::SetUpTestCase() {}
void RouterAdvertisementDaemonTest::TearDownTestCase() {}
void RouterAdvertisementDaemonTest::SetUp() {}
void RouterAdvertisementDaemonTest::TearDown() {}

/**
 * @tc.name: StartRaTest
 * @tc.desc: Test RouterAdvertisementDaemon StartRa/StopRa.Test starting and
 * stopping the transmission of RA messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, StartStopRaTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    auto ret = routerAdvertiseDaemon->StartRa();
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(routerAdvertiseDaemon->IsSocketValid());
    routerAdvertiseDaemon->StopRa();
    while (routerAdvertiseDaemon->raParams_ != nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    EXPECT_FALSE(routerAdvertiseDaemon->IsSocketValid());
    routerAdvertiseDaemon->StopRa();
    EXPECT_EQ(routerAdvertiseDaemon->taskHandle_, nullptr);
}

/**
 * @tc.name: CreateRASocketTest
 * @tc.desc: Test RouterAdvertisementDaemon CreateRASocket.Test the socket for
 * creating RA messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, CreateRASocketTest001, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    auto ret = routerAdvertiseDaemon->CreateRASocket();
    EXPECT_TRUE(ret);
    EXPECT_TRUE(routerAdvertiseDaemon->IsSocketValid());
}

/**
 * @tc.name: CreateRASocketTest
 * @tc.desc: Test RouterAdvertisementDaemon CreateRASocket.Test the socket for
 * creating RA messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, CreateRASocketTest002, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->Init(std::string(IFNAMSIZ, ' '));
    auto ret = routerAdvertiseDaemon->CreateRASocket();
    EXPECT_FALSE(ret);
    EXPECT_FALSE(routerAdvertiseDaemon->IsSocketValid());
}

/**
 * @tc.name: CloseRaSocketTest
 * @tc.desc: Test RouterAdvertisementDaemon CloseRaSocket.Test the socket for
 * closing RA messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, CloseRaSocketTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->CloseRaSocket();
    EXPECT_FALSE(routerAdvertiseDaemon->IsSocketValid());
}

/**
 * @tc.name: MaybeSendRaTest
 * @tc.desc: Test RouterAdvertisementDaemon MaybeSendRa.Test sending RA
 * messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, MaybeSendRaTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    sockaddr_in6 dest;
    dest.sin6_port = htons(0);
    dest.sin6_family = AF_INET6;
    dest.sin6_scope_id = 0;
    inet_pton(AF_INET6, "ff02::1", &dest.sin6_addr);
    routerAdvertiseDaemon->raPacketLength_ = RA_HEADER_SIZE - 1;
    auto ret = routerAdvertiseDaemon->MaybeSendRa(dest);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HupRaThreadTest
 * @tc.desc: Test RouterAdvertisementDaemon HupRaThreadTest.Test setting RA
 * thread stop flag.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, HupRaThreadTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->HupRaThread();
    EXPECT_TRUE(routerAdvertiseDaemon->stopRaThread_);
}

/**
 * @tc.name: GetDeprecatedRaParamsTest
 * @tc.desc: Test RouterAdvertisementDaemon GetDeprecatedRaParamsTest.Test the
 * backup address and DNS information of RA.[RFC4861-4.6.2]
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, GetDeprecatedRaParamsTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    const char *ipv6Prefix1 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    const char *ipv6Prefix2 = "2001:0db8:85a3:0000:0000:8a2e:0370:7335";
    const char *dns1 = "2001:0db8:85a3:0000:0000:8a2e:0370:7336";
    const char *dns2 = "2001:0db8:85a3:0000:0000:8a2e:0370:7337";
    IpPrefix ipPrefix1;
    IpPrefix ipPrefix2;
    EXPECT_TRUE(inet_pton(AF_INET6, ipv6Prefix1, &(ipPrefix1.prefix)) > 0);
    EXPECT_TRUE(inet_pton(AF_INET6, ipv6Prefix2, &(ipPrefix2.prefix)) > 0);
    in6_addr testDns1;
    in6_addr testDns2;
    EXPECT_TRUE(inet_pton(AF_INET6, dns1, &testDns1) > 0);
    EXPECT_TRUE(inet_pton(AF_INET6, dns2, &testDns2) > 0);

    RaParams oldRa;
    RaParams newRa;
    oldRa.prefixes_.emplace_back(ipPrefix1);
    oldRa.dnses_.emplace_back(testDns1);
    newRa.prefixes_.emplace_back(ipPrefix1);
    newRa.dnses_.emplace_back(testDns1);
    newRa.prefixes_.emplace_back(ipPrefix2);
    newRa.dnses_.emplace_back(testDns2);
    RaParams deprecateRa = routerAdvertiseDaemon->GetDeprecatedRaParams(oldRa, newRa);
    EXPECT_EQ(deprecateRa.prefixes_.size(), 1);
    EXPECT_EQ(deprecateRa.dnses_.size(), 1);
    int prefixRet = memcmp(deprecateRa.prefixes_[0].address.s6_addr, ipPrefix1.address.s6_addr, sizeof(in6_addr));
    EXPECT_EQ(prefixRet, 0);
    int dnsRet = memcmp(deprecateRa.dnses_[0].s6_addr, testDns1.s6_addr, sizeof(in6_addr));
    EXPECT_EQ(dnsRet, 0);
}

/**
 * @tc.name: BuildNewRaTest
 * @tc.desc: Test RouterAdvertisementDaemon BuildNewRaTest.Test the message
 * information for the new settings of RA.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, BuildNewRaTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    RaParams newRa;
    newRa.hasDefaultRoute_ = true;
    newRa.hopLimit_ = 255;
    newRa.mtu_ = 1500;
    routerAdvertiseDaemon->BuildNewRa(newRa);
    EXPECT_EQ(routerAdvertiseDaemon->raParams_->hasDefaultRoute_, true);
    EXPECT_EQ(routerAdvertiseDaemon->raParams_->hopLimit_, 255);
    EXPECT_EQ(routerAdvertiseDaemon->raParams_->mtu_, 1500);
}

/**
 * @tc.name: ResetRaRetryIntervalTest
 * @tc.desc: Test RouterAdvertisementDaemon
 * ResetRaRetryIntervalTest.Test the delay time of the next message
 * in RA.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, ResetRaRetryIntervalTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->sendRaTimes_ = 2;
    routerAdvertiseDaemon->ResetRaRetryInterval();
    EXPECT_EQ(routerAdvertiseDaemon->sendRaTimes_, 3);
    routerAdvertiseDaemon->sendRaTimes_ = 12;
    routerAdvertiseDaemon->ResetRaRetryInterval();
    EXPECT_EQ(routerAdvertiseDaemon->sendRaTimes_, 12);
}

/**
 * @tc.name: PutRaHeaderTest00
 * @tc.desc: Test RouterAdvertisementDaemon PutRaHeaderTest00.Test the header
 *          information of RA.[RFC4861-4.2]
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, PutRaHeaderTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    std::vector<uint8_t> buffer;
    uint8_t raBuf[TEST_BUF] = {};
    uint16_t raHeadLen = routerAdvertiseDaemon->PutRaHeader(raBuf);
    EXPECT_EQ(raHeadLen, sizeof(Icmpv6HeadSt));
    EXPECT_EQ(raBuf[0], ICMPV6_ND_ROUTER_ADVERT_TYPE);
    EXPECT_EQ(raBuf[1], 0);
    EXPECT_EQ(raBuf[2], 0);
    EXPECT_EQ(raBuf[3], 0);
    EXPECT_EQ(raBuf[4], 254);
    EXPECT_EQ(raBuf[5], 0x08);
    EXPECT_EQ(raBuf[6], DEFAULT_LIFETIME >> SHIFT_8);
    EXPECT_EQ(raBuf[7], DEFAULT_LIFETIME & 0xFF);
    EXPECT_EQ(raBuf[8], 0);
    EXPECT_EQ(raBuf[9], 0);
    EXPECT_EQ(raBuf[10], 0);
    EXPECT_EQ(raBuf[11], 0);
    EXPECT_EQ(raBuf[12], 0);
    EXPECT_EQ(raBuf[13], 0);
    EXPECT_EQ(raBuf[14], 0);
    EXPECT_EQ(raBuf[15], 0);
}

/**
 * @tc.name: PutRaSllaTest
 * @tc.desc: Test RouterAdvertisementDaemon PutRaSllaTest.Test the link address
 * information of RA messages.[RFC4861-4.6.1]
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, PutRaSllaTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    std::string mac = std::string("02:00:00:00:00:00");
    const char defaultMac[HW_MAC_LENGTH] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t raBuf[TEST_BUF] = {};
    uint16_t raSllLen = routerAdvertiseDaemon->PutRaSlla(raBuf, mac);
    EXPECT_EQ(raSllLen, sizeof(Icmpv6SllOpt));
    EXPECT_EQ(raBuf[0], ND_OPTION_SLLA_TYPE);
    EXPECT_EQ(raBuf[1], 1);
    for (size_t i = 2; i < RA_SLLA_SIZE; i++) {
        EXPECT_EQ(raBuf[i], defaultMac[i - 2]);
    }
}

/**
 * @tc.name: PutRaMtuTest
 * @tc.desc: Test RouterAdvertisementDaemon PutRaMtuTest.Test the Mtu field
 * options for RA messages.[RFC4861-4.6.4]
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, PutRaMtuTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    int32_t mtu = 1500;
    uint8_t raBuf[TEST_BUF] = {};
    uint16_t raMtuLen = routerAdvertiseDaemon->PutRaMtu(raBuf, mtu);
    EXPECT_EQ(raMtuLen, sizeof(Icmpv6MtuOpt));
    EXPECT_EQ(raBuf[0], ND_OPTION_MTU_TYPE);
    EXPECT_EQ(raBuf[1], 1);
    EXPECT_EQ(raBuf[2], 0);
    EXPECT_EQ(raBuf[3], 0);
    EXPECT_EQ(raBuf[4], (mtu >> SHIFT_24) & 0xFF);
    EXPECT_EQ(raBuf[5], (mtu >> SHIFT_16) & 0xFF);
    EXPECT_EQ(raBuf[6], (mtu >> SHIFT_8) & 0xFF);
    EXPECT_EQ(raBuf[7], mtu & 0xFF);
}

/**
 * @tc.name: PutRaPioTest
 * @tc.desc: Test RouterAdvertisementDaemon PutRaPioTest.Test the prefix address
 * lease information of RA messages.[RFC4861-4.6.2]
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, PutRaPioTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    uint8_t raBuf[TEST_BUF] = {};
    IpPrefix ipp;
    ipp.prefix = IN6ADDR_LOOPBACK_INIT;
    ipp.prefixesLength = 64;
    int validTime = 3600;
    int preferredTime = 3600;
    uint16_t raPrefixLen = routerAdvertiseDaemon->PutRaPio(raBuf, ipp);
    EXPECT_EQ(raPrefixLen, sizeof(Icmpv6PrefixInfoOpt));
    EXPECT_EQ(raBuf[0], ND_OPTION_PIO_TYPE);
    EXPECT_EQ(raBuf[1], 4);
    EXPECT_EQ(raBuf[2], ipp.prefixesLength);
    EXPECT_EQ(raBuf[3], 0xC0);
    EXPECT_EQ(raBuf[4], (validTime >> SHIFT_24) & 0xFF);
    EXPECT_EQ(raBuf[5], (validTime >> SHIFT_16) & 0xFF);
    EXPECT_EQ(raBuf[6], (validTime >> SHIFT_8) & 0xFF);
    EXPECT_EQ(raBuf[7], validTime & 0xFF);
    EXPECT_EQ(raBuf[8], (preferredTime >> SHIFT_24) & 0xFF);
    EXPECT_EQ(raBuf[9], (preferredTime >> SHIFT_16) & 0xFF);
    EXPECT_EQ(raBuf[10], (preferredTime >> SHIFT_8) & 0xFF);
    EXPECT_EQ(raBuf[11], preferredTime & 0xFF);
    EXPECT_EQ(raBuf[12], 0);
    EXPECT_EQ(raBuf[13], 0);
    EXPECT_EQ(raBuf[14], 0);
    EXPECT_EQ(raBuf[15], 0);
    size_t offset = RA_PIO_HEADER_SIZE;
    for (uint8_t byte : ipp.prefix.s6_addr) {
        EXPECT_EQ(raBuf[offset++], byte);
    }
}

/**
 * @tc.name: PutRaRdnssTest
 * @tc.desc: Test RouterAdvertisementDaemon PutRaRdnssTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, PutRaRdnssTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    uint8_t raBuf[TEST_BUF] = {};
    size_t raRdnsNum = 1;

    struct IpPrefix ipp;
    ipp.address = IN6ADDR_LOOPBACK_INIT;
    routerAdvertiseDaemon->raParams_->prefixes_.push_back(ipp);

    uint16_t rdnsLen = routerAdvertiseDaemon->PutRaRdnss(raBuf);
    EXPECT_EQ(rdnsLen, sizeof(Icmpv6RdnsOpt) + raRdnsNum * IPV6_ADDR_LEN);

    EXPECT_EQ(raBuf[0], ND_OPTION_RDNSS_TYPE);
    EXPECT_EQ(raBuf[1], (sizeof(Icmpv6RdnsOpt) + raRdnsNum * IPV6_ADDR_LEN) / UNITS_OF_OCTETS);
    EXPECT_EQ(raBuf[2], 0);
    EXPECT_EQ(raBuf[3], 0);

    uint32_t expectedLifetime = htonl(DEFAULT_LIFETIME);
    EXPECT_NE((raBuf[4] << 24) | (raBuf[5] << 16) | (raBuf[6] << 8) | raBuf[7], expectedLifetime);

    routerAdvertiseDaemon->raParams_->prefixes_.clear();
    uint16_t rdnsLenEmpty = routerAdvertiseDaemon->PutRaRdnss(raBuf);
    EXPECT_NE(rdnsLenEmpty, 0);
}

/**
 * @tc.name: AssembleRaLockedTest
 * @tc.desc: Test RouterAdvertisementDaemon AssembleRaLockedTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, AssembleRaLockedTest, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    bool ret = routerAdvertiseDaemon->AssembleRaLocked();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ProcessSendRaPacketTest001
 * @tc.desc: Test RouterAdvertisementDaemon ProcessSendRaPacketTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, ProcessSendRaPacketTest001, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->socket_ = -1;
    routerAdvertiseDaemon->stopRaThread_ = true;
    routerAdvertiseDaemon->ProcessSendRaPacket();
    EXPECT_EQ(routerAdvertiseDaemon->sendRaTimes_, 1);
}

/**
 * @tc.name: ProcessSendRaPacketTest002
 * @tc.desc: Test RouterAdvertisementDaemon ProcessSendRaPacketTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, ProcessSendRaPacketTest002, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->socket_ = 1;
    routerAdvertiseDaemon->stopRaThread_ = false;
    routerAdvertiseDaemon->ProcessSendRaPacket();
    EXPECT_NE(routerAdvertiseDaemon->sendRaTimes_, 1);
}

/**
 * @tc.name: ProcessSendRaPacketTest003
 * @tc.desc: Test RouterAdvertisementDaemon ProcessSendRaPacketTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, ProcessSendRaPacketTest003, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->socket_ = 1;
    routerAdvertiseDaemon->stopRaThread_ = true;
    routerAdvertiseDaemon->ProcessSendRaPacket();
    EXPECT_EQ(routerAdvertiseDaemon->sendRaTimes_, 1);
}

/**
 * @tc.name: ProcessSendRaPacketTest004
 * @tc.desc: Test RouterAdvertisementDaemon ProcessSendRaPacketTest.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, ProcessSendRaPacketTest004, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    routerAdvertiseDaemon->socket_ = -1;
    routerAdvertiseDaemon->stopRaThread_ = false;
    routerAdvertiseDaemon->ProcessSendRaPacket();
    EXPECT_EQ(routerAdvertiseDaemon->sendRaTimes_, 1);
}

/**
 * @tc.name: MaybeSendRaTest002
 * @tc.desc: Test RouterAdvertisementDaemon MaybeSendRa.Test sending RA
 * messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, MaybeSendRaTest002, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    sockaddr_in6 dest;
    dest.sin6_port = htons(0);
    dest.sin6_family = AF_INET6;
    dest.sin6_scope_id = 0;
    inet_pton(AF_INET6, "ff02::1", &dest.sin6_addr);
    routerAdvertiseDaemon->raPacketLength_ = RA_HEADER_SIZE + 1;
    routerAdvertiseDaemon->socket_ = 1;
    auto ret = routerAdvertiseDaemon->MaybeSendRa(dest);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: MaybeSendRaTest003
 * @tc.desc: Test RouterAdvertisementDaemon MaybeSendRa.Test sending RA
 * messages.
 * @tc.type: FUNC
 */
HWTEST_F(RouterAdvertisementDaemonTest, MaybeSendRaTest003, TestSize.Level1)
{
    auto routerAdvertiseDaemon = std::make_shared<RouterAdvertisementDaemon>();
    sockaddr_in6 dest;
    dest.sin6_port = htons(0);
    dest.sin6_family = AF_INET6;
    dest.sin6_scope_id = 0;
    inet_pton(AF_INET6, "ff02::1", &dest.sin6_addr);
    routerAdvertiseDaemon->raPacketLength_ = RA_HEADER_SIZE + 1;
    routerAdvertiseDaemon->socket_ = -1;
    auto ret = routerAdvertiseDaemon->MaybeSendRa(dest);
    EXPECT_FALSE(ret);
}
} // namespace NetManagerStandard
} // namespace OHOS
