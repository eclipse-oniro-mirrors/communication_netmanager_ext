# ohos-networkShare

## Overview

Network sharing management CLI tool for OpenHarmony. This tool provides commands to check network sharing support status, query current sharing state, and control network sharing (start/stop) for different sharing types including WiFi, USB, and Bluetooth.

## Features

- Check if network sharing is supported on the device
- Check current network sharing status
- Start network sharing for WiFi, USB, or Bluetooth
- Stop network sharing for WiFi, USB, or Bluetooth

## Dependencies

### System Capability
- `SystemCapability.Communication.NetManager.NetSharing`

### Libraries
- `net_tether_manager_if` (Network sharing client library)

### Permissions
- `ohos.permission.CONNECTIVITY_INTERNAL` (System permission required for all operations)

### System Requirements
- Must be executed by system processes (system caller check)
- EDM policy check for `persist.edm.tethering_disallowed` (for start command)

## Basic Usage

```bash
ohos-networkShare <command> [args]
```

## Command List

| Command | Description | Arguments | Permission | Prerequisites |
|---------|-------------|-----------|------------|---------------|
| help | Show help information | [command] [--format json] | None | None |
| is-supported | Check if network sharing is supported | None | ohos.permission.CONNECTIVITY_INTERNAL | None |
| is-sharing | Check if network sharing is currently active | None | ohos.permission.CONNECTIVITY_INTERNAL | None |
| start | Start network sharing | --type <wifi\|usb\|bluetooth> | ohos.permission.CONNECTIVITY_INTERNAL | None |
| stop | Stop network sharing | --type <wifi\|usb\|bluetooth> | ohos.permission.CONNECTIVITY_INTERNAL | None |

**Prerequisites Description**:
- **None**: The command can be executed directly without any prerequisites.

## Examples

```bash
# Show help information
ohos-networkShare help

# Show help for a specific command in JSON format
ohos-networkShare help start --format json

# Check if network sharing is supported
ohos-networkShare is-supported

# Check if network sharing is currently active
ohos-networkShare is-sharing

# Start WiFi network sharing
ohos-networkShare start --type wifi

# Start USB network sharing
ohos-networkShare start --type usb

# Start Bluetooth network sharing
ohos-networkShare start --type bluetooth

# Stop WiFi network sharing
ohos-networkShare stop --type wifi

# Stop USB network sharing
ohos-networkShare stop --type usb

# Stop Bluetooth network sharing
ohos-networkShare stop --type bluetooth
```

## Output Format

All commands output a JSON response to stdout with the following structure:

```json
{
  "type": "result",
  "status": "success",
  "data": { ... }
}
```

For errors:
```json
{
  "type": "result",
  "status": "failed",
  "errCode": "ERR_XXX",
  "errMsg": "Error description",
  "suggestion": "Suggested next steps"
}
```

## Error Codes

| Error Code | Description | Suggestion |
|------------|-------------|------------|
| ERR_OPERATION_FAILED | Operation failed due to permission or internal error | Ensure proper permissions and system configuration |
| ERR_MISSING_PARAM | Missing required parameter | Provide required parameters as shown in help |
| ERR_INVALID_PARAM | Invalid parameter value | Use valid parameter values: wifi, usb, bluetooth |
| INVALID_COMMAND | Unknown command | Run help to list available commands |

## Installation

The tool is installed to `/system/bin/cli_tool/executable/ohos-networkShare` and is included in the system image when building with `netmanager_ext` component.

## Source Code

- `src/main.cpp` - Main entry point with command routing
- `BUILD.gn` - Build configuration
- `config.json` - Tool metadata and schema definitions

## License

Apache License 2.0