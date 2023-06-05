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

#include <gtest/gtest.h>

#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

#include "mdns_client.h"
#include "mdns_common.h"
#include "mdns_event_stub.h"
#include "mdns_packet_parser.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

static constexpr uint8_t SERVICE_QUERY[] = {
    // Query ID
    0x00, 0x00,
    // Flags
    0x00, 0x00,
    // QDCOUNT
    0x00, 0x01,
    // ANCOUNT, NSCOUNT, ARCOUNT
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // _services._dns-sd._udp.local.
    0x09, '_', 's', 'e', 'r', 'v', 'i', 'c', 'e', 's', 0x07, '_', 'd', 'n', 's', '-', 's', 'd', 0x04, '_', 'u', 'd',
    'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00,
    // PTR QTYPE
    0x00, static_cast<uint8_t>(DNSProto::RRTYPE_PTR),
    // QU and class IN
    0x80, static_cast<uint8_t>(DNSProto::RRCLASS_IN)};

static constexpr uint8_t RESPONSE[] =
    "\x00\x00\x84\x00\x00\x00\x00\x05\x00\x00\x00\x00\x04\x5f\x73\x6d"
    "\x62\x04\x5f\x74\x63\x70\x05\x6c\x6f\x63\x61\x6c\x00\x00\x0c\x00"
    "\x01\x00\x00\x11\x94\x00\x06\x03\x4d\x4f\x45\xc0\x0c\xc0\x27\x00"
    "\x10\x80\x01\x00\x00\x11\x94\x00\x01\x00\xc0\x27\x00\x21\x80\x01"
    "\x00\x00\x00\x78\x00\x0c\x00\x00\x00\x00\x01\xbd\x03\x6d\x6f\x65"
    "\xc0\x16\xc0\x4c\x00\x1c\x80\x01\x00\x00\x00\x78\x00\x10\xfe\x80"
    "\x00\x00\x00\x00\x00\x00\x5d\xde\x75\x10\x90\x99\x2a\xf9\xc0\x4c"
    "\x00\x01\x80\x01\x00\x00\x00\x78\x00\x04\x0a\x11\x02\xd4";

static constexpr uint8_t ATTACK_RESPONSE1[] = "\x0a\x0b\x1b\x20\x20\x20\x20\x0b\x0b\x0b\x0b\x0b\x0b";
static constexpr uint8_t ATTACK_RESPONSE2[] = "\x0a\x20\x20\x20\x20\x20\x6f\x20\x20\x01\x20\x20\xfb";
static constexpr uint8_t ATTACK_RESPONSE3[] = "\x00\x01\x00\x01\x00\x00\x00\x01\x00\x01\x00\x01\xC0\x0C";
static constexpr uint8_t ATTACK_RESPONSE4[] =
    "\x01\x00\x2b\x00\x00\x00\x00\x00\x01\x10\x00\x00\x00\x00\x0C\xff\xf6"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1c\x00\xd4\x03\x00\x00\x2b\xfd\x02"
    "\x00\x00\x00\xfd\xfd";

class MDnsProtocolTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MDnsProtocolTest::SetUpTestCase() {}

void MDnsProtocolTest::TearDownTestCase() {}

void MDnsProtocolTest::SetUp() {}

void MDnsProtocolTest::TearDown() {}

/**
 * @tc.name: PacketParserTest001
 * @tc.desc: Test MDnsPayloadParser
 * @tc.type: FUNC
 */
HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest001, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(SERVICE_QUERY), std::end(SERVICE_QUERY));
    auto msg = parser.FromBytes(payload);
    EXPECT_EQ(parser.ToBytes(msg), payload);
}

/**
 * @tc.name: PacketParserTest002
 * @tc.desc: Test MDnsPayloadParser
 * @tc.type: FUNC
 */
HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest002, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(RESPONSE), std::end(RESPONSE) - 1);
    auto msg = parser.FromBytes(payload);
    EXPECT_EQ(parser.ToBytes(msg), payload);
}

HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest003, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(ATTACK_RESPONSE1), std::end(ATTACK_RESPONSE1) - 1);
    auto msg = parser.FromBytes(payload);
    EXPECT_NE(parser.GetError(), ERR_OK);
}

HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest004, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(ATTACK_RESPONSE2), std::end(ATTACK_RESPONSE2) - 1);
    auto msg = parser.FromBytes(payload);
    EXPECT_NE(parser.GetError(), ERR_OK);
}

HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest005, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(ATTACK_RESPONSE3), std::end(ATTACK_RESPONSE3) - 1);
    auto msg = parser.FromBytes(payload);
    EXPECT_NE(parser.GetError(), ERR_OK);
}

HWTEST_F(MDnsProtocolTest, MDnsPayloadParserTest006, TestSize.Level1)
{
    MDnsPayloadParser parser;
    MDnsPayload payload(std::begin(ATTACK_RESPONSE4), std::end(ATTACK_RESPONSE4) - 1);
    auto msg = parser.FromBytes(payload);
    EXPECT_NE(parser.GetError(), ERR_OK);
}
} // namespace NetManagerStandard
} // namespace OHOS