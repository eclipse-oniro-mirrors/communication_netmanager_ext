/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "urspconfig.h"
#include "cellular_data_client.h"
#include "hwnetworkslicemanager.h"
#include "networkslicemanager.h"
#include <parameters.h>
namespace OHOS {
namespace NetManagerStandard {
static std::string DEFAULT_NETSLICE_CONFIG_DIR = "/system/profile/";
static std::string URSP_FILENAME1 = "UrspConfig1.xml";
static std::string URSP_FILENAME2 = "UrspConfig2.xml";
const std::string COMMA_SEPARATOR = ",";
const std::string PORT_RANGE_SEPARATOR = "-";
const std::string TAG_UEPOLICY = "uePolicy";
const std::string TAG_PRECONFIGURSP = "preConfigUrsp";
const std::string TAG_TRAFFICDESCRIPTOR = "trafficDescriptor";
const std::string TAG_ROUTESELECTIONDESCRIPTOR = "routeSelectionDescriptor";
const std::string TAG_ROUTESELECTIONDESCRIPTORCONTENTS = "routeSelectionDescriptorContents";
const std::string ATTR_PLMN = "plmn";
const std::string ATTR_URSPPRECEDENCE = "precedence";
const std::string ATTR_TRAFFICDESCRIPTORTYPE = "trafficDescriptorComponentTypeIdentifier";
const std::string ATTR_TRAFFICDESCRIPTORVALUE = "trafficDescriptorComponentValue";
const std::string ATTR_ROUTEPRECEDENCE = "precedenceValueOfRouteSelectionDescriptor";
const std::string ATTR_RSDTYPE = "routeSelectionDescriptorComponentTypeIdentifier";
const std::string ATTR_RSDVALUE = "routeSelectionDescriptorComponentValue";
const std::string NETWORKSLICE_WHITELIST = "networkslice_whitelist";
constexpr int URSPFILE_ID2 = 2;
constexpr int MODEM_ID = 0;
constexpr int BASE_16 = 16;
constexpr int DECODE_FAIL_UNKNOWN_IDENTIFIER = 1;
constexpr int DECODE_SUCCESS = 0;
constexpr int DECODE_FAIL_OTHER = -1;
constexpr int PREFERRED_ACCESS_TYPE_NON_3GPP = 0x10;
constexpr int FIRSTBYTE = 1;
constexpr int SECONDBYTE = 2;
constexpr int THIRDBYTE = 3;
/**
* traffic descriptor component
*/
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_MATCH_ALL = 0x01;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_OS_ID_OS_APP_ID = 0x08;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_IPV4_ADDR = 0x10;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_IPV6_ADDR = 0x21;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID = 0x30;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT = 0x50;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE = 0x51;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_DNN = 0x88;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY = 0x90;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_FQDN = 0x91;
constexpr int TRAFFIC_DESCRIPTOR_COMPONENT_OS_APP_ID = 0xA0;

/**
* route selection descriptor
*/
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SSC_MODE_TYPE = 1;
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SNSSAI = 2;
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_DNN = 4;
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PDU_SESSION_TYPE = 8;
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PREFERRED_ACCESS_TYPE = 16;
constexpr int ROUTE_SELECTION_DESCRIPTOR_COMPONENT_NON_3GPP_OFFLOAD = 32;
/**
* protocol pdu session type value
*/
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV4 = 1;
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV6 = 2;
constexpr int PROTOCOL_PDU_SESSION_TYPE_IPV4V6 = 3;
constexpr int PROTOCOL_PDU_SESSION_TYPE_UNSTRUCTURED = 4;
constexpr int PROTOCOL_PDU_SESSION_TYPE_ETHERNET = 5;
constexpr int PROTOCOL_PDU_SESSION_TYPE_RESERVED = 7;

/**
* HAL pdu session type value
*/

constexpr int HAL_PDU_SESSION_TYPE_IP = 0;
constexpr int HAL_PDU_SESSION_TYPE_IPV6 = 1;
constexpr int HAL_PDU_SESSION_TYPE_IPV4V6 = 2;
constexpr int HAL_PDU_SESSION_TYPE_NON_IP = 4;
constexpr int HAL_PDU_SESSION_TYPE_UNSTRUCTURED = 5;

constexpr int32_t CONNECTION_CAPABILITY_TYPE_IMS = 1;
constexpr int RSD_MAX_DNN_NUM = 2;
constexpr int RSD_MAX_NSSAI_NUM = 3;
constexpr int RSD_MAX_LIST_NUM = 3;
constexpr int INVALID_VALUE = 0;
constexpr int RSD_MAX_DNN_LEN = 99;
constexpr short IMSA_VERSION = 1;

UrspConfig& UrspConfig::GetInstance()
{
    static UrspConfig instance;
    return instance;
}

UrspConfig::UrspConfig()
{
    NETMGR_EXT_LOG_I("init UrspConfig start");
    ParseConfig();
}

void UrspConfig::ParseConfig()
{
    NETMGR_EXT_LOG_I("UrspConfig::ParseConfig ");
    int urspid = system::GetIntParameter("persist.sys.choose_ursp", 1);
    std::string urspName = URSP_FILENAME1;
    if (urspid == URSPFILE_ID2) {
        urspName = URSP_FILENAME2;
    }
    std::string path = DEFAULT_NETSLICE_CONFIG_DIR  + urspName;
    if (path.empty()) {
        NETMGR_EXT_LOG_E("config file path is empty!");
        return;
    }
    xmlDocPtr doc = xmlReadFile(path.c_str(), NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (doc == NULL) {
        NETMGR_EXT_LOG_E("exception parse config file:[%{public}s]! ", path.c_str());
        return;
    }
    xmlNodePtr curNode = xmlDocGetRootElement(doc);
    if (curNode == NULL) {
        NETMGR_EXT_LOG_E("No root element found in config file!");
        xmlFreeDoc(doc);
        return;
    }
    ParseAllUePolicy(doc);
    xmlFreeDoc(doc);
}

void UrspConfig::ParseAllUePolicy(xmlDocPtr doc)
{
    NETMGR_EXT_LOG_I("UrspConfig::ParseAllUePolicy");
    xmlNodePtr curNode = xmlDocGetRootElement(doc);
    std::string curName = std::string(reinterpret_cast<const char*>(curNode->name));
    NETMGR_EXT_LOG_I("UrspConfig::ParseAllUePolicy::curName:%{public}s ", curName.c_str());
    if (curNode == NULL) {
        NETMGR_EXT_LOG_E("No root element found in config file!");
        return;
    }
    xmlNodePtr child = curNode->children;
    while (child != NULL) {
        if (child->type == XML_TEXT_NODE || child->type == XML_COMMENT_NODE) {
            std::string childName(reinterpret_cast<const char*>(child->name));
            NETMGR_EXT_LOG_I("continue childName:%{public}s ", childName.c_str());
            child = child->next;
            continue;
        }
        std::string childName(reinterpret_cast<const char*>(child->name));
        NETMGR_EXT_LOG_I("UrspConfig::ParseAllUePolicy::childName:%{public}s ", childName.c_str());
        if (child->type == XML_ELEMENT_NODE && childName.compare(TAG_UEPOLICY.c_str()) == 0) {
            ParseUePolicy(child);
        }
        child = child->next;
    }
    SaveTrafficDescriptorWhiteListToDb();
    DumpPreConfigUrspMap();
}

void UrspConfig::ParseUePolicy(xmlNodePtr curNode)
{
    NETMGR_EXT_LOG_I("UrspConfig::ParseUePolicy");
    std::vector<UrspRule> urspRules;
    xmlChar *plmnstr = xmlGetProp(curNode, reinterpret_cast<const xmlChar *>(ATTR_PLMN.c_str()));
    std::string plmnStr;
    if (plmnstr != nullptr) {
        plmnStr = std::string(reinterpret_cast<const char *>(plmnstr));
        NETMGR_EXT_LOG_I("UrspConfig::ParseUePolicy::plmnStr = %{public}s", plmnStr.c_str());
        xmlFree(plmnstr);
    }
    xmlNodePtr child = curNode->children;
    while (child != NULL) {
        if (child->type == XML_TEXT_NODE || child->type == XML_COMMENT_NODE) {
            child = child->next;
            continue;
        }
        std::string childName(reinterpret_cast<const char*>(child->name));
        if (child->type == XML_ELEMENT_NODE && childName.compare(TAG_PRECONFIGURSP.c_str()) == 0) {
            NETMGR_EXT_LOG_I("UrspConfig::ParseUePolicy::childName:%{public}s ", childName.c_str());
            UrspRule urspRule = ParsePreConfigUrsp(child);
            urspRules.push_back(urspRule);
        }
        child = child->next;
    }
    SndPreImsRsdList();
    UrspRuleSort(urspRules);
    mPreConfigUrspMap[plmnStr] = urspRules;
}

UrspRule UrspConfig::ParsePreConfigUrsp(xmlNodePtr node)
{
    NETMGR_EXT_LOG_I("UrspConfig::ParsePreConfigUrsp ");
    UrspRule urspRule;
    xmlChar *urspprecedence = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_URSPPRECEDENCE.c_str()));
    if (urspprecedence != nullptr) {
        urspRule.urspPrecedence = atoi(reinterpret_cast<char *>(urspprecedence));
        NETMGR_EXT_LOG_I("UrspConfig::ParsePreConfigUrsp::urspPrecedence = %{public}d", urspRule.urspPrecedence);
        xmlFree(urspprecedence);
    }
    xmlNodePtr child = node->children;
    while (child != NULL) {
        if (child->type == XML_TEXT_NODE || child->type == XML_COMMENT_NODE) {
            child = child->next;
            continue;
        }
        std::string childName(reinterpret_cast<const char*>(child->name));
        NETMGR_EXT_LOG_I("UrspConfig::ParsePreConfigUrsp::childName = %{public}s", childName.c_str());
        if (child->type == XML_ELEMENT_NODE && childName.compare(TAG_TRAFFICDESCRIPTOR.c_str()) == 0) {
            ParseTrafficDescriptor(child, urspRule.trafficDescriptor);
        } else if (child->type == XML_ELEMENT_NODE && childName.compare(TAG_ROUTESELECTIONDESCRIPTOR.c_str()) == 0) {
            ParseRouteSelectionDescriptor(child, urspRule.routeSelectionDescriptors);
        }
        child = child->next;
    }
    if (mIsTrafficDescriptorIncludeIms) {
        NETMGR_EXT_LOG_I("UrspConfig::ParsePreConfigUrsp IncludeIms");
        mImsRsdsMap[urspRule.urspPrecedence] = urspRule.routeSelectionDescriptors;
        mIsTrafficDescriptorIncludeIms = false;
    }
    return urspRule;
}

void UrspConfig::ParseTrafficDescriptor(xmlNodePtr node, TrafficDescriptor &trafficDescriptor)
{
    xmlChar *attrvalue = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORTYPE.c_str()));
    std::string attrValue;
    if (attrvalue != nullptr) {
        attrValue = std::string(reinterpret_cast<char *>(attrvalue));
        xmlFree(attrvalue);
    }
    int trafficDescriptorType = std::stoi(attrValue);
    xmlChar *AttrtrafficDescriptorvalue = nullptr;
    switch (trafficDescriptorType) {
        case TRAFFIC_DESCRIPTOR_COMPONENT_MATCH_ALL:
            trafficDescriptor.isMatchAll = true;
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_OS_ID_OS_APP_ID:
            ParseOsAppId(node, trafficDescriptor);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_IPV4_ADDR:
            ParseIpv4Addr(node, trafficDescriptor);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_IPV6_ADDR:
            ParseIpv6Addr(node, trafficDescriptor);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID:
        case TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT:
        case TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE:
        case TRAFFIC_DESCRIPTOR_COMPONENT_DNN:
        case TRAFFIC_DESCRIPTOR_COMPONENT_FQDN:
        case TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY:
            ParseTrafficDescriptorEx(node, trafficDescriptor, attrValue);
            break;
        default:
            break;
    }
    if (AttrtrafficDescriptorvalue != nullptr) {
        xmlFree(AttrtrafficDescriptorvalue);
    }
}

void UrspConfig::ParseTrafficDescriptorEx(xmlNodePtr node, TrafficDescriptor &trafficDescriptor, std::string attrValue)
{
    std::string trafficDescriptorValue;
    int trafficDescriptorType = std::stoi(attrValue);
    xmlChar *AttrtrafficDescriptorvalue = nullptr;
    switch (trafficDescriptorType) {
        case TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID:
            AttrtrafficDescriptorvalue = xmlGetProp(node,
                reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
            trafficDescriptorValue = (std::string)reinterpret_cast<char *>(AttrtrafficDescriptorvalue);
            trafficDescriptor.protocolIds.push_back(std::stoi(trafficDescriptorValue));
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT:
            AttrtrafficDescriptorvalue = xmlGetProp(node,
                reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
            trafficDescriptorValue = (std::string)reinterpret_cast<char *>(AttrtrafficDescriptorvalue);
            trafficDescriptor.singleRemotePorts.push_back(std::stoi(trafficDescriptorValue));
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE:
            ParseRemotePortRange(node, trafficDescriptor);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_DNN:
            AttrtrafficDescriptorvalue = xmlGetProp(node,
                reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
            trafficDescriptorValue = (std::string)reinterpret_cast<char *>(AttrtrafficDescriptorvalue);
            if (trafficDescriptorValue == "ims" || trafficDescriptorValue == "IMS") {
                mIsTrafficDescriptorIncludeIms = true;
            }
            trafficDescriptor.dnns.push_back(trafficDescriptorValue);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_FQDN:
            AttrtrafficDescriptorvalue = xmlGetProp(node,
                reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
            trafficDescriptorValue = (std::string)reinterpret_cast<char *>(AttrtrafficDescriptorvalue);
            trafficDescriptor.fqdns.push_back(trafficDescriptorValue);
            break;
        case TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY:
            ParseConnectionCapabilities(node, trafficDescriptor);
            break;
    }
}

void UrspConfig::ParseRouteSelectionDescriptor(xmlNodePtr node,
    std::vector<RouteSelectionDescriptor> &routeSelectionDescriptors)
{
    NETMGR_EXT_LOG_I("UrspConfig::ParseRouteSelectionDescriptor ");
    xmlChar *attrvalue = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_ROUTEPRECEDENCE.c_str()));
    std::string attrValue;
    if (attrvalue != nullptr) {
        attrValue = std::string(reinterpret_cast<char *>(attrvalue));
        xmlFree(attrvalue);
    }
    NETMGR_EXT_LOG_I("routeSelectionDescriptorPrecedence::%{public}s", attrValue.c_str());
    int Precedence = std::stoi(attrValue);
    RouteSelectionDescriptor routeSelectionDescriptor;
    routeSelectionDescriptor.routePrecedence = Precedence;
    xmlNodePtr child = node->children;
    while (child != NULL) {
        if (child->type == XML_TEXT_NODE || child->type == XML_COMMENT_NODE) {
            child = child->next;
            continue;
        }
        std::string childName(reinterpret_cast<const char*>(child->name));
        if (child->type == XML_ELEMENT_NODE &&
            childName.compare(TAG_ROUTESELECTIONDESCRIPTORCONTENTS.c_str()) == 0) {
            ParseRouteRule(child, routeSelectionDescriptor);
        }
        child = child->next;
    }
    routeSelectionDescriptors.push_back(routeSelectionDescriptor);
}

void UrspConfig::ParseRouteRule(xmlNodePtr node, RouteSelectionDescriptor &routeSelectionDescriptor)
{
    std::string routeRuleValue;
    Snssai snssai;
    xmlChar *attrvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_RSDTYPE.c_str()));
    std::string attrValue;
    if (attrvalue != nullptr) {
        attrValue = std::string(reinterpret_cast<char *>(attrvalue));
        xmlFree(attrvalue);
    }
    int routeRuleType = std::stoi(attrValue);
    NETMGR_EXT_LOG_I("ParseRouteRule routeRuleType = %{public}d", routeRuleType);
    xmlChar *AttrrouteRulevalue = nullptr;
    switch (routeRuleType) {
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SSC_MODE_TYPE:
            AttrrouteRulevalue = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_RSDVALUE.c_str()));
            routeRuleValue = (std::string)reinterpret_cast<char *>(AttrrouteRulevalue);
            routeSelectionDescriptor.sscMode = static_cast<int>(std::stoi(routeRuleValue));
            break;
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SNSSAI:
            AttrrouteRulevalue = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_RSDVALUE.c_str()));
            routeRuleValue = (std::string)reinterpret_cast<char *>(AttrrouteRulevalue);
            snssai.setSnssai(routeRuleValue);
            if (sAllowedNssaiConfig_->ParseSnssai(snssai) == false) {
                NETMGR_EXT_LOG_E("parseSNssai(sNssai) is false");
                break;
            }
            routeSelectionDescriptor.snssais.push_back(snssai);
            break;
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_DNN:
            AttrrouteRulevalue = xmlGetProp(node,
                reinterpret_cast<const xmlChar *>(ATTR_RSDVALUE.c_str()));
            routeRuleValue = (std::string)reinterpret_cast<char *>(AttrrouteRulevalue);
            routeSelectionDescriptor.dnns.push_back(routeRuleValue);
            break;
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PDU_SESSION_TYPE:
            AttrrouteRulevalue = xmlGetProp(node, reinterpret_cast<const xmlChar *>(ATTR_RSDVALUE.c_str()));
            routeRuleValue = (std::string)reinterpret_cast<char *>(AttrrouteRulevalue);
            routeSelectionDescriptor.pduSessionType = static_cast<int>(std::stoi(routeRuleValue));
            break;
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PREFERRED_ACCESS_TYPE:
        case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_NON_3GPP_OFFLOAD:
            break;
        default:
            break;
    }
    if (AttrrouteRulevalue != nullptr) {
        xmlFree(AttrrouteRulevalue);
    }
}

bool UrspConfig::ParseOsAppId(xmlNodePtr node, TrafficDescriptor& trafficDescriptor)
{
    xmlChar *trafficDescriptorvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
    std::string trafficDescriptorValue = (std::string)reinterpret_cast<char *>(trafficDescriptorvalue);
    xmlFree(trafficDescriptorvalue);
    std::vector<std::string> values;
    values = Split(trafficDescriptorValue, COMMA_SEPARATOR);
    OsAppId osAppId;
    osAppId.setOsId(values[0]);
    osAppId.setAppId(values[1]);
    trafficDescriptor.osAppIds.push_back(osAppId);
    return true;
}

bool UrspConfig::ParseIpv4Addr(xmlNodePtr node, TrafficDescriptor& trafficDescriptor)
{
    xmlChar *trafficDescriptorvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
    std::string trafficDescriptorValue = (std::string)reinterpret_cast<char *>(trafficDescriptorvalue);
    xmlFree(trafficDescriptorvalue);
    std::vector<std::string> values;
    values = Split(trafficDescriptorValue, COMMA_SEPARATOR);
    uint32_t ipv4Address = 0;
    uint32_t ipv4Mask = 0;
    Ipv4Addr ipv4Addr;
    ipv4Address = inet_addr(values[0].c_str());
    if (ipv4Address == INADDR_NONE) {
        return false;
    }
    ipv4Mask = inet_addr(values[1].c_str());
    if (ipv4Mask == INADDR_NONE) {
        return false;
    }
    ipv4Addr.setIpv4Addr(ipv4Address);
    ipv4Addr.setIpv4Mask(ipv4Mask);
    trafficDescriptor.ipv4Addrs.push_back(ipv4Addr);
    return true;
}

bool UrspConfig::ParseIpv6Addr(xmlNodePtr node, TrafficDescriptor& trafficDescriptor)
{
    xmlChar *trafficDescriptorvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
    std::string trafficDescriptorValue = (std::string)reinterpret_cast<char *>(trafficDescriptorvalue);
    xmlFree(trafficDescriptorvalue);
    std::vector<std::string> values;
    values = Split(trafficDescriptorValue, COMMA_SEPARATOR);
    struct in6_addr ipv6;
    int prefixLen = 0;
    Ipv6Addr ipv6Addr;
    if (inet_pton(AF_INET6, values[0].c_str(), &ipv6) == 0) {
        return false;
    }

    prefixLen = std::stoi(values[1]);
    ipv6Addr.setIpv6Addr(*reinterpret_cast<std::array<uint8_t, BASE_16>*>(ipv6.s6_addr));
    ipv6Addr.setIpv6PrefixLen(prefixLen);
    trafficDescriptor.ipv6Addrs.push_back(ipv6Addr);
    return true;
}

bool UrspConfig::ParseRemotePortRange(xmlNodePtr node, TrafficDescriptor& trafficDescriptor)
{
    xmlChar *trafficDescriptorvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
    std::string trafficDescriptorValue = (std::string)reinterpret_cast<char *>(trafficDescriptorvalue);
    xmlFree(trafficDescriptorvalue);
    std::vector<std::string> values;
    values = Split(trafficDescriptorValue, COMMA_SEPARATOR);
    RemotePortRange portRang;
    portRang.setPortRangeLowLimit(std::stoi(values[0]));
    portRang.setPortRangeHighLimit(std::stoi(values[1]));

    trafficDescriptor.remotePortRanges.push_back(portRang);
    return true;
}

bool UrspConfig::ParseConnectionCapabilities(xmlNodePtr node, TrafficDescriptor& trafficDescriptor)
{
    xmlChar *trafficDescriptorvalue = xmlGetProp(node,
        reinterpret_cast<const xmlChar *>(ATTR_TRAFFICDESCRIPTORVALUE.c_str()));
    std::string trafficDescriptorValue = (std::string)reinterpret_cast<char *>(trafficDescriptorvalue);
    xmlFree(trafficDescriptorvalue);
    int connectionCapabilityType = std::stoi(trafficDescriptorValue);
    if ((connectionCapabilityType & CONNECTION_CAPABILITY_TYPE_IMS) != 0) {
        mIsTrafficDescriptorIncludeIms = true;
    }
    trafficDescriptor.connectionCapabilities.push_back(connectionCapabilityType);
    return true;
}


void UrspConfig::setUrspRules(const std::string& plmn, std::vector<UrspRule>& urspRules)
{
    UrspRuleSort(urspRules);
    mUePolicyMap[plmn] = urspRules;
}

void UrspConfig::ClearUrspRules()
{
    mUePolicyMap.clear();
}

void UrspConfig::SetUrspVersion(int urspVersion)
{
    mUrspVersion = urspVersion;
}

void UrspConfig::UrspRuleSort(std::vector<UrspRule>& urspRules)
{
    for (auto& urspRule : urspRules) {
        std::sort(urspRule.routeSelectionDescriptors.begin(),
            urspRule.routeSelectionDescriptors.end(), [](const auto& r1, const auto& r2) {
        return r1.routePrecedence < r2.routePrecedence;
    });
    }
    // Order ursp rules based on urspPrecedence
    std::sort(urspRules.begin(), urspRules.end(), [](const auto& r1, const auto& r2) {
        return r1.urspPrecedence < r2.urspPrecedence;
    });
}

void UrspConfig::SaveTrafficDescriptorWhiteListToDb()
{
    NETMGR_EXT_LOG_I("SaveTrafficDescriptorWhiteList ");
    std::shared_ptr<TrafficDescriptorWhiteList> whiteList = std::make_shared<TrafficDescriptorWhiteList>();
    std::vector<UrspRule> urspRules;
    std::unordered_map<std::string, std::vector<UrspRule>> uePolicyMap =
        mUePolicyMap.empty() ? mPreConfigUrspMap : mUePolicyMap;
    for (const auto& entry : uePolicyMap) {
        urspRules = entry.second;
        FillTrafficDescriptorWhiteList(whiteList, urspRules);
    }

    if (whiteList != nullptr) {
        DelayedSingleton<HwNetworkSliceManager>::GetInstance()->GetTrafficDescriptorWhiteList(*whiteList);
    }
}

void UrspConfig::FillTrafficDescriptorWhiteList(std::shared_ptr<TrafficDescriptorWhiteList>& whiteList,
    const std::vector<UrspRule>& urspRules)
{
    NETMGR_EXT_LOG_I("FillTrafficDescriptorWhiteList ");
    for (const auto& urspRule : urspRules) {
        const TrafficDescriptor& trafficDescriptor = urspRule.trafficDescriptor;

        for (auto& osAppId : trafficDescriptor.osAppIds) {
            std::string osAppIdStr = osAppId.getOsId() + "#" + osAppId.getAppId();
            if (!whiteList->osAppIds.empty() && whiteList->osAppIds.find(osAppIdStr) != std::string::npos) {
                continue;
            }
            if (!whiteList->osAppIds.empty()) {
                whiteList->osAppIds += COMMA_SEPARATOR;
            }
            whiteList->osAppIds += osAppIdStr;
        }

        for (const auto& dnn : trafficDescriptor.dnns) {
            if (!whiteList->dnns.empty() && whiteList->dnns.find(dnn) != std::string::npos) {
                continue;
            }
            if (!whiteList->dnns.empty()) {
                whiteList->dnns += COMMA_SEPARATOR;
            }
            whiteList->dnns += dnn;
        }

        for (const auto& fqdn : trafficDescriptor.fqdns) {
            if (!whiteList->fqdns.empty() && whiteList->fqdns.find(fqdn) != std::string::npos) {
                continue;
            }
            if (!whiteList->fqdns.empty()) {
                whiteList->fqdns += COMMA_SEPARATOR;
            }
            whiteList->fqdns += fqdn;
        }

        for (const auto& connectionCapability : trafficDescriptor.connectionCapabilities) {
            if (!whiteList->cct.empty() && whiteList->cct.find(connectionCapability) != std::string::npos) {
                continue;
            }
            if (!whiteList->cct.empty()) {
                whiteList->cct += COMMA_SEPARATOR;
            }
            whiteList->cct += std::to_string(connectionCapability);
        }
    }
}

bool UrspConfig::DecodeUrspRules(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, std::vector<UrspRule>& urspRules)
{
    int startIndex_tmp = startIndex;
    SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1520);
    if (DecodeUrspByVersion(inputLen, startIndex, buffer, urspRules, NetworkSliceCommConfig::URSP_VERSION_1520)) {
        NETMGR_EXT_LOG_I("decodeUrspRules, success urspVersion is 1520");
        sSuccUrspVersion = NetworkSliceCommConfig::URSP_VERSION_1520;
        return true;
    }
    SetUrspVersion(NetworkSliceCommConfig::URSP_VERSION_1510);
    if (DecodeUrspByVersion(inputLen, startIndex_tmp, buffer, urspRules, NetworkSliceCommConfig::URSP_VERSION_1510)) {
        NETMGR_EXT_LOG_I("decodeUrspRules, success urspVersion is 1510");
        sSuccUrspVersion = NetworkSliceCommConfig::URSP_VERSION_1510;
        return true;
    }
    NETMGR_EXT_LOG_E("decodeUrspRules(inputLen, buffer, urspRules) is false");
    return false;
}

short UrspConfig::GetSuccUrspVersion()
{
    return sSuccUrspVersion;
}

bool UrspConfig::DecodeUrspByVersion(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, std::vector<UrspRule>& urspRules, short version)
{
    int subLen;
    int len = inputLen;
    NETMGR_EXT_LOG_I("version[%{public}d]:startIndex:%{public}d", version, startIndex);
    while (len > 0) {
        subLen = GetSubLenByversion(startIndex, buffer, version);
        if (subLen == DECODE_FAIL_OTHER) {
            return false;
        }
        len = len - GetLenBytesByversion(version) - subLen;
        if (((int(buffer.size()) - startIndex) < subLen) || (len < 0)) {
            NETMGR_EXT_LOG_E("(buffer.remaining()[%{public}d] < subLen[%{public}d]) || (len[%{public}d] < 0)",
                (int(buffer.size()) - startIndex), subLen, len);
            return false;
        }

        if (!DecodeUrspRule(subLen, startIndex, buffer, urspRules, version)) {
            urspRules.clear();
            ResetImsRsdsMap();
            NETMGR_EXT_LOG_E("decodeUrspRule(subLen, buffer, urspRules, verision) is false");
            return false;
        }
    }
    return true;
}

void UrspConfig::ResetImsRsdsMap()
{
    mIsTrafficDescriptorIncludeIms = false;
    mImsRsdsMap.clear();
}

int UrspConfig::GetSubLenByversion(int& startIndex, std::vector<uint8_t> buffer, short version)
{
    int subLen;
    if (version == NetworkSliceCommConfig::URSP_VERSION_1510) {
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
            NETMGR_EXT_LOG_E("buffer.remaining() < NetworkSliceCommConfig::LEN_BYTE");
            return DECODE_FAIL_OTHER;
        }
        subLen = ConvertUnsignedShort2Int(buffer[startIndex++]);
    } else if (version == NetworkSliceCommConfig::URSP_VERSION_1520) {
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
            NETMGR_EXT_LOG_E("buffer.remaining() < NetworkSliceCommConfig::LEN_BYTE");
            return DECODE_FAIL_OTHER;
        }
        short subLen_short = GetShort(startIndex, buffer);
        subLen = ConvertUnsignedShort2Int(subLen_short);
    } else {
        NETMGR_EXT_LOG_E("Ursp version invalid");
        return DECODE_FAIL_OTHER;
    }
    return subLen;
}

int UrspConfig::GetLenBytesByversion(short version)
{
    if (version == NetworkSliceCommConfig::URSP_VERSION_1510) {
        return NetworkSliceCommConfig::LEN_BYTE;
    } else {
        return NetworkSliceCommConfig::LEN_SHORT;
    }
}

bool UrspConfig::DecodeUrspRule(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, std::vector<UrspRule>& urspRules, short version)
{
    NETMGR_EXT_LOG_I("DecodeUrspRule");
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_THREE_BYTE) {
        /* LEN_THREE_BYTE = Precedence + Length of traffic descriptor */
        NETMGR_EXT_LOG_E("buffer.remaining()[%{public}d] < NetworkSliceCommConfig::LEN_THREE_BYTE",
            (int(buffer.size()) - startIndex));
        return false;
    }
    UrspRule urspRule;
    /* Precedence value of URSP rule */
    urspRule.urspPrecedence = ConvertUnsignedShort2Int(buffer[startIndex++]);
    return DecodeUrspRuleExtra(inputLen, startIndex, buffer, urspRule, urspRules, version);
}

bool UrspConfig::DecodeUrspRuleExtra(int inputLen, int& startIndex, std::vector<uint8_t> buffer,
    UrspRule& urspRule, std::vector<UrspRule>& urspRules, short version)
{
    int trafficDescriptorLen = ConvertUnsignedShort2Int(GetShort(startIndex, buffer));
    int len = inputLen;
    if (((int(buffer.size()) - startIndex) < trafficDescriptorLen)
        || (len < (NetworkSliceCommConfig::LEN_THREE_BYTE + trafficDescriptorLen))) {
        NETMGR_EXT_LOG_E("buffer.remaining() < trafficDescriptorLen");
        return false;
    }
    len = len - NetworkSliceCommConfig::LEN_THREE_BYTE - trafficDescriptorLen;
    int ret = DecodeTrafficDescriptor(trafficDescriptorLen, startIndex, buffer, urspRule.trafficDescriptor);
    if (ret == DECODE_FAIL_OTHER) {
        return false;
    }
    if (ret == DECODE_FAIL_UNKNOWN_IDENTIFIER) {
        if ((int(buffer.size()) - startIndex) < len) {
            NETMGR_EXT_LOG_E("buffer.remaining() < len");
            return false;
        }
        std::vector<uint8_t> dsts =std::vector<uint8_t>(len);
        std::copy(buffer.begin() + startIndex, buffer.begin() + startIndex + len, dsts.begin());
        return true;
    }
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("buffer.remaining() < NetworkSliceCommConfig::LEN_SHORT");
        return false;
    }
    int routeRuleListLen = ConvertUnsignedShort2Int(GetShort(startIndex, buffer));
    if (((int(buffer.size()) - startIndex) < routeRuleListLen)
        || (len != NetworkSliceCommConfig::LEN_SHORT + routeRuleListLen)) {
        NETMGR_EXT_LOG_E("buffer.remaining[%{public}d] < routeRuleListLen[%{public}d]",
            (int(buffer.size()) - startIndex), routeRuleListLen);
        return false;
    }
    ret = DecodeRouteRuleList(routeRuleListLen, startIndex, buffer, urspRule, version);
    if (ret == DECODE_FAIL_OTHER) {
        return false;
    }
    if (ret == DECODE_FAIL_UNKNOWN_IDENTIFIER) {
        return true;
    }
    if (mIsTrafficDescriptorIncludeIms) {
        NETMGR_EXT_LOG_I("UrspConfig::DecodeUrspRuleExtra IncludeIms");
        mImsRsdsMap[urspRule.urspPrecedence] = urspRule.routeSelectionDescriptors;
        mIsTrafficDescriptorIncludeIms = false;
    }
    urspRules.push_back(urspRule);
    return true;
}

int UrspConfig::DecodeTrafficDescriptor(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    NETMGR_EXT_LOG_I("DecodeTrafficDescriptor");
    int len = inputLen;
    int initBufferRemaining = (int(buffer.size()) - startIndex);
    while (initBufferRemaining - (int(buffer.size()) - startIndex) < len) {
        /* Traffic descriptor component type identifier */
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
            NETMGR_EXT_LOG_E("buffer.remaining() < Traffic descriptor component type identifier Len");
            return DECODE_FAIL_OTHER;
        }
        int type = static_cast<int>(buffer[startIndex++]);
        int ret = DECODE_SUCCESS;
        switch (type) {
            case TRAFFIC_DESCRIPTOR_COMPONENT_MATCH_ALL:
                /* Match-all type */
                trafficDescriptor.isMatchAll = true;
                break;
            case TRAFFIC_DESCRIPTOR_COMPONENT_OS_ID_OS_APP_ID:
                /* OS Id + OS App Id type */
                ret = DecodeOsIdOsAppId(startIndex, buffer, trafficDescriptor, false);
                break;
            case TRAFFIC_DESCRIPTOR_COMPONENT_IPV4_ADDR:
                /* IPv4 remote address type */
                ret = DecodeIpv4Addr(startIndex, buffer, trafficDescriptor);
                break;
            case TRAFFIC_DESCRIPTOR_COMPONENT_IPV6_ADDR:
                /* IPv6 remote address/prefix length type */
                ret = DecodeIpv6Addr(startIndex, buffer, trafficDescriptor);
                break;
            case TRAFFIC_DESCRIPTOR_COMPONENT_PROTOCOL_ID:
                /* Protocol identifier/next header type */
                ret = DecodeProtocolId(startIndex, buffer, trafficDescriptor);
                break;
            default:
                ret = DecodeTrafficDescriptorExtra(inputLen, startIndex, type,
                    buffer, trafficDescriptor, initBufferRemaining);
                break;
        }
        if (ret != DECODE_SUCCESS) {
            return ret;
        }
    }
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeRouteRuleList(int inputLen, int& startIndex,
    std::vector<uint8_t> buffer, UrspRule& urspRule, short version)
{
    NETMGR_EXT_LOG_I("DecodeRouteRuleList");
    int subLen;
    int len = inputLen;
    while (len > 0) {
        subLen = GetSubLenByversion(startIndex, buffer, version);
        if (subLen == DECODE_FAIL_OTHER) {
            return DECODE_FAIL_OTHER;
        }
        len = len - GetLenBytesByversion(version) - subLen;
        if ((int(buffer.size()) - startIndex) < subLen || (len < 0)) {
            NETMGR_EXT_LOG_E("(buffer.remaining([%{public}d]) < subLen[%{public}d]) || (len[%{public}d] < 0)",
                (int(buffer.size()) - startIndex), subLen, len);
            return DECODE_FAIL_OTHER;
        }
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_THREE_BYTE) {
            /* LEN_THREE_BYTE = Precedence + Length of route selection descriptor contents */
            NETMGR_EXT_LOG_E("buffer.remaining() < NetworkSliceCommConfig::LEN_THREE_BYTE");
            return DECODE_FAIL_OTHER;
        }
        RouteSelectionDescriptor routeRule;
        /* Precedence value of route selection descriptor */
        routeRule.routePrecedence = buffer[startIndex++];
        /* Length of route selection descriptor contents */
        int routeRuleLen = ConvertUnsignedShort2Int(GetShort(startIndex, buffer));
        if (((int(buffer.size()) - startIndex) < routeRuleLen)
            || (subLen != NetworkSliceCommConfig::LEN_THREE_BYTE + routeRuleLen)) {
            NETMGR_EXT_LOG_E("buffer.remaining([%{public}d]) < routeRuleLen[%{public}d], subLen[%{public}d])",
                (int(buffer.size()) - startIndex), routeRuleLen, subLen);
            return DECODE_FAIL_OTHER;
        }
        int ret = DecodeRouteRule(startIndex, routeRuleLen, buffer, routeRule);
        if (ret == DECODE_FAIL_OTHER) {
            return DECODE_FAIL_OTHER;
        }
        if (ret == DECODE_FAIL_UNKNOWN_IDENTIFIER) {
            continue;
        }
        urspRule.routeSelectionDescriptors.push_back(routeRule);
    }
    if (urspRule.routeSelectionDescriptors.empty()) {
        return DECODE_FAIL_UNKNOWN_IDENTIFIER;
    }
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeTrafficDescriptorExtra(int inputLen, int& startIndex, int type,
    std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor, int initBufferRemaining)
{
    int ret = DECODE_SUCCESS;
    int len = inputLen;
    switch (type) {
        case TRAFFIC_DESCRIPTOR_COMPONENT_SINGLE_REMOTE_PORT:
            /* Single remote port type */
            ret = DecodeSingleRemotePort(startIndex, buffer, trafficDescriptor);
            return ret;
        case TRAFFIC_DESCRIPTOR_COMPONENT_REMOTE_PORT_RANGE:
            /* Remote port range type */
            ret = DecodeRemotePortRange(startIndex, buffer, trafficDescriptor);
            return ret;
        case TRAFFIC_DESCRIPTOR_COMPONENT_DNN:
            /* DNN type */
            ret = DecodeTrafficDescriptorDnn(startIndex, buffer, trafficDescriptor);
            return ret;
        case TRAFFIC_DESCRIPTOR_COMPONENT_FQDN:
            /* Destination FQDN */
            ret = DecodeFqdn(startIndex, buffer, trafficDescriptor);
            if (ret == DECODE_FAIL_UNKNOWN_IDENTIFIER) {
                int sublen = len - (initBufferRemaining - (int(buffer.size()) - startIndex));
                std::vector<uint8_t> dsts = std::vector<uint8_t>(sublen);
                for (int i = 0; i < sublen; ++i) {
                    dsts[i] = buffer[startIndex];
                    startIndex++;
                }
            }
            return ret;
        case TRAFFIC_DESCRIPTOR_COMPONENT_CONNECTION_CAPABILITY:
            /* Connection capability type */
            ret = DecodeConnectionCapabilities(startIndex, buffer, trafficDescriptor);
            return ret;
        case TRAFFIC_DESCRIPTOR_COMPONENT_OS_APP_ID:
            /* OS App Id type */
            ret = DecodeOsIdOsAppId(startIndex, buffer, trafficDescriptor, true);
            return ret;
        default:
            NETMGR_EXT_LOG_E("decodeTrafficDescriptor failed, invalid Traffic descriptor component type identifier");
            int sublen = len - (initBufferRemaining - (int(buffer.size()) - startIndex));
            std::vector<uint8_t> dsts = std::vector<uint8_t>(sublen);
            for (int i = 0; i < sublen; ++i) {
                dsts[i] = buffer[startIndex];
                startIndex++;
            }
            return ret;
    }
}

int UrspConfig::DecodeRouteRule(int& startIndex, int inputLen,
    std::vector<uint8_t> buffer, RouteSelectionDescriptor& routeRule)
{
    int len = inputLen;
    int initBufferRemaining = (int(buffer.size()) - startIndex);
    while (initBufferRemaining - (int(buffer.size()) - startIndex) < len) {
        if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) { /* Rsd component type identifier */
            NETMGR_EXT_LOG_E("buffer.remaining() < Route selection descriptor component type identifier len");
            return DECODE_FAIL_OTHER;
        }
        int type = static_cast<int>(buffer[startIndex++]);
        int ret = DECODE_SUCCESS;
        switch (type) {
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SSC_MODE_TYPE:
                ret = DecodeSscMode(startIndex, buffer, routeRule);
                break;
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_SNSSAI:
                ret = DecodeSnssai(startIndex, buffer, routeRule);
                break;
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_DNN:
                ret = DecodeDnn(startIndex, buffer, routeRule);
                break;
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PDU_SESSION_TYPE:
                ret = DecodePduSessionType(startIndex, buffer, routeRule);
                break;
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_PREFERRED_ACCESS_TYPE:
                ret = DecodePreferredAccessType(startIndex, buffer, routeRule);
                if (ret == DECODE_FAIL_UNKNOWN_IDENTIFIER) {
                    int sublen = len - (initBufferRemaining - (int(buffer.size()) - startIndex));
                    std::vector<uint8_t> dsts = std::vector<uint8_t>(sublen);
                    std::copy(buffer.begin() + startIndex, buffer.begin() + startIndex + sublen, dsts.begin());
                    startIndex += sublen;
                }
                break;
            case ROUTE_SELECTION_DESCRIPTOR_COMPONENT_NON_3GPP_OFFLOAD:
                break;
            default:
                int sublen = len - (initBufferRemaining - (int(buffer.size()) - startIndex));
                std::vector<uint8_t> dsts = std::vector<uint8_t>(sublen);
                for (int i = 0; i < sublen; ++i) {
                    dsts[i] = buffer[startIndex];
                    startIndex++;
                }
                ret = DECODE_FAIL_UNKNOWN_IDENTIFIER;
                break;
        }
        if (ret != DECODE_SUCCESS) {
            return ret;
        }
    }
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeOsIdOsAppId(int& startIndex, std::vector<uint8_t> buffer,
    TrafficDescriptor& trafficDescriptor, bool isAppIdOnly)
{
    if ((!isAppIdOnly) && ((int(buffer.size()) - startIndex) < (NetworkSliceCommConfig::LEN_SIXTEEN_BYTE
        + NetworkSliceCommConfig::LEN_BYTE))) {
        NETMGR_EXT_LOG_E("buffer.remaining() < osId Len");
        return DECODE_FAIL_OTHER;
    }
    std::string osId = "";
    if (!isAppIdOnly) {
        for (int i = 0; i < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE; i++) {
            osId += ByteToHexStr(buffer[startIndex++]);
        }
    }
    OsAppId osAppId;
    osAppId.setOsId(osId);
    uint8_t stringLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < stringLen) {
        NETMGR_EXT_LOG_E("buffer.remaining() < appId Len");
        return DECODE_FAIL_OTHER;
    }
    std::string stringBuffer = "";
    for (int i = 0; i < stringLen; i++) {
        stringBuffer += static_cast<char>(buffer[startIndex++]);
    }
    NETMGR_EXT_LOG_I("DecodeOsId:%{public}s, OsAppId:%{public}s", osId.c_str(), stringBuffer.c_str());
    osAppId.setAppId(stringBuffer);
    trafficDescriptor.osAppIds.push_back(osAppId);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeIpv4Addr(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    std::vector<uint8_t> ipv4(NetworkSliceCommConfig::LEN_INT, 0);
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_EIGHT_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Ipv4Addr Len");
        return DECODE_FAIL_OTHER;
    }
    for (int i = 0; i < NetworkSliceCommConfig::LEN_INT; i++) {
        ipv4[i] = buffer[startIndex++];
    }
    Ipv4Addr ipv4Addr;
    ipv4Addr.setIpv4Addr(vectorToUint32(ipv4));
    for (int i = 0; i < NetworkSliceCommConfig::LEN_INT; i++) {
        ipv4[i] = buffer[startIndex++];
    }
    ipv4Addr.setIpv4Mask(vectorToUint32(ipv4));
    trafficDescriptor.ipv4Addrs.push_back(ipv4Addr);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeIpv6Addr(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    std::vector<uint8_t> ipv6(NetworkSliceCommConfig::LEN_SIXTEEN_BYTE, 0);
    if ((int(buffer.size()) - startIndex) <
        NetworkSliceCommConfig::LEN_SIXTEEN_BYTE + NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Ipv6Addr Len");
        return DECODE_FAIL_OTHER;
    }
    for (int i = 0; i < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE; i++) {
        ipv6[i] = buffer[startIndex++];
    }
    Ipv6Addr ipv6Addr;
    ipv6Addr.setIpv6Addr(vectorToIPv6Array(ipv6));
    ipv6Addr.setIpv6PrefixLen(static_cast<int>(buffer[startIndex++]));
    trafficDescriptor.ipv6Addrs.push_back(ipv6Addr);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeProtocolId(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Protocol identifier/next header type Len");
        return DECODE_FAIL_OTHER;
    }
    trafficDescriptor.protocolIds.push_back(static_cast<int>(buffer[startIndex++]));
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeSingleRemotePort(int& startIndex, std::vector<uint8_t> buffer,
    TrafficDescriptor& trafficDescriptor)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_SHORT) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Single remote port type Len");
        return DECODE_FAIL_OTHER;
    }
    trafficDescriptor.singleRemotePorts.push_back(ConvertUnsignedShort2Int(GetShort(startIndex, buffer)));
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeRemotePortRange(int& startIndex,
    std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_INT) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Remote port range type Len");
        return DECODE_FAIL_OTHER;
    }
    RemotePortRange portRang;
    portRang.setPortRangeLowLimit(ConvertUnsignedShort2Int(GetShort(startIndex, buffer)));
    portRang.setPortRangeHighLimit(ConvertUnsignedShort2Int(GetShort(startIndex, buffer)));
    trafficDescriptor.remotePortRanges.push_back(portRang);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeTrafficDescriptorDnn(int& startIndex, std::vector<uint8_t> buffer,
    TrafficDescriptor& trafficDescriptor)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < LEN_BYTE for dnn type");
        return DECODE_FAIL_OTHER;
    }
    uint8_t stringLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < stringLen) {
        NETMGR_EXT_LOG_E("buffer.remaining()[%{public}d] < dnn Len[%{public}d]",
            (int(buffer.size()) - startIndex), stringLen);
        return DECODE_FAIL_OTHER;
    }
    std::string stringBuffer = DecodeSubDnns(startIndex, buffer, stringLen);
    if (stringBuffer == "") {
        return DECODE_FAIL_OTHER;
    }
    std::string dnn = stringBuffer;
    trafficDescriptor.dnns.push_back(dnn);
    if ("ims" == dnn || "IMS" == dnn) {
        mIsTrafficDescriptorIncludeIms = true;
    }
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeFqdn(int& startIndex, std::vector<uint8_t> buffer, TrafficDescriptor& trafficDescriptor)
{
    if (mUrspVersion == NetworkSliceCommConfig::URSP_VERSION_1510) {
        NETMGR_EXT_LOG_E("mUrspVersion = 1510 but Traffic descriptor is FQDN");
        return DECODE_FAIL_UNKNOWN_IDENTIFIER;
    }
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < LEN_BYTE for fqdn type");
        return DECODE_FAIL_OTHER;
    }
    uint8_t stringLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < stringLen) {
        NETMGR_EXT_LOG_E("buffer.remaining() < fqdn Len");
        return DECODE_FAIL_OTHER;
    }
    std::string stringBuffer = DecodeSubDnns(startIndex, buffer, stringLen);
    if (stringBuffer == "") {
        return DECODE_FAIL_OTHER;
    }
    trafficDescriptor.fqdns.push_back(stringBuffer);
    return DECODE_SUCCESS;
}

std::string UrspConfig::DecodeSubDnns(int& startIndex, std::vector<uint8_t> buffer, uint8_t stringLen)
{
    std::string stringBuffer = "";
    int cnt = 0;
    for (int i = 0; i < (int)stringLen; i++) {
        if (cnt == 0) {
            cnt = buffer[startIndex++];
            if (cnt > stringLen - i - 1) {
                NETMGR_EXT_LOG_E("buffer remaining [%{public}d] < sub dnn len [%{public}d]", (stringLen - i), cnt);
                return "";
            }
            if (i > 0) {
                stringBuffer += ".";
            }
        } else {
            stringBuffer += static_cast<char>(buffer[startIndex++]);
            cnt--;
        }
    }
    if (cnt > 0) {
        NETMGR_EXT_LOG_E("can not get full sub dnn [%{public}d]", cnt);
        return "";
    }
    return stringBuffer;
}

int UrspConfig::DecodeConnectionCapabilities(int& startIndex, std::vector<uint8_t> buffer,
    TrafficDescriptor& trafficDescriptor)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < LEN_BYTE for Connection capabilities type");
        return DECODE_FAIL_OTHER;
    }
    uint8_t stringLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < stringLen) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Connection capabilities type len");
        return DECODE_FAIL_OTHER;
    }
    for (int i = 0; i < stringLen; i++) {
        int connCapability = static_cast<int>(buffer[startIndex++]);
        trafficDescriptor.connectionCapabilities.push_back(connCapability);
        if ((connCapability & CONNECTION_CAPABILITY_TYPE_IMS) != 0) {
            mIsTrafficDescriptorIncludeIms = true;
        }
    }
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeSscMode(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < SSC mode type len");
        return DECODE_FAIL_OTHER;
    }
    /*
    * The bits 8 through 4 of the octet shall be spare,
    * and the bits 3 through 1 shall be encoded as the value part of the SSC mode
    */
    routeRule.sscMode = buffer[startIndex++] & 0x07;
    NETMGR_EXT_LOG_I("SSC mode:%{public}d", routeRule.sscMode);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeSnssai(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < LEN_BYTE for S-NSSAI type");
        return DECODE_FAIL_OTHER;
    }
    uint8_t subLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < subLen) {
        NETMGR_EXT_LOG_E("buffer.remaining() < S-NSSAI type len");
        return DECODE_FAIL_OTHER;
    }
    Snssai snssai;
    if (!sAllowedNssaiConfig_->DecodeSnssai(startIndex, subLen, buffer, snssai)) {
        NETMGR_EXT_LOG_E("AllowedNssaiConfig.decodeSNssai(subLen, buffer, sNssai) is false");
        return DECODE_FAIL_OTHER;
    }
    routeRule.snssais.push_back(snssai);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodeDnn(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < LEN_BYTE for dnn type");
        return DECODE_FAIL_OTHER;
    }
    uint8_t stringLen = buffer[startIndex++];
    if ((int(buffer.size()) - startIndex) < stringLen) {
        NETMGR_EXT_LOG_E("buffer.remaining()[%{public}d] < dnn Len[%{public}d]",
            (int(buffer.size()) - startIndex), stringLen);
        return DECODE_FAIL_OTHER;
    }
    std::string stringBuffer = DecodeSubDnns(startIndex, buffer, stringLen);
    if (stringBuffer == "") {
        return DECODE_FAIL_OTHER;
    }
    std::string dnn = stringBuffer;
    NETMGR_EXT_LOG_I("DNN:%{public}s", dnn.c_str());
    routeRule.dnns.push_back(dnn);
    return DECODE_SUCCESS;
}

int UrspConfig::DecodePduSessionType(int& startIndex, std::vector<uint8_t> buffer, RouteSelectionDescriptor &routeRule)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < Preferred access type len");
        return DECODE_FAIL_OTHER;
    }
    /*
    * The bits 8 through 4 of the octet shall be spare,
    * and the bits 3 through 1 shall be encoded as the value part of the PDU session type
    */
    routeRule.pduSessionType = TransferPduSessionTypeToHal(buffer[startIndex++] & 0x07);
    NETMGR_EXT_LOG_I("decodePduSessionType pduSessionType = %{public}d", routeRule.pduSessionType);
    return DECODE_SUCCESS;
}

int UrspConfig::TransferPduSessionTypeToHal(int pduSessionType)
{
    NETMGR_EXT_LOG_I("transferPduSessionTypeToHal pduSessionType = %{public}d", pduSessionType);
    switch (pduSessionType) {
        case PROTOCOL_PDU_SESSION_TYPE_IPV4:
            return HAL_PDU_SESSION_TYPE_IP;
        case PROTOCOL_PDU_SESSION_TYPE_IPV6:
            return HAL_PDU_SESSION_TYPE_IPV6;
        case PROTOCOL_PDU_SESSION_TYPE_IPV4V6:
            return HAL_PDU_SESSION_TYPE_IPV4V6;
        case PROTOCOL_PDU_SESSION_TYPE_UNSTRUCTURED:
            return HAL_PDU_SESSION_TYPE_UNSTRUCTURED;
        case PROTOCOL_PDU_SESSION_TYPE_ETHERNET:
            return HAL_PDU_SESSION_TYPE_NON_IP;
        case PROTOCOL_PDU_SESSION_TYPE_RESERVED:
            return HAL_PDU_SESSION_TYPE_IPV4V6;
        default:
            return HAL_PDU_SESSION_TYPE_IPV4V6;
    }
}

int UrspConfig::DecodePreferredAccessType(int& startIndex, std::vector<uint8_t> buffer,
    RouteSelectionDescriptor &routeRule)
{
    if ((int(buffer.size()) - startIndex) < NetworkSliceCommConfig::LEN_BYTE) {
        NETMGR_EXT_LOG_E("buffer.remaining() < PDU session type len");
        return DECODE_FAIL_OTHER;
    }
    if (buffer[startIndex++] == PREFERRED_ACCESS_TYPE_NON_3GPP) {
        return DECODE_FAIL_UNKNOWN_IDENTIFIER;
    }
    return DECODE_SUCCESS;
}

void UrspConfig::SndPreImsRsdList()
{
    std::vector<uint8_t> rsdBytes = GetImsRsdList();
    if (!rsdBytes.empty()) {
        NETMGR_EXT_LOG_I("UrspConfig::SndPreImsRsdList");
        Telephony::CellularDataClient::GetInstance().SendImsRsdList(MODEM_ID, rsdBytes);
    }
}

std::vector<uint8_t> UrspConfig::GetImsRsdList()
{
    if (mImsRsdsMap.empty()) {
        return std::vector<uint8_t>();
    }
    std::vector<uint8_t> rsdBytes = ConvertRsdList2BufferArray(mImsRsdsMap);
    ResetImsRsdsMap();
    return rsdBytes;
}

std::vector<RouteSelectionDescriptor> UrspConfig::SortRsdsMap(
    std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap)
{
    std::vector<int> keyList;
    for (auto& it: rsdsMap) {
        keyList.push_back(it.first);
    }
    std::sort(keyList.begin(), keyList.end(), [](int a, int b) { return a > b;});

    std::vector<RouteSelectionDescriptor> rsdList;
    for (int& key : keyList) {
        std::vector<RouteSelectionDescriptor> outOfOrderList = rsdsMap[key];
        std::sort(outOfOrderList.begin(), outOfOrderList.end(), [](auto& r1, auto& r2) {
            return r1.routePrecedence < r2.routePrecedence;
        });
        rsdList.insert(rsdList.end(), outOfOrderList.begin(), outOfOrderList.end());
    }
    return rsdList;
}

std::vector<uint8_t> UrspConfig::ConvertRsdList2BufferArray(
    std::unordered_map<int, std::vector<RouteSelectionDescriptor>> rsdsMap)
{
    std::vector<RouteSelectionDescriptor> rsdList = SortRsdsMap(rsdsMap);
    for (size_t i = 0; i < rsdList.size(); ++i) {
        if (sAllowedNssaiConfig_ == nullptr) {
            break;
        }
        if (rsdList[i].snssais.size() == 0) {
            ++i;
            continue;
        }
        if (sAllowedNssaiConfig_->FindSnssaiInAllowedNssai(rsdList[i].snssais) != "") {
            ++i;
            continue;
        } else {
            rsdList.erase(rsdList.begin() + i);
        }
    }
    if (rsdList.size() == 0) {
        return NotifyImsaDelRsdInfo();
    }
    while (rsdList.size() > RSD_MAX_LIST_NUM) {
        rsdList.erase(rsdList.begin() + RSD_MAX_LIST_NUM);
    }
    int dataLen = CalculateRsdListLen();
    int rsdNum = rsdList.size();
    NETMGR_EXT_LOG_I("convertRsdList2Buffer rsdNum = %{public}d, dataLen = %{public}d", rsdNum, dataLen);
    std::vector<uint8_t> rsdBytes;
    if (dataLen == INVALID_VALUE) {
        NETMGR_EXT_LOG_E("dataLen of rsdList is 0 ");
        return rsdBytes;
    }

    /* put version */
    PutShort(rsdBytes, IMSA_VERSION);
    /* put length */
    PutShort(rsdBytes, static_cast<short>(dataLen));
    /* put rsdNum */
    PutInt(rsdBytes, rsdNum);
    /* put rsdInfo */
    PutRsdListInfo(rsdBytes, rsdList);
    return rsdBytes;
}

void UrspConfig::PutRsdListInfo(std::vector<uint8_t>& buffer, std::vector<RouteSelectionDescriptor> rsdList)
{
    int rsdNum = (int)rsdList.size();
    int emptyRsdNum = RSD_MAX_LIST_NUM - rsdNum;
    for (RouteSelectionDescriptor rsd : rsdList) {
        /* put routePrecedence and 3B reserve */
        buffer.push_back(ConvertInt2UnsignedByte(rsd.routePrecedence));
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);

        /* put isExistSscMode */
        if (rsd.sscMode == 0) {
            buffer.push_back(0);
        } else {
            buffer.push_back(1);
        }
        /* put sscMode */
        buffer.push_back(rsd.sscMode);
        /* put pduSessionType */
        buffer.push_back(ConvertInt2UnsignedByte(ConvertPduTypeFromHal2Imsa(rsd.pduSessionType)));
        /* put 2B(reserve) */
        buffer.push_back(0);
        buffer.push_back(0);
        /* put dnnsInfo */
        PutDnnsInfo(buffer, rsd);
        /* put nssaisInfo */
        PutNssaisInfo(buffer, rsd);
        /* put 4B reserve */
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
        buffer.push_back(0);
    }
    if (emptyRsdNum > 0) {
        int rsdLen = CalculateRsdLen();
        int nullLen = rsdLen * emptyRsdNum;
        PutInvalidValue(buffer, nullLen);
    }
}

void UrspConfig::PutDnnsInfo(std::vector<uint8_t>& buffer, RouteSelectionDescriptor rsd)
{
    while (rsd.dnns.size() > RSD_MAX_DNN_NUM) {
        rsd.dnns.erase(rsd.dnns.begin() + RSD_MAX_DNN_NUM);
    }
    int dnnNum = rsd.dnns.size();
    int emptyNum = RSD_MAX_DNN_NUM - dnnNum;

    /* put dnnNum */
    PutInt(buffer, dnnNum);

    /* put dnns */
    for (std::string dnn : rsd.dnns) {
        std::vector<uint8_t> dnnBytes = ConvertstringTouInt8Vector(dnn);
        int dnnLen = dnnBytes.size();
        int nullNum = RSD_MAX_DNN_LEN - dnnLen;

        /* put dnnLen */
        buffer.push_back(ConvertInt2UnsignedByte(dnnLen));

        /* put dnnValue */
        buffer.insert(buffer.end(), dnnBytes.begin(), dnnBytes.end());

        if (nullNum > 0) {
            PutInvalidValue(buffer, nullNum);
        }
    }

    if (emptyNum > 0) {
        int nullNum = emptyNum * (RSD_MAX_DNN_LEN + NetworkSliceCommConfig::LEN_BYTE);
        PutInvalidValue(buffer, nullNum);
    }
}

void UrspConfig::PutNssaisInfo(std::vector<uint8_t>& buffer, RouteSelectionDescriptor rsd)
{
    for (int i = 0; i < (int)rsd.snssais.size();) {
        if (sAllowedNssaiConfig_ == nullptr) {
            break;
        }
        if (sAllowedNssaiConfig_->isSnssaiInAllowedNssai(rsd.snssais[i]) != "") {
            i++;
            continue;
        } else {
            rsd.snssais.erase(rsd.snssais.begin() + i);
        }
    }
    while (rsd.snssais.size() > RSD_MAX_NSSAI_NUM) {
        rsd.snssais.erase(rsd.snssais.begin() + RSD_MAX_NSSAI_NUM);
    }
    int snssaiNum = (int)rsd.snssais.size();
    int emptyNum = RSD_MAX_NSSAI_NUM - snssaiNum;
    /* put snssaiNum */
    PutInt(buffer, snssaiNum);
    /* put nssaiInfo */
    for (Snssai snssai : rsd.snssais) {
        /* put bitOption */
        int bitOpt = 0;
        if (snssai.getSd() != 0) {
            bitOpt = SetBitOpt(bitOpt, FIRSTBYTE);
        }
        if (snssai.getMappedSst() != 0) {
            bitOpt = SetBitOpt(bitOpt, SECONDBYTE);
        }
        if (snssai.getMappedSd() != 0) {
            bitOpt = SetBitOpt(bitOpt, THIRDBYTE);
        }

        PutInt(buffer, bitOpt);
        /* put sst */
        buffer.push_back(snssai.getSst());

        /* put mappedSst */
        buffer.push_back(snssai.getMappedSst());

        /* put 2B reserve */
        buffer.push_back(0);
        buffer.push_back(0);

        /* put sd */
        PutInt(buffer, snssai.getSd());

        /* put mappedSd */
        PutInt(buffer, snssai.getMappedSd());
    }

    if (emptyNum > 0) {
        /* 4B(bitOption) + 1B(sst) + 1B(mappedSst) + 2B(reserve) + 4B(sd) + 4B(mappedSd) */
        int snssaiLen = (4 + 1 + 1 + 2 + 4 + 4) * NetworkSliceCommConfig::LEN_BYTE;
        int nullNum = emptyNum * snssaiLen;
        PutInvalidValue(buffer, nullNum);
    }
}

int UrspConfig::SetBitOpt(int num, int position)
{
    int temp = 1;
    temp <<= position - 1;
    num = num | temp;
    return num;
}


int UrspConfig::ConvertPduTypeFromHal2Imsa(int halPduType)
{
    return (halPduType + 1);
}

std::vector<uint8_t> UrspConfig::NotifyImsaDelRsdInfo()
{
    std::vector<uint8_t> buffer;
    /* put version */
    PutShort(buffer, IMSA_VERSION);
    /* put length */
    PutShort(buffer, 0);
    return buffer;
}

void UrspConfig::PutInvalidValue(std::vector<uint8_t>& buffer, int num)
{
    for (int i = 0; i < num; i++) {
        buffer.push_back(0);
    }
}

short UrspConfig::CalculateRsdListLen()
{
    short dataLen = 0;
    /* 4B RsdNum */
    dataLen += NetworkSliceCommConfig::LEN_INT;
    dataLen += CalculateRsdLen() * RSD_MAX_LIST_NUM;
    return dataLen;
}

short UrspConfig::CalculateRsdLen()
{
    short dataLen = 0;
    /* 4B = 1B(routePrecedence) + 3B(reserve) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;

    /* 4B = 1B(isExist) + 1B(sscMode) + 2B(reserve) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;

    /* 4B = 1B(isExist) + 1B(pduType) + 2B(reserve) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;

    /* 4B(dnnNum) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;
    dataLen += (RSD_MAX_DNN_LEN + NetworkSliceCommConfig::LEN_BYTE) * RSD_MAX_DNN_NUM;

    /* 4B(nssaiNum) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;

    /* 4B(bitOption) + 1B(sst) + 1B(mappedSst) + 2B(reserve) + 4B(sd) + 4B(mappedSd) */
    short snssaiLen = (4 + 1 + 1 + 2 + 4 + 4) * NetworkSliceCommConfig::LEN_BYTE;
    dataLen += snssaiLen * RSD_MAX_NSSAI_NUM;

    /* 4B(reserve) */
    dataLen += 4 * NetworkSliceCommConfig::LEN_BYTE;
    return dataLen;
}

bool UrspConfig::SliceNetworkSelection(
    SelectedRouteDescriptor& routeRule, std::string plmn, AppDescriptor appDescriptor)
{
    NETMGR_EXT_LOG_I("SliceNetworkSelection");
    std::unordered_map<std::string, std::vector<UrspRule>> uePolicyMap =
        mUePolicyMap.empty() ? mPreConfigUrspMap : mUePolicyMap;
    std::vector<UrspRule> urspRules = uePolicyMap.begin()->second;
    UrspRule urspRule;
    for (size_t i = 0; i < urspRules.size(); i++) {
        urspRule = urspRules[i];
        if (!isTrafficDescriptorMatch(urspRule.trafficDescriptor, appDescriptor)) {
            NETMGR_EXT_LOG_I("!isTrafficDescriptorMatch");
            continue;
        }
        if (!FindAvailableRouteRule(urspRule.routeSelectionDescriptors, routeRule)) {
            NETMGR_EXT_LOG_I("Not FindAvailableRouteRule");
            continue;
        }
        routeRule.setUrspPrecedence(static_cast<uint8_t>(urspRule.urspPrecedence));
        FillTrafficDescriptor(urspRule.trafficDescriptor, appDescriptor, routeRule);
        return true;
    }
    return false;
}

bool UrspConfig::FindAvailableRouteRule(
    const std::vector<RouteSelectionDescriptor>& routeSelectionDescriptors, SelectedRouteDescriptor& routeRule)
{
    NETMGR_EXT_LOG_I("FindAvailableRouteRule size = %{public}d", (int)routeSelectionDescriptors.size());
    for (size_t i = 0; i < routeSelectionDescriptors.size(); i++) {
        routeRule.setPduSessionType(routeSelectionDescriptors[i].pduSessionType);
        routeRule.setSscMode(routeSelectionDescriptors[i].sscMode);
        if (DelayedSingleton<NetworkSliceManager>::GetInstance()->isRouteRuleInForbiddenList(routeRule)) {
            NETMGR_EXT_LOG_I("isRouteRuleInForbiddenList");
            continue;
        }
        if (!routeSelectionDescriptors[i].snssais.empty() && !routeSelectionDescriptors[i].dnns.empty()) {
            if (!FindAvailableSnssaiAndDnn(routeSelectionDescriptors[i], routeRule)) {
                continue;
            }
        }
        if (!routeSelectionDescriptors[i].snssais.empty() && routeSelectionDescriptors[i].dnns.empty()) {
            if (!FindAvailableSnssai(routeSelectionDescriptors[i], routeRule)) {
                continue;
            }
        }
        if (routeSelectionDescriptors[i].snssais.empty() && !routeSelectionDescriptors[i].dnns.empty()) {
            if (!FindAvailableDnn(routeSelectionDescriptors[i], routeRule)) {
                continue;
            }
        }
        return true;
    }
    return false;
}

bool UrspConfig::FindAvailableSnssaiAndDnn(const RouteSelectionDescriptor& routeSelectionDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    NETMGR_EXT_LOG_I("FindAvailableSnssaiAndDnn");
    for (size_t i = 0; i < routeSelectionDescriptor.snssais.size(); i++) {
        if (sAllowedNssaiConfig_ == nullptr) {
            continue;
        }
        Snssai snssai = routeSelectionDescriptor.snssais[i];
        std::string stringSnssai = sAllowedNssaiConfig_->isSnssaiInAllowedNssai(snssai);
        NETMGR_EXT_LOG_I("FindAvailableSnssaiAndDnn stringSnssai = %{public}s", stringSnssai.c_str());
        if (stringSnssai.empty()) {
            continue;
        }
        routeRule.setSnssai(stringSnssai);
        for (size_t j = 0; j < routeSelectionDescriptor.dnns.size(); j++) {
            std::string dnn = routeSelectionDescriptor.dnns[j];
            routeRule.setDnn(dnn);
            NETMGR_EXT_LOG_I("FindAvailableDnn:%{public}s", dnn.c_str());
            if (DelayedSingleton<NetworkSliceManager>::GetInstance()->isRouteRuleInForbiddenList(routeRule)) {
                continue;
            }
            return true;
        }
    }
    return false;
}

bool UrspConfig::FindAvailableSnssai(const RouteSelectionDescriptor& routeSelectionDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    for (size_t i = 0; i < routeSelectionDescriptor.snssais.size(); i++) {
        if (sAllowedNssaiConfig_ == nullptr) {
            continue;
        }
        Snssai snssai = routeSelectionDescriptor.snssais[i];
        NETMGR_EXT_LOG_I("FindAvailableSnssai snssai = %{public}s", snssai.getSnssai().c_str());
        std::string stringSnssai = sAllowedNssaiConfig_->isSnssaiInAllowedNssai(snssai);
        if (stringSnssai.empty()) {
            continue;
        }
        routeRule.setSnssai(stringSnssai);
        if (DelayedSingleton<NetworkSliceManager>::GetInstance()->isRouteRuleInForbiddenList(routeRule)) {
            continue;
        }
        return true;
    }
    return false;
}

bool UrspConfig::FindAvailableDnn(const RouteSelectionDescriptor& routeSelectionDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    for (size_t j = 0; j < routeSelectionDescriptor.dnns.size(); j++) {
        const auto& dnn = routeSelectionDescriptor.dnns[j];
        routeRule.setDnn(dnn);
        if (DelayedSingleton<NetworkSliceManager>::GetInstance()->isRouteRuleInForbiddenList(routeRule)) {
            continue;
        }
        return true;
    }
    return false;
}
/**
    * judge if has available ursp rule
    *
    * @return true: has available ursp rule false: do not have available ursp rule
    */
bool UrspConfig::hasAvailableUrspRule()
{
    return (!mUePolicyMap.empty() || !mPreConfigUrspMap.empty());
}

void UrspConfig::FillTrafficDescriptor(TrafficDescriptor urspTrafficDescriptor,
    AppDescriptor appDescriptor, SelectedRouteDescriptor& routeRule)
{
    NETMGR_EXT_LOG_I("FillTrafficDescriptor");
    uint8_t routeBitmap = 0;

    if (urspTrafficDescriptor.isMatchAll) {
        routeBitmap = static_cast<uint8_t>(routeBitmap | 0x01);
        routeRule.setRouteBitmap(routeBitmap);
        return;
    }
    if (!urspTrafficDescriptor.osAppIds.empty()) {
        FillOsAppIds(urspTrafficDescriptor, routeRule);
    }
    if (!urspTrafficDescriptor.ipv4Addrs.empty()) {
        FillIpv4Addrs(urspTrafficDescriptor, routeRule);
    }
    if (!urspTrafficDescriptor.ipv6Addrs.empty()) {
        FillIpv6Addrs(urspTrafficDescriptor, routeRule);
    }
    if (!urspTrafficDescriptor.protocolIds.empty()) {
        FillProtocolIds(urspTrafficDescriptor, routeRule);
    }
    if (!urspTrafficDescriptor.dnns.empty()) {
        routeBitmap = static_cast<uint8_t>(routeBitmap | 0x02);
        routeRule.setDnn(appDescriptor.getDnn());
    }
    if (!urspTrafficDescriptor.fqdns.empty()) {
        routeBitmap = static_cast<uint8_t>(routeBitmap | 0x04);
    }
    if (!urspTrafficDescriptor.connectionCapabilities.empty()) {
        routeBitmap = static_cast<uint8_t>(routeBitmap | 0x08);
    }
    routeRule.setRouteBitmap(routeBitmap);
    NETMGR_EXT_LOG_I("fillTrafficDescriptor routeBitmap = %{public}d", static_cast<int>(routeBitmap));
    FillRemotePorts(urspTrafficDescriptor, routeRule);
}

void UrspConfig::FillOsAppIds(TrafficDescriptor urspTrafficDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    std::string osAppIdsStr;
    for (size_t i = 0; i < urspTrafficDescriptor.osAppIds.size(); ++i) {
        std::string combinedOsAppId = urspTrafficDescriptor.osAppIds[i].getOsId()
            + "#" + urspTrafficDescriptor.osAppIds[i].getAppId();
        osAppIdsStr += combinedOsAppId;
        if (i < urspTrafficDescriptor.osAppIds.size() - 1) {
            osAppIdsStr += COMMA_SEPARATOR;
        }
    }
    routeRule.setAppIds(osAppIdsStr);
}

void UrspConfig::FillIpv4Addrs(TrafficDescriptor urspTrafficDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    uint8_t ipv4Num = static_cast<uint8_t>(urspTrafficDescriptor.ipv4Addrs.size());
    std::vector<uint8_t> ipv4AddrAndMask(ipv4Num *
        (NetworkSliceCommConfig::LEN_INT + NetworkSliceCommConfig::LEN_INT, 0));
    size_t index = 0;
    for (const auto& ipv4Addr : urspTrafficDescriptor.ipv4Addrs) {
        uint32_t ipv4AddrValues = ipv4Addr.getIpv4Addr();
        uint32_t ipv4MaskValues = ipv4Addr.getIpv4Mask();
        std::vector<uint8_t> ipv4AddrValue_vec = uInt32ToVector(ipv4AddrValues);
        std::vector<uint8_t> ipv4MaskValues_vec = uInt32ToVector(ipv4MaskValues);
        for (size_t j = 0; j < NetworkSliceCommConfig::LEN_INT; ++j) {
            ipv4AddrAndMask[index++] = ipv4AddrValue_vec[j];
        }
        for (size_t j = 0; j < NetworkSliceCommConfig::LEN_INT; ++j) {
            ipv4AddrAndMask[index++] = ipv4MaskValues_vec[j];
        }
    }
    routeRule.setIpv4Num(ipv4Num);
    routeRule.setIpv4AddrAndMask(ipv4AddrAndMask);
}

void UrspConfig::FillIpv6Addrs(TrafficDescriptor urspTrafficDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    size_t ipv6Num = urspTrafficDescriptor.ipv6Addrs.size();
    size_t index = 0;
    std::vector<uint8_t> ipv6AddrAndPrefix((NetworkSliceCommConfig::LEN_SIXTEEN_BYTE +
        NetworkSliceCommConfig::LEN_BYTE) * ipv6Num, 0);
    for (size_t i = 0; i < ipv6Num; i++) {
        Ipv6Addr ipv6Addr = urspTrafficDescriptor.ipv6Addrs[i];
        std::array<uint8_t, NetworkSliceCommConfig::LEN_IPV6ADDR> ipv6AddrValues_arry = ipv6Addr.getIpv6Addr();
        for (size_t j = 0; j < NetworkSliceCommConfig::LEN_SIXTEEN_BYTE; ++j) {
            ipv6AddrAndPrefix[index++] = ipv6AddrValues_arry[j];
        }
        int ipv6PrefixLen = ipv6Addr.getIpv6PrefixLen();
        ipv6AddrAndPrefix[index++] = ipv6PrefixLen;
    }
    routeRule.setIpv6Num(ipv6Num);
    routeRule.setIpv6AddrAndPrefix(ipv6AddrAndPrefix);
}

void UrspConfig::FillProtocolIds(TrafficDescriptor urspTrafficDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    std::string protocolIds = ConvertIntListToString(urspTrafficDescriptor.protocolIds);
    routeRule.setProtocolIds(protocolIds);
}

void UrspConfig::FillRemotePorts(TrafficDescriptor urspTrafficDescriptor,
    SelectedRouteDescriptor& routeRule)
{
    std::string remotePorts = ConvertIntListToString(urspTrafficDescriptor.singleRemotePorts);
    if (!urspTrafficDescriptor.remotePortRanges.empty()) {
        if (!remotePorts.empty()) {
            remotePorts += COMMA_SEPARATOR;
        }
        RemotePortRange remotePortRange;
        for (size_t i = 0; i < urspTrafficDescriptor.remotePortRanges.size(); i++) {
            remotePortRange = urspTrafficDescriptor.remotePortRanges[i];
            remotePorts += std::to_string(remotePortRange.getPortRangeLowLimit())
                + PORT_RANGE_SEPARATOR + std::to_string(remotePortRange.getPortRangeHighLimit());
            if (i < urspTrafficDescriptor.remotePortRanges.size() - 1) {
                remotePorts += COMMA_SEPARATOR;
            }
        }
    }
    routeRule.setRemotePorts(remotePorts);
}

bool UrspConfig::isTrafficDescriptorMatch(TrafficDescriptor urspTrafficDescriptor,
    AppDescriptor appDescriptor)
{
    if (urspTrafficDescriptor.isMatchAll) {
        NETMGR_EXT_LOG_I("urspTrafficDescriptor.isMatchAll == true");
        return true;
    }
    if (!isOsAppIdMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    if (!isIpv4AddrMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    if (!isIpv6AddrMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    if (!urspTrafficDescriptor.protocolIds.empty()
        && std::find(urspTrafficDescriptor.protocolIds.begin(), urspTrafficDescriptor.protocolIds.end(),
            appDescriptor.getProtocolId()) == urspTrafficDescriptor.protocolIds.end()) {
        NETMGR_EXT_LOG_I("protocolId not match: %{public}d", appDescriptor.getProtocolId());
        return false;
    }
    if (!urspTrafficDescriptor.dnns.empty()
        && std::find(urspTrafficDescriptor.dnns.begin(), urspTrafficDescriptor.dnns.end(),
            appDescriptor.getDnn()) == urspTrafficDescriptor.dnns.end()) {
        return false;
    }
    if (!urspTrafficDescriptor.fqdns.empty()
        && std::find(urspTrafficDescriptor.fqdns.begin(), urspTrafficDescriptor.fqdns.end(),
            appDescriptor.getFqdn()) == urspTrafficDescriptor.fqdns.end()) {
        NETMGR_EXT_LOG_I("fqdn not match: %{public}s", appDescriptor.getFqdn().c_str());
        return false;
    }
    if (!urspTrafficDescriptor.connectionCapabilities.empty()
        && std::find(urspTrafficDescriptor.connectionCapabilities.begin(),
            urspTrafficDescriptor.connectionCapabilities.end(),
            appDescriptor.getConnectionCapability()) == urspTrafficDescriptor.connectionCapabilities.end()) {
        NETMGR_EXT_LOG_I("connectionCapabilities not match: %{public}d", appDescriptor.getConnectionCapability());
        return false;
    }

    if (!isRemotePortMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    return true;
}

bool UrspConfig::isIpThreeTuplesInWhiteList(std::string plmn, AppDescriptor appDescriptor)
{
    std::unordered_map<std::string, std::vector<UrspRule>> uePolicyMap =
        mUePolicyMap.empty() ? mPreConfigUrspMap : mUePolicyMap;
    if (uePolicyMap.find(plmn) == uePolicyMap.end()) {
        NETMGR_EXT_LOG_I("cannot find plmn in sliceNetworkSelection");
        return false;
    }
    std::vector<UrspRule> urspRules = uePolicyMap[plmn];
    for (size_t i = 0; i < urspRules.size(); ++i) {
        UrspRule urspRule = urspRules[i];
        if (isIpThreeTuplesInTrafficDescriptor(urspRule.trafficDescriptor, appDescriptor)) {
            return true;
        }
    }
    return false;
}

bool UrspConfig::isIpThreeTuplesInTrafficDescriptor(TrafficDescriptor urspTrafficDescriptor,
    AppDescriptor appDescriptor)
{
    if (urspTrafficDescriptor.ipv4Addrs.empty()
        && urspTrafficDescriptor.ipv6Addrs.empty()
        && urspTrafficDescriptor.protocolIds.empty()
        && urspTrafficDescriptor.singleRemotePorts.empty()
        && urspTrafficDescriptor.remotePortRanges.empty()) {
        return false;
    }
    if (!isIpv4AddrMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    if (!isIpv6AddrMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    if (!urspTrafficDescriptor.protocolIds.empty()
        && std::find(urspTrafficDescriptor.protocolIds.begin(), urspTrafficDescriptor.protocolIds.end(),
            appDescriptor.getProtocolId()) == urspTrafficDescriptor.protocolIds.end()) {
        NETMGR_EXT_LOG_I("protocolId not in white list: %{public}d", appDescriptor.getProtocolId());
        return false;
    }
    if (!isRemotePortMatch(urspTrafficDescriptor, appDescriptor)) {
        return false;
    }
    NETMGR_EXT_LOG_I("is IpThreeTuples In TrafficDescriptor");
    return true;
}

bool UrspConfig::isOsAppIdMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor)
{
    if (!urspTrafficDescriptor.osAppIds.empty()) {
        size_t i;
        for (i = 0; i < urspTrafficDescriptor.osAppIds.size(); i++) {
            OsAppId osAppId = urspTrafficDescriptor.osAppIds[i];
            if (osAppId.getAppId() == appDescriptor.getOsAppId().getAppId()) {
                break;
            }
        }
        if (i == urspTrafficDescriptor.osAppIds.size()) {
            NETMGR_EXT_LOG_I("appId not match: %{public}s", appDescriptor.getOsAppId().getAppId().c_str());
            return false;
        }
        NETMGR_EXT_LOG_I("appId match: %{public}s", appDescriptor.getOsAppId().getAppId().c_str());
    }
    return true;
}

bool UrspConfig::isRemotePortMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor)
{
    if (!urspTrafficDescriptor.singleRemotePorts.empty()
        && std::find(urspTrafficDescriptor.singleRemotePorts.begin(), urspTrafficDescriptor.singleRemotePorts.end(),
            appDescriptor.getRemotePort()) != urspTrafficDescriptor.singleRemotePorts.end()) {
        return true;
    }

    if (!urspTrafficDescriptor.remotePortRanges.empty()) {
        RemotePortRange remotePortRange;
        size_t i;
        for (i = 0; i < urspTrafficDescriptor.remotePortRanges.size(); i++) {
            remotePortRange = urspTrafficDescriptor.remotePortRanges[i];
            if ((appDescriptor.getRemotePort() >= remotePortRange.getPortRangeLowLimit())
                && (appDescriptor.getRemotePort() <= remotePortRange.getPortRangeHighLimit())) {
                break;
            }
        }
        if (i == urspTrafficDescriptor.remotePortRanges.size()) {
            return false;
        }
    }
    return true;
}

bool UrspConfig::isIpv4AddrMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor)
{
    if (urspTrafficDescriptor.ipv4Addrs.empty()) {
        return true;
    }
    size_t i;
    bool ipv4match = false;
    for (i = 0; i < urspTrafficDescriptor.ipv4Addrs.size(); ++i) {
        Ipv4Addr ipv4Addr = urspTrafficDescriptor.ipv4Addrs[i];
        uint32_t ipv4AddrValues = ipv4Addr.getIpv4Addr();
        uint32_t ipv4MaskValues = ipv4Addr.getIpv4Mask();
        uint32_t appAddrValues = appDescriptor.getIpv4Addr();
        if ((appAddrValues & ipv4MaskValues) != (ipv4AddrValues & ipv4MaskValues)) {
            NETMGR_EXT_LOG_I("ipv4Addr not match");
            continue;
        }
        ipv4match = true;
    }
    if (ipv4match == false) {
        NETMGR_EXT_LOG_I("ipv4Addr not match");
        return false;
    }
    return true;
}

bool UrspConfig::isIpv6AddrMatch(TrafficDescriptor urspTrafficDescriptor, AppDescriptor appDescriptor)
{
    if (urspTrafficDescriptor.ipv6Addrs.empty()) {
        return true;
    }
    size_t i;
    bool ipv6match = false;
    for (i = 0; i < urspTrafficDescriptor.ipv6Addrs.size(); ++i) {
        Ipv6Addr ipv6Addr = urspTrafficDescriptor.ipv6Addrs[i];
        std::string ipv6AddrValues = transIpv6AddrToStr(ipv6Addr.getIpv6Addr());
        std::string appAddrValues = transIpv6AddrToStr(appDescriptor.getIpv6Addr());
        if (ipv6AddrValues != appAddrValues) {
            NETMGR_EXT_LOG_I("ipv6Addr not match");
            continue;
        }
        ipv6match = true;
    }
    if (ipv6match == false) {
        NETMGR_EXT_LOG_I("ipv6Addr not match");
        return false;
    }
    return true;
}

SelectedRouteDescriptor UrspConfig::GetMatchAllUrspRule(const std::string& plmn)
{
    NETMGR_EXT_LOG_I("GetMatchAllUrspRule");
    std::unordered_map<std::string, std::vector<UrspRule>> uePolicyMap;
    if (mUePolicyMap.empty()) {
        uePolicyMap = mPreConfigUrspMap;
    } else {
        uePolicyMap = mUePolicyMap;
    }
    SelectedRouteDescriptor routeRule;
    if (uePolicyMap.find(plmn) == uePolicyMap.end()) {
        NETMGR_EXT_LOG_I("GetMatchAllUrspRule not find plmn");
        return routeRule;
    }
    std::vector<UrspRule> urspRules = uePolicyMap[plmn];
    if (urspRules.empty()) {
        NETMGR_EXT_LOG_I("GetMatchAllUrspRule urspRules is empty");
        return routeRule;
    }
    uint8_t routeBitmap = 0; /* bit 1: is matchAll, bit3: hasUrsp */
    routeBitmap = ConvertInt2UnsignedByte(routeBitmap | 0x08);
    UrspRule urspRule = urspRules.back();
    if (!urspRule.trafficDescriptor.isMatchAll) {
        routeRule.setRouteBitmap(routeBitmap);
        NETMGR_EXT_LOG_I("TD is not MatchAll, routeBitmap = %{public}d", routeBitmap);
        return routeRule;
    }
    for (int j = 0; j < (int)urspRule.routeSelectionDescriptors.size(); ++j) {
        if (!urspRule.routeSelectionDescriptors[j].snssais.empty()) {
            if (sAllowedNssaiConfig_ == nullptr) {
                continue;
            }
            std::vector<Snssai> snssais = urspRule.routeSelectionDescriptors[j].snssais;
            std::string snssai = sAllowedNssaiConfig_->FindSnssaiInAllowedNssai(snssais);
            if (snssai.empty()) {
                continue;
            }
            routeRule.setSnssai(snssai);
        }
        routeRule.setPduSessionType(urspRule.routeSelectionDescriptors[j].pduSessionType);
        routeRule.setSscMode(urspRule.routeSelectionDescriptors[j].sscMode);
        if (!urspRule.routeSelectionDescriptors[j].dnns.empty()) {
            routeRule.setDnn(urspRule.routeSelectionDescriptors[j].dnns[0]);
        }
        routeRule.setUrspPrecedence((uint8_t) urspRule.urspPrecedence);
        routeBitmap = ConvertInt2UnsignedByte(routeBitmap | 0x01);
        routeRule.setRouteBitmap(routeBitmap);
        return routeRule;
    }
    return routeRule;
}

void UrspConfig::DumpUePolicyMap()
{
    NETMGR_EXT_LOG_I("dump UrspConfig.mUePolicyMap begin");
    for (const auto& entry : mUePolicyMap) {
        NETMGR_EXT_LOG_I("plmn = %{public}s", entry.first.c_str());
        const auto& urspRules = entry.second;
        NETMGR_EXT_LOG_I("dump UrspConfig: urspRules.size = %{public}d", (int)urspRules.size());
        for (size_t i = 0; i < urspRules.size(); ++i) {
            const auto& urspRule = urspRules[i];
            NETMGR_EXT_LOG_I("urspPrecedence = %{public}d", urspRule.urspPrecedence);
            DumptrafficDescriptor(urspRule.trafficDescriptor);
            DumpRouteSelectionDescriptors(urspRule.routeSelectionDescriptors);
        }
    }
    NETMGR_EXT_LOG_I("dump end");
}

void UrspConfig::DumpPreConfigUrspMap()
{
    NETMGR_EXT_LOG_I("dump UrspConfig.mPreConfigUrspMap begin");
    for (const auto& entry : mPreConfigUrspMap) {
        NETMGR_EXT_LOG_I("plmn = %{public}s", entry.first.c_str());
        const auto& urspRules = entry.second;
        NETMGR_EXT_LOG_I("dump UrspConfig: urspRules.size = %{public}d", (int)urspRules.size());
        for (size_t i = 0; i < urspRules.size(); ++i) {
            const auto& urspRule = urspRules[i];
            NETMGR_EXT_LOG_I("urspPrecedence = %{public}d", urspRule.urspPrecedence);
            DumptrafficDescriptor(urspRule.trafficDescriptor);
            DumpRouteSelectionDescriptors(urspRule.routeSelectionDescriptors);
        }
    }
    NETMGR_EXT_LOG_I("dump end");
}

void UrspConfig::DumptrafficDescriptor(const TrafficDescriptor& trafficDescriptor)
{
    NETMGR_EXT_LOG_I("dump trafficDescriptor");
    size_t i;

    NETMGR_EXT_LOG_I("trafficDescriptor.isMatchAll = %{public}s", trafficDescriptor.isMatchAll ? "true":"false");
    for (i = 0; i < trafficDescriptor.osAppIds.size(); ++i) {
        NETMGR_EXT_LOG_I("osAppIds.OsId = %{public}s, osAppIds.AppId = %{public}s",
            trafficDescriptor.osAppIds[i].getOsId().c_str(), trafficDescriptor.osAppIds[i].getAppId().c_str());
    }
    for (i = 0; i < trafficDescriptor.ipv4Addrs.size(); ++i) {
        NETMGR_EXT_LOG_I("ipv4Addrs.mIpv4Addr = %{public}d, ipv4Addrs.mIpv4Mask = %{public}d",
            trafficDescriptor.ipv4Addrs[i].getIpv4Addr(), trafficDescriptor.ipv4Addrs[i].getIpv4Mask());
    }
    for (i = 0; i < trafficDescriptor.ipv6Addrs.size(); ++i) {
        std::string ipv6addr = transIpv6AddrToStr(trafficDescriptor.ipv6Addrs[i].getIpv6Addr());
        NETMGR_EXT_LOG_I("ipv6Addrs.mIpv6Addr = %{public}s, ipv6Addrs.mIpv6Prefix = %{public}d",
            ipv6addr.c_str(), trafficDescriptor.ipv6Addrs[i].getIpv6PrefixLen());
    }
    for (i = 0; i < trafficDescriptor.protocolIds.size(); ++i) {
        NETMGR_EXT_LOG_I("protocolId = %{public}d", trafficDescriptor.protocolIds[i]);
    }
    for (i = 0; i < trafficDescriptor.singleRemotePorts.size(); ++i) {
        NETMGR_EXT_LOG_I("singleRemotePorts = %{public}d", trafficDescriptor.singleRemotePorts[i]);
    }
    for (i = 0; i < trafficDescriptor.remotePortRanges.size(); ++i) {
        NETMGR_EXT_LOG_I("portRangeLowLimit = %{public}d, portRangeHighLimit = %{public}d",
            trafficDescriptor.remotePortRanges[i].getPortRangeLowLimit(),
            trafficDescriptor.remotePortRanges[i].getPortRangeHighLimit());
    }
    for (i = 0; i < trafficDescriptor.dnns.size(); ++i) {
        NETMGR_EXT_LOG_I("trafficDescriptor.dnn = %{public}s", trafficDescriptor.dnns[i].c_str());
    }
    for (i = 0; i < trafficDescriptor.fqdns.size(); ++i) {
        NETMGR_EXT_LOG_I("trafficDescriptor.fqdn = %{public}s", trafficDescriptor.fqdns[i].c_str());
    }
    for (i = 0; i < trafficDescriptor.connectionCapabilities.size(); ++i) {
        NETMGR_EXT_LOG_I("trafficDescriptor.connectionCapabilities = %{public}d",
            trafficDescriptor.connectionCapabilities[i]);
    }
}

void UrspConfig::DumpRouteSelectionDescriptors(const std::vector<RouteSelectionDescriptor>& routeSelectionDescriptors)
{
    NETMGR_EXT_LOG_I("dump routeSelectionDescriptors,size = %{public}d", (int)routeSelectionDescriptors.size());
    size_t i;
    size_t j;
    for (i = 0; i < routeSelectionDescriptors.size(); ++i) {
        NETMGR_EXT_LOG_I("routeSelectionDescriptor.routePrecedence = %{public}d",
            routeSelectionDescriptors[i].routePrecedence);
        NETMGR_EXT_LOG_I("routeSelectionDescriptor.sscMode = %{public}d", routeSelectionDescriptors[i].sscMode);
        NETMGR_EXT_LOG_I("routeSelectionDescriptor.pduSessionType = %{public}d",
            routeSelectionDescriptors[i].pduSessionType);
        for (j = 0; j < routeSelectionDescriptors[i].snssais.size(); ++j) {
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.sNssai.mSNssaiLen = %{public}d",
                routeSelectionDescriptors[i].snssais[j].getSnssaiLen());
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.sNssai.mSst = %{public}d",
                routeSelectionDescriptors[i].snssais[j].getSst());
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.sNssai.mSd = %{public}d",
                routeSelectionDescriptors[i].snssais[j].getSd());
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.sNssai.mMappedSst = %{public}d",
                routeSelectionDescriptors[i].snssais[j].getMappedSst());
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.sNssai.mMappedSd = %{public}d",
                routeSelectionDescriptors[i].snssais[j].getMappedSd());
        }
        for (j = 0; j < routeSelectionDescriptors[i].dnns.size(); ++j) {
            NETMGR_EXT_LOG_I("routeSelectionDescriptor.dnn = %{public}s", routeSelectionDescriptors[i].dnns[j].c_str());
        }
    }
}

} // namespace NetManagerStandard
} // namespace OHOS
