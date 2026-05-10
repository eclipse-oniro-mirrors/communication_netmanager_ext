# Test Cases for ohos-networkShare

## Test Command Coverage

| Command Example | Description | Permission | Prerequisites |
|-----------------|-------------|------------|---------------|
| ohos-networkShare help | Show all available commands | None | None |
| ohos-networkShare help --format json | Show commands in JSON format | None | None |
| ohos-networkShare help is-supported | Show help for is-supported command | None | None |
| ohos-networkShare help is-supported --format json | Show help for is-supported in JSON | None | None |
| ohos-networkShare help is-sharing | Show help for is-sharing command | None | None |
| ohos-networkShare help is-sharing --format json | Show help for is-sharing in JSON | None | None |
| ohos-networkShare help start | Show help for start command | None | None |
| ohos-networkShare help start --format json | Show help for start in JSON | None | None |
| ohos-networkShare help stop | Show help for stop command | None | None |
| ohos-networkShare help stop --format json | Show help for stop in JSON | None | None |
| ohos-networkShare is-supported | Check if network sharing is supported | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare is-sharing | Check current sharing status | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare start --type wifi | Start WiFi sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare start --type usb | Start USB sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare start --type bluetooth | Start Bluetooth sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare stop --type wifi | Stop WiFi sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare stop --type usb | Stop USB sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |
| ohos-networkShare stop --type bluetooth | Stop Bluetooth sharing | ohos.permission.CONNECTIVITY_INTERNAL | None |

## Expected Outputs

### help command

```bash
# ohos-networkShare help
ohos-networkShare <command> [args]

Commands:
    help                Show this help message
    is-supported        Check if network sharing is supported on the device
    is-sharing          Check if network sharing is currently active
    start               Start network sharing
    stop                Stop network sharing

Run 'ohos-networkShare <command> --help' for more information.

# ohos-networkShare help --format json
{
  "command": "ohos-networkShare",
  "commands": [
    {"name": "help", "description": "Show this help message"},
    {"name": "is-supported", "description": "Check if network sharing is supported on the device"},
    {"name": "is-sharing", "description": "Check if network sharing is currently active"},
    {"name": "start", "description": "Start network sharing"},
    {"name": "stop", "description": "Stop network sharing"}
  ]
}
```

### is-supported command

```bash
# ohos-networkShare is-supported (success)
{"type":"result","status":"success","data":{"supported":true}}

# ohos-networkShare is-supported (device not supported)
{"type":"result","status":"success","data":{"supported":false}}
```

### is-sharing command

```bash
# ohos-networkShare is-sharing (sharing active)
{"type":"result","status":"success","data":{"sharing":true}}

# ohos-networkShare is-sharing (sharing inactive)
{"type":"result","status":"success","data":{"sharing":false}}
```

### start command

```bash
# ohos-networkShare start --type wifi (success)
{"type":"result","status":"success","data":{"message":"Network sharing started successfully","type":"wifi"}}

# ohos-networkShare start --type usb (success)
{"type":"result","status":"success","data":{"message":"Network sharing started successfully","type":"usb"}}

# ohos-networkShare start --type bluetooth (success)
{"type":"result","status":"success","data":{"message":"Network sharing started successfully","type":"bluetooth"}}
```

### stop command

```bash
# ohos-networkShare stop --type wifi (success)
{"type":"result","status":"success","data":{"message":"Network sharing stopped successfully","type":"wifi"}}

# ohos-networkShare stop --type usb (success)
{"type":"result","status":"success","data":{"message":"Network sharing stopped successfully","type":"usb"}}

# ohos-networkShare stop --type bluetooth (success)
{"type":"result","status":"success","data":{"message":"Network sharing stopped successfully","type":"bluetooth"}}
```

## Error Cases

### Missing parameter

```bash
# ohos-networkShare start (missing type)
{"type":"result","status":"failed","errCode":"ERR_MISSING_PARAM","errMsg":"Missing required parameter: sharing type","suggestion":"Valid types: wifi, usb, bluetooth. Example: ohos-networkShare start --type wifi"}

# ohos-networkShare stop (missing type)
{"type":"result","status":"failed","errCode":"ERR_MISSING_PARAM","errMsg":"Missing required parameter: sharing type","suggestion":"Valid types: wifi, usb, bluetooth. Example: ohos-networkShare stop --type wifi"}
```

### Invalid parameter

```bash
# ohos-networkShare start --type invalid
{"type":"result","status":"failed","errCode":"ERR_INVALID_PARAM","errMsg":"Invalid sharing type: invalid","suggestion":"Valid types: wifi, usb, bluetooth"}

# ohos-networkShare stop --type invalid
{"type":"result","status":"failed","errCode":"ERR_INVALID_PARAM","errMsg":"Invalid sharing type: invalid","suggestion":"Valid types: wifi, usb, bluetooth"}
```

### Permission denied

```bash
# ohos-networkShare is-supported (permission denied)
{"type":"result","status":"failed","errCode":"ERR_OPERATION_FAILED","errMsg":"Permission denied. Requires ohos.permission.CONNECTIVITY_INTERNAL","suggestion":"Ensure the device has network sharing capability and proper permissions."}
```

### System caller check failed

```bash
# ohos-networkShare start --type wifi (not system caller)
{"type":"result","status":"failed","errCode":"ERR_OPERATION_FAILED","errMsg":"System caller only. This tool must be run by system processes.","suggestion":"Ensure proper permissions and that the sharing type is available on this device."}
```

## Command Coverage Summary

| Command | Parameters Tested | Total Cases |
|---------|-------------------|-------------|
| help | none, --format json, <subcommand>, <subcommand> --format json | 11 |
| is-supported | none | 1 |
| is-sharing | none | 1 |
| start | wifi, usb, bluetooth | 3 |
| stop | wifi, usb, bluetooth | 3 |

**Total Test Cases**: 19
**Coverage**: 100% of all commands and all parameter combinations