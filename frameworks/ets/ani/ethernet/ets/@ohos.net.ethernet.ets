/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the language governing permissions and
 * limitations under the License.
 */

import type { AsyncCallback, Callback } from '@ohos.base';
import { BusinessError } from '@ohos.base';
import type connection from '@ohos.net.connection';

export default namespace ethernet {
  loadLibrary("ethernet_ani");

  // NOTE: ArkTS forbids 'any' type annotations (ESE153545), so err must remain
  // typed as Error. In practice, taskpool.execute wraps all throwables in Error
  // instances, so non-Error throws are not expected at runtime in the ArkTS env.
  function toBusinessError(err: Error): BusinessError {
    let businessErr = new BusinessError();
    businessErr.message = err.message;
    businessErr.stack = err.stack ?? '';
    businessErr.cause = err;
    if (err instanceof BusinessError) {
      businessErr.code = err.code;
    }
    return businessErr;
  }

  // ===== HttpProxy =====

  export type HttpProxy = connection.HttpProxy;

  export class HttpProxyInner implements HttpProxy {
    host: string = "";
    port: int = 0;
    exclusionList: Array<string> = [];
    username?: string;
    password?: string;
  }

  // ===== isEthernetEnabled =====

  native function isEthernetEnabledSyncNative(): boolean;

  export function isEthernetEnabled(): boolean {
    return isEthernetEnabledSyncNative();
  }

  // ===== disableEthernetInterface =====

  native function disableEthernetInterfaceSyncNative(): void;

  export function disableEthernetInterface(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return disableEthernetInterfaceSync();
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export function disableEthernetInterfaceSync(): void {
    disableEthernetInterfaceSyncNative();
  }

  // ===== enableEthernetInterface =====

  native function enableEthernetInterfaceSyncNative(): void;

  export function enableEthernetInterface(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return enableEthernetInterfaceSync();
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export function enableEthernetInterfaceSync(): void {
    enableEthernetInterfaceSyncNative();
  }

  // ===== getAllActiveIfaces =====

  native function getAllActiveIfacesSyncNative(): Array<string>;

  export function getAllActiveIfacesSync(): Array<string> {
    return getAllActiveIfacesSyncNative();
  }

  export function getAllActiveIfaces(callback: AsyncCallback<Array<string>>): void {
    let p1 = taskpool.execute((): Array<string> => {
      return getAllActiveIfacesSync();
    })
    p1.then((content: Any) => {
      callback(null, content as Array<string>);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getAllActiveIfaces(): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
        return getAllActiveIfacesSync();
      }).then((content: Any) => {
        resolve(content as Array<string>);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== isIfaceActive =====

  native function isIfaceActiveSyncNative(iface: string): int;

  export function isIfaceActiveSync(iface: string): int {
    return isIfaceActiveSyncNative(iface);
  }

  export function isIfaceActive(iface: string, callback: AsyncCallback<int>): void {
    let p1 = taskpool.execute((): int => {
      return isIfaceActiveSync(iface);
    })
    p1.then((content: Any) => {
      callback(null, content as int);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function isIfaceActive(iface: string): Promise<int> {
    return new Promise<int>((resolve, reject) => {
      taskpool.execute((): int => {
        return isIfaceActiveSync(iface);
      }).then((content: Any) => {
        resolve(content as int);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== getIfaceConfig =====

  native function getIfaceConfigSyncNative(iface: string): InterfaceConfiguration;

  export function getIfaceConfigSync(iface: string): InterfaceConfiguration {
    return getIfaceConfigSyncNative(iface);
  }

  export function getIfaceConfig(iface: string, callback: AsyncCallback<InterfaceConfiguration>): void {
    let p1 = taskpool.execute((): InterfaceConfiguration => {
      return getIfaceConfigSync(iface);
    })
    p1.then((content: Any) => {
      callback(null, content as InterfaceConfiguration);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getIfaceConfig(iface: string): Promise<InterfaceConfiguration> {
    return new Promise<InterfaceConfiguration>((resolve, reject) => {
      taskpool.execute((): InterfaceConfiguration => {
        return getIfaceConfigSync(iface);
      }).then((content: Any) => {
        resolve(content as InterfaceConfiguration);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== setIfaceConfig =====

  native function setIfaceConfigSyncNative(iface: string, ic: InterfaceConfiguration): void;

  export function setIfaceConfigSync(iface: string, ic: InterfaceConfiguration): void {
    setIfaceConfigSyncNative(iface, ic);
  }

  export function setIfaceConfig(iface: string, ic: InterfaceConfiguration, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setIfaceConfigSync(iface, ic);
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function setIfaceConfig(iface: string, ic: InterfaceConfiguration): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setIfaceConfigSync(iface, ic);
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== getEthernetDeviceInfos =====

  native function getEthernetDeviceInfosSyncNative(): Array<EthernetDeviceInfos>;

  export function getEthernetDeviceInfosSync(): Array<EthernetDeviceInfos> {
    return getEthernetDeviceInfosSyncNative();
  }

  export function getEthernetDeviceInfos(): Promise<Array<EthernetDeviceInfos>> {
    return new Promise<Array<EthernetDeviceInfos>>((resolve, reject) => {
      taskpool.execute((): Array<EthernetDeviceInfos> => {
        return getEthernetDeviceInfosSync();
      }).then((content: Any) => {
        resolve(content as Array<EthernetDeviceInfos>);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== getMacAddress =====

  native function getMacAddressSyncNative(): Array<MacAddressInfo>;

  export function getMacAddressSync(): Array<MacAddressInfo> {
    return getMacAddressSyncNative();
  }

  export function getMacAddress(): Promise<Array<MacAddressInfo>> {
    return new Promise<Array<MacAddressInfo>>((resolve, reject) => {
      taskpool.execute((): Array<MacAddressInfo> => {
        return getMacAddressSync();
      }).then((content: Any) => {
        resolve(content as Array<MacAddressInfo>);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  // ===== on / off interfaceStateChange =====

  export function on(type: 'interfaceStateChange', callback: (err: BusinessError | null, data: InterfaceStateInfo) => void): void {
    onInterfaceStateChangeNative((err: BusinessError, data: InterfaceStateInfo): void => {
      if (err) {
        console.error(`Interface state change callback error: code=${err.code}, message=${err.message}`);
        callback(err, data);
        return;
      }
      callback(null, data);
    });
  }

  native function onInterfaceStateChangeNative(callback: (err: BusinessError, data: InterfaceStateInfo) => void): void;

  native function offInterfaceStateChangeNative(callback?: Callback<InterfaceStateInfo>): void;

  export function off(type: 'interfaceStateChange', callback?: Callback<InterfaceStateInfo>): void {
    offInterfaceStateChangeNative(callback);
  }

  // ===== Enums =====

  export enum IPSetMode {
    STATIC = 0,
    DHCP = 1,
    LAN_STATIC = 2,
    LAN_DHCP = 3
  }

  export enum DeviceConnectionType {
    BUILT_IN = 0,
    EXTERNAL = 1
  }

  // ===== Classes =====

  export class InterfaceConfiguration {
    mode: IPSetMode = IPSetMode.STATIC;
    ipAddr: string = "";
    route: string = "";
    gateway: string = "";
    netMask: string = "";
    dnsServers: string = "";
    httpProxy?: HttpProxy;
  }

  export class InterfaceStateInfo {
    iface: string = "";
    active: boolean = false;
  }

  export class MacAddressInfo {
    iface: string = "";
    macAddress: string = "";
  }

  export class EthernetDeviceInfos {
    ifaceName: string = "";
    deviceName: string = "";
    connectionMode: DeviceConnectionType = DeviceConnectionType.BUILT_IN;
    supplierName: string = "";
    supplierId: string = "";
    productName: string = "";
    maximumRate: string = "";
  }
}
