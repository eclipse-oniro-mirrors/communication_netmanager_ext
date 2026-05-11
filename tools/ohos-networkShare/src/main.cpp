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

constexpr int COMMAND_OFFSET = 2;
constexpr int MIN_ARGC_WITH_SUBCOMMAND = 2;
constexpr int ARGC_FOR_TOOL_HELP = 2;

static void RegisterCommand(const Command& cmd)
{
    g_commands[cmd.name] = cmd;
}

int OutputSuccess(const json& data)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"] = data;
    std::cout << response.dump() << std::endl;
    return 0;
}

int OutputError(const std::string& code, const std::string& message, const std::string& suggestion)
{
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["errCode"] = code;
    response["errMsg"] = message;
    response["suggestion"] = suggestion;
    std::cout << response.dump() << std::endl;
    return 1;
}

std::string GetErrorMessage(int32_t errCode)
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

static void ShowToolLevelHelp()
{
    std::cout << "ohos-networkShare - Network sharing management CLI tool\n\n";
    std::cout << "Usage:\n";
    std::cout << "  ohos-networkShare <command> [options]\n\n";
    std::cout << "Parameters:\n";
    std::cout << "  --help              Display this help message\n\n";
    std::cout << "SubCommands:\n";
    std::cout << "  is-supported        Check if network sharing is supported on the device\n";
    std::cout << "  is-sharing          Check if network sharing is currently active\n";
    std::cout << "  start               Start network sharing of specified type\n";
    std::cout << "  stop                Stop network sharing of specified type\n\n";
    std::cout << "Examples:\n";
    std::cout << "  # Check if network sharing is supported\n";
    std::cout << "  ohos-networkShare is-supported\n\n";
    std::cout << "  # Start WiFi hotspot sharing\n";
    std::cout << "  ohos-networkShare start --type wifi\n\n";
    std::cout << "  # Stop USB tethering\n";
    std::cout << "  ohos-networkShare stop --type usb\n\n";
    std::cout << "  # View subcommand help\n";
    std::cout << "  ohos-networkShare start --help\n";
}

static void ShowCommandHelp(const Command& cmd)
{
    std::cout << "ohos-networkShare " << cmd.name << " - "
              << (cmd.description ? cmd.description : "N/A") << "\n\n";

    if (cmd.usage) {
        std::cout << "Usage:\n";
        std::cout << "  " << cmd.usage << "\n\n";
    }

    if (cmd.parameters) {
        std::cout << "Parameters:\n";
        std::cout << cmd.parameters << "\n";
        std::cout << "  --help              Display this help message\n\n";
    }

    if (cmd.examples) {
        std::cout << "Examples:\n";
        std::cout << cmd.examples << "\n";
    }
}

/*
 * Check if network sharing is supported on the device.
 * This function queries the system to determine whether the device has network sharing capability.
 *
 * Returns:
 *   0 on success with JSON output containing "supported" boolean field
 *   1 on failure with error details
 */
int CmdIsSupported(int argc, char** argv)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t supported = NETWORKSHARE_IS_UNSUPPORTED;
    int32_t ret = client->IsSharingSupported(supported);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = GetErrorMessage(ret);
        return OutputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure the device has network sharing capability and proper permissions.");
    }

    json data;
    data["supported"] = (supported == NETWORKSHARE_IS_SUPPORTED);
    return OutputSuccess(data);
}

/*
 * Check if network sharing is currently active.
 * This function queries the current network sharing status to determine if any sharing type
 * (WiFi, USB, or Bluetooth) is currently active on the device.
 *
 * Returns:
 *   0 on success with JSON output containing "sharing" boolean field
 *   1 on failure with error details
 */
int CmdIsSharing(int argc, char** argv)
{
    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t sharingStatus = NETWORKSHARE_IS_UNSHARING;
    int32_t ret = client->IsSharing(sharingStatus);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = GetErrorMessage(ret);
        return OutputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure the device is properly configured for network sharing.");
    }

    json data;
    data["sharing"] = (sharingStatus == NETWORKSHARE_IS_SHARING);
    return OutputSuccess(data);
}

/*
 * Start network sharing for a specified type.
 * This function initiates network sharing for WiFi hotspot, USB tethering, or Bluetooth PAN.
 *
 * Parameters:
 *   --type: Sharing type (wifi, usb, or bluetooth)
 *
 * Permissions Required:
 *   ohos.permission.CONNECTIVITY_INTERNAL
 *
 * System Requirements:
 *   - Must be called by system processes
 *   - EDM policy check for persist.edm.tethering_disallowed
 *
 * Returns:
 *   0 on success with confirmation message
 *   1 on failure with error details
 */
int CmdStart(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    std::string typeStr = GetOption(args, "--type");
    if (typeStr.empty()) {
        return OutputError("ERR_ARG_MISSING", "Missing required parameter: sharing type",
            "Valid types: wifi, usb, bluetooth. Example: ohos-networkShare start --type wifi");
    }

    SharingIfaceType type = parseSharingType(typeStr);
    if (type == SharingIfaceType::SHARING_NONE) {
        return OutputError("ERR_ARG_INVALID", "Invalid sharing type: " + typeStr,
            "Valid types: wifi, usb, bluetooth");
    }

    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t ret = client->StartSharing(type);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = GetErrorMessage(ret);
        return OutputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure proper permissions and that the sharing type is available on this device.");
    }

    json data;
    data["message"] = "Network sharing started successfully";
    data["type"] = typeStr;
    return OutputSuccess(data);
}

/*
 * Stop network sharing for a specified type.
 * This function terminates active network sharing for WiFi hotspot, USB tethering, or Bluetooth PAN.
 *
 * Parameters:
 *   --type: Sharing type (wifi, usb, or bluetooth)
 *
 * Permissions Required:
 *   ohos.permission.CONNECTIVITY_INTERNAL
 *
 * System Requirements:
 *   - Must be called by system processes
 *   - The specified sharing type must be currently active
 *
 * Returns:
 *   0 on success with confirmation message
 *   1 on failure with error details
 */
int CmdStop(int argc, char** argv)
{
    std::vector<std::string> args(argv, argv + argc);
    std::string typeStr = GetOption(args, "--type");
    if (typeStr.empty()) {
        return OutputError("ERR_ARG_MISSING", "Missing required parameter: sharing type",
            "Valid types: wifi, usb, bluetooth. Example: ohos-networkShare stop --type wifi");
    }

    SharingIfaceType type = parseSharingType(typeStr);
    if (type == SharingIfaceType::SHARING_NONE) {
        return OutputError("ERR_ARG_INVALID", "Invalid sharing type: " + typeStr,
            "Valid types: wifi, usb, bluetooth");
    }

    auto client = DelayedSingleton<NetworkShareClient>::GetInstance();
    int32_t ret = client->StopSharing(type);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        std::string errMsg = GetErrorMessage(ret);
        return OutputError("ERR_NET_INTERNAL_ERROR", errMsg,
            "Ensure proper permissions and that the specified sharing type was active.");
    }

    json data;
    data["message"] = "Network sharing stopped successfully";
    data["type"] = typeStr;
    return OutputSuccess(data);
}

void InitCommands()
{
    RegisterCommand({"is-supported", "Check if network sharing is supported on the device",
        "ohos-networkShare is-supported",
        "    None",
        "    ohos-networkShare is-supported",
        CmdIsSupported});

    RegisterCommand({"is-sharing", "Check if network sharing is currently active",
        "ohos-networkShare is-sharing",
        "    None",
        "    ohos-networkShare is-sharing",
        CmdIsSharing});

    RegisterCommand({"start", "Start network sharing",
        "ohos-networkShare start --type <wifi|usb|bluetooth>",
        "    --type           Required string. Sharing type.\n"
        "                     Valid values: wifi, usb, bluetooth",
        "    ohos-networkShare start --type wifi\n"
        "    ohos-networkShare start --type usb\n"
        "    ohos-networkShare start --type bluetooth",
        CmdStart});

    RegisterCommand({"stop", "Stop network sharing",
        "ohos-networkShare stop --type <wifi|usb|bluetooth>",
        "    --type           Required string. Sharing type.\n"
        "                     Valid values: wifi, usb, bluetooth",
        "    ohos-networkShare stop --type wifi\n"
        "    ohos-networkShare stop --type usb\n"
        "    ohos-networkShare stop --type bluetooth",
        CmdStop});
}

void PrintUsage(const char* prog)
{
    OutputError("ERR_ARG_MISSING",
        std::string("Missing required subcommand"),
        std::string("Run '") + prog + " --help' for more information");
}

static bool HasHelpFlag(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            return true;
        }
    }
    return false;
}

static int HandleToolLevelHelp()
{
    ShowToolLevelHelp();
    return 0;
}

static int HandleCommandHelp(const std::string& cmdName)
{
    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        return OutputError("ERR_ARG_INVALID", "Unknown command: " + cmdName,
            "Run 'ohos-networkShare --help' to list available commands.");
    }
    ShowCommandHelp(it->second);
    return 0;
}

} // namespace NetManagerStandard
} // namespace OHOS

int main(int argc, char** argv)
{
    using namespace OHOS::NetManagerStandard;

    InitCommands();

    // Handle tool-level --help: ohos-networkShare --help
    if (argc == ARGC_FOR_TOOL_HELP && strcmp(argv[1], "--help") == 0) {
        return HandleToolLevelHelp();
    }

    // Require at least a subcommand
    if (argc < MIN_ARGC_WITH_SUBCOMMAND) {
        PrintUsage(argv[0]);
        return 1;
    }

    std::string cmdName = argv[1];

    // Handle subcommand --help: ohos-networkShare <subcommand> --help
    if (HasHelpFlag(argc, argv)) {
        return HandleCommandHelp(cmdName);
    }

    // Execute the subcommand
    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        return OutputError("ERR_ARG_INVALID", "Unknown command: " + cmdName,
            "Run 'ohos-networkShare --help' to list available commands.");
    }

    return it->second.handler(argc - COMMAND_OFFSET, argv + COMMAND_OFFSET);
}