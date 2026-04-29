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

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstring>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "networkshare_client.h"
#include "networkshare_constants.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {

constexpr int32_t NETMANAGER_EXT_SUCCESS = 0;
constexpr int32_t NETMANAGER_EXT_ERR_PERMISSION_DENIED = -1;
constexpr int32_t NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL = -2;
constexpr int32_t NETMANAGER_EXT_ERR_INVALID_PARAMETER = -3;
constexpr int32_t NETMANAGER_EXT_ERR_INTERNAL_ERROR = -4;

typedef std::function<int(int, char**)> CommandHandler;

struct Command {
    const char* name;
    const char* description;
    const char* usage;
    const char* parameters;
    const char* examples;
    CommandHandler handler;
};

static std::unordered_map<std::string, Command> g_commands;
static const char* g_program_name = "ohos-networkShare";

#define REGISTER_CMD(name, desc, usage, params, examples, handler) \
    g_commands[name] = {name, desc, usage, params, examples, handler}

int outputSuccess(const json& data)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"] = data;
    std::cout << response.dump() << std::endl;
    return 0;
}

int outputError(const std::string& code, const std::string& message, const std::string& suggestion = "")
{
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["data"] = "";
    response["errCode"] = code;
    response["errMsg"] = message;
    if (!suggestion.empty()) {
        response["suggestion"] = suggestion;
    }
    std::cout << response.dump() << std::endl;
    return 1;
}

std::string getErrorMessage(int32_t errCode)
{
    switch (errCode) {
        case NETMANAGER_EXT_SUCCESS:
            return "Success";
        case NETMANAGER_EXT_ERR_PERMISSION_DENIED:
            return "Permission denied. Requires ohos.permission.CONNECTIVITY_INTERNAL";
        case NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL:
            return "System caller only. This tool must be run by system processes.";
        case NETMANAGER_EXT_ERR_INVALID_PARAMETER:
            return "Invalid parameter";
        case NETMANAGER_EXT_ERR_INTERNAL_ERROR:
            return "Internal error";
        default:
            return "Unknown error: " + std::to_string(errCode);
    }
}

SharingIfaceType parseSharingType(const std::string& typeStr)
{
    if (typeStr == "wifi") {
        return SharingIfaceType::SHARING_WIFI;
    } else if (typeStr == "usb") {
        return SharingIfaceType::SHARING_USB;
    } else if (typeStr == "bluetooth") {
        return SharingIfaceType::SHARING_BLUETOOTH;
    }
    return SharingIfaceType::SHARING_NONE;
}

std::string GetOption(const std::vector<std::string>& args, const std::string& option)
{
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == option && i + 1 < args.size()) {
            return args[i + 1];
        }
    }
    return "";
}

static bool ParseHelpArgs(int argc, char** argv, bool& jsonFormat, std::string& targetCmd)
{
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--format") == 0 && i + 1 < argc && strcmp(argv[i + 1], "json") == 0) {
            jsonFormat = true;
        } else if (argv[i][0] != '-') {
            targetCmd = argv[i];
        }
    }
    return true;
}

static int ShowHelpListJson()
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    json data;
    data["command"] = g_program_name;
    json cmds = json::array();
    for (const auto& pair : g_commands) {
        json cmd;
        cmd["name"] = pair.first;
        cmd["description"] = pair.second.description ? pair.second.description : "";
        cmds.push_back(cmd);
    }
    data["commands"] = cmds;
    response["data"] = data;
    std::cout << response.dump(2) << std::endl;
    return 0;
}

static void ShowHelpListText()
{
    std::cerr << g_program_name << " <command> [args]\n";
    std::cerr << "\nCommands:\n";
    for (const auto& pair : g_commands) {
        std::cerr << "    " << pair.first << "    "
                  << (pair.second.description ? pair.second.description : "") << "\n";
    }
    std::cerr << "\nRun '" << g_program_name << " <command> --help' for more information.\n";
}

static int ShowCommandHelpJson(const Command& cmd)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    json data;
    data["command"] = cmd.name;
    data["description"] = cmd.description ? cmd.description : "";
    data["usage"] = cmd.usage ? cmd.usage : "";
    data["parameters"] = cmd.parameters ? cmd.parameters : "";
    data["examples"] = cmd.examples ? cmd.examples : "";
    response["data"] = data;
    std::cout << response.dump(2) << std::endl;
    return 0;
}

static void ShowCommandHelpText(const Command& cmd)
{
    std::cerr << "Command: " << cmd.name << "\n";
    std::cerr << "Description: " << (cmd.description ? cmd.description : "N/A") << "\n";
    if (cmd.usage) {
        std::cerr << "\nUsage:\n";
        std::cerr << "    " << cmd.usage << "\n";
    }
    if (cmd.parameters) {
        std::cerr << "\nParameters:\n";
        std::cerr << cmd.parameters << "\n";
    }
    if (cmd.examples) {
        std::cerr << "\nExamples:\n";
        std::cerr << cmd.examples << "\n";
    }
}

int cmdHelp(int argc, char** argv)
{
    bool jsonFormat = false;
    std::string targetCmd;
    ParseHelpArgs(argc, argv, jsonFormat, targetCmd);

    if (targetCmd.empty()) {
        if (jsonFormat) {
            return ShowHelpListJson();
        } else {
            ShowHelpListText();
            return 0;
        }
    }

    auto it = g_commands.find(targetCmd);
    if (it == g_commands.end()) {
        return outputError("ERR_NET_ARG_INVALID",
            "Unknown command: " + targetCmd,
            "Run '" + std::string(g_program_name) + " help' to list available commands.");
    }

    const Command& cmd = it->second;
    if (jsonFormat) {
        return ShowCommandHelpJson(cmd);
    }

    ShowCommandHelpText(cmd);
    return 0;
}

int cmdIsSupported(int argc, char** argv)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t supported = NETWORKSHARE_IS_UNSUPPORTED;
    int32_t ret = client->IsSharingSupported(supported);

    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = getErrorMessage(ret);
        return outputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure the device has network sharing capability and proper permissions.");
    }

    json data;
    data["supported"] = (supported == NETWORKSHARE_IS_SUPPORTED);
    return outputSuccess(data);
}

int cmdIsSharing(int argc, char** argv)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t sharingStatus = NETWORKSHARE_IS_UNSHARING;
    int32_t ret = client->IsSharing(sharingStatus);

    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = getErrorMessage(ret);
        return outputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure the device is properly configured for network sharing.");
    }

    json data;
    data["sharing"] = (sharingStatus == NETWORKSHARE_IS_SHARING);
    return outputSuccess(data);
}

int cmdStart(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    std::string typeStr = GetOption(args, "--type");

    if (typeStr.empty()) {
        return outputError("ERR_ARG_MISSING", "Missing required parameter: sharing type",
            "Valid types: wifi, usb, bluetooth. Example: ohos-networkShare start wifi");
    }

    SharingIfaceType type = parseSharingType(typeStr);
    if (type == SharingIfaceType::SHARING_NONE) {
        return outputError("ERR_ARG_INVALID", "Invalid sharing type: " + typeStr,
            "Valid types: wifi, usb, bluetooth");
    }

    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t ret = client->StartSharing(type);

    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = getErrorMessage(ret);
        return outputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure proper permissions and that the sharing type is available on this device.");
    }

    json data;
    data["message"] = "Network sharing started successfully";
    data["type"] = typeStr;
    return outputSuccess(data);
}

int cmdStop(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    std::string typeStr = GetOption(args, "--type");

    if (typeStr.empty()) {
        return outputError("ERR_ARG_MISSING", "Missing required parameter: sharing type",
            "Valid types: wifi, usb, bluetooth. Example: ohos-networkShare stop wifi");
    }

    SharingIfaceType type = parseSharingType(typeStr);
    if (type == SharingIfaceType::SHARING_NONE) {
        return outputError("ERR_ARG_INVALID", "Invalid sharing type: " + typeStr,
            "Valid types: wifi, usb, bluetooth");
    }

    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t ret = client->StopSharing(type);

    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = getErrorMessage(ret);
        return outputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure proper permissions and that the specified sharing type was active.");
    }

    json data;
    data["message"] = "Network sharing stopped successfully";
    data["type"] = typeStr;
    return outputSuccess(data);
}

void initCommands()
{
    REGISTER_CMD("help", "Show this help message",
        "ohos-networkShare help [command] [--format json]",
        "    command          Optional string. Show detailed help for a specific command.\n"
        "    --format json    Optional flag. Output in JSON format.",
        "    ohos-networkShare help\n"
        "    ohos-networkShare help start --format json",
        cmdHelp);

    REGISTER_CMD("is-supported", "Check if network sharing is supported on the device",
        "ohos-networkShare is-supported",
        "    None",
        "    ohos-networkShare is-supported",
        cmdIsSupported);

    REGISTER_CMD("is-sharing", "Check if network sharing is currently active",
        "ohos-networkShare is-sharing",
        "    None",
        "    ohos-networkShare is-sharing",
        cmdIsSharing);

    REGISTER_CMD("start", "Start network sharing",
        "ohos-networkShare start <type>",
        "    <type>           Required string. Sharing type.\n"
        "                     Valid values: wifi, usb, bluetooth",
        "    ohos-networkShare start wifi\n"
        "    ohos-networkShare start usb\n"
        "    ohos-networkShare start bluetooth",
        cmdStart);

    REGISTER_CMD("stop", "Stop network sharing",
        "ohos-networkShare stop <type>",
        "    <type>           Required string. Sharing type.\n"
        "                     Valid values: wifi, usb, bluetooth",
        "    ohos-networkShare stop wifi\n"
        "    ohos-networkShare stop usb\n"
        "    ohos-networkShare stop bluetooth",
        cmdStop);
}

void printUsage(const char* prog)
{
    std::cerr << "[ERROR] Usage: " << prog << " <command> [options...]\n";
    std::cerr << "[ERROR] Run '" << prog << " help' for more information\n";
}

static bool HasGlobalHelpArg(int argc, char** argv, int& helpIndex)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            helpIndex = i;
            return true;
        }
    }
    return false;
}

static int BuildHelpArgv(char** argv, int argc, int helpIndex)
{
    char* helpArgv[32] = {argv[0], const_cast<char*>("help")};
    int helpArgc = 2;
    for (int j = 1; j < argc; j++) {
        if (j != helpIndex) {
            helpArgv[helpArgc++] = argv[j];
        }
    }
    return cmdHelp(helpArgc, helpArgv);
}

static int CheckGlobalHelp(int argc, char** argv)
{
    int helpIndex = 0;
    if (HasGlobalHelpArg(argc, argv, helpIndex)) {
        return BuildHelpArgv(argv, argc, helpIndex);
    }
    return 0;
}

static int CheckCommandHelp(int argc, char* argv[], const std::string& cmdName)
{
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            char* helpArgv[32] = {argv[0], const_cast<char*>("help"),
                                  const_cast<char*>(cmdName.c_str())};
            cmdHelp(3, helpArgv);
            return 1;
        }
    }
    return 0;
}

} // namespace NetManagerStandard
} // namespace OHOS

int main(int argc, char** argv)
{
    using namespace OHOS::NetManagerStandard;

    if (CheckGlobalHelp(argc, argv) != 0) {
        return 0;
    }

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    initCommands();

    std::string cmdName = argv[1];

    if (CheckCommandHelp(argc, argv, cmdName) != 0) {
        return 0;
    }

    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        std::cerr << "[ERROR] Unknown command: " << cmdName << "\n";
        printUsage(argv[0]);
        return 1;
    }

    return it->second.handler(argc - 2, argv + 2);
}