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

  export type HttpProxy = connection.HttpProxy;

  export class HttpProxyInner implements HttpProxy {
    host: string = "";
    port: int = 0;
    exclusionList: Array<string> = [];
    username?: string;
    password?: string;
  }

  native function isEthernetEnabledNative(): boolean;

  export function isEthernetEnabled(): boolean {
    return isEthernetEnabledNative();
  }

  native function disableEthernetInterfaceNative(): void;

  export function disableEthernetInterface(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return disableEthernetInterfaceNative();
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function enableEthernetInterfaceNative(): void;

  export function enableEthernetInterface(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return enableEthernetInterfaceNative();
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function getAllActiveIfacesNative(): Array<string>;

  export function getAllActiveIfaces(callback: AsyncCallback<Array<string>>): void {
    let p1 = taskpool.execute((): Array<string> => {
      return getAllActiveIfacesNative();
    })
    p1.then((content: Any) => {
      callback(null, content as Array<string>);
    }).catch((err: Error) => {
      callback(err as BusinessError, new Array<string>());
    });
  }

  export function getAllActiveIfaces(): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
        return getAllActiveIfacesNative();
      }).then((content: Any) => {
        resolve(content as Array<string>);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function isIfaceActiveNative(iface: string): int;

  export function isIfaceActive(iface: string, callback: AsyncCallback<int>): void {
    let p1 = taskpool.execute((): int => {
      return isIfaceActiveNative(iface);
    })
    p1.then((content: Any) => {
      callback(null, content as int);
    }).catch((err: Error) => {
      callback(err as BusinessError, 0);
    });
  }

  export function isIfaceActive(iface: string): Promise<int> {
    return new Promise<int>((resolve, reject) => {
      taskpool.execute((): int => {
        return isIfaceActiveNative(iface);
      }).then((content: Any) => {
        resolve(content as int);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function getIfaceConfigNative(iface: string): InterfaceConfiguration;

  export function getIfaceConfig(iface: string, callback: AsyncCallback<InterfaceConfiguration>): void {
    let p1 = taskpool.execute((): InterfaceConfiguration => {
      return getIfaceConfigNative(iface);
    })
    p1.then((content: Any) => {
      callback(null, content as InterfaceConfiguration);
    }).catch((err: Error) => {
      callback(err as BusinessError, undefined);
    });
  }

  export function getIfaceConfig(iface: string): Promise<InterfaceConfiguration> {
    return new Promise<InterfaceConfiguration>((resolve, reject) => {
      taskpool.execute((): InterfaceConfiguration => {
        return getIfaceConfigNative(iface);
      }).then((content: Any) => {
        resolve(content as InterfaceConfiguration);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function setIfaceConfigNative(iface: string, ic: InterfaceConfiguration): void;

  export function setIfaceConfig(iface: string, ic: InterfaceConfiguration, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): void => {
      return setIfaceConfigNative(iface, ic);
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(err as BusinessError, undefined);
    });
  }

  export function setIfaceConfig(iface: string, ic: InterfaceConfiguration): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setIfaceConfigNative(iface, ic);
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function getEthernetDeviceInfosNative(): Array<EthernetDeviceInfos>;

  export function getEthernetDeviceInfos(): Promise<Array<EthernetDeviceInfos>> {
    return new Promise<Array<EthernetDeviceInfos>>((resolve, reject) => {
      taskpool.execute((): Array<EthernetDeviceInfos> => {
        return getEthernetDeviceInfosNative();
      }).then((content: Any) => {
        resolve(content as Array<EthernetDeviceInfos>);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function getMacAddressNative(): Array<MacAddressInfo>;

  export function getMacAddress(): Promise<Array<MacAddressInfo>> {
    return new Promise<Array<MacAddressInfo>>((resolve, reject) => {
      taskpool.execute((): Array<MacAddressInfo> => {
        return getMacAddressNative();
      }).then((content: Any) => {
        resolve(content as Array<MacAddressInfo>);
      }).catch((err: Error) => {
        reject(err as BusinessError);
      });
    });
  }

  native function onInterfaceStateChangeNative(callback: Callback<InterfaceStateInfo>): void;

  export function onInterfaceStateChange(callback: Callback<InterfaceStateInfo>): void {
    onInterfaceStateChangeNative(callback);
  }

  native function offInterfaceStateChangeNative(callback?: Callback<InterfaceStateInfo>): void;

  export function offInterfaceStateChange(callback?: Callback<InterfaceStateInfo>): void {
    if (callback == undefined) {
      offInterfaceStateChangeNative();
    } else {
      offInterfaceStateChangeNative(callback);
    }
  }

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

  export interface InterfaceConfiguration {
    mode: IPSetMode;
    ipAddr: string;
    route: string;
    gateway: string;
    netMask: string;
    dnsServers: string;
    httpProxy?: HttpProxy;
  }

  export class InterfaceConfigurationInner implements InterfaceConfiguration {
    mode: IPSetMode;
    ipAddr: string;
    route: string;
    gateway: string;
    netMask: string;
    dnsServers: string;
    httpProxy?: HttpProxy;
  }

  export interface InterfaceStateInfo {
    iface: string;
    active: boolean;
  }

  export class InterfaceStateInfoInner implements InterfaceStateInfo {
    iface: string;
    active: boolean;
  }

  export interface MacAddressInfo {
    iface: string;
    macAddress: string;
  }

  export class MacAddressInfoInner implements MacAddressInfo {
    iface: string;
    macAddress: string;
  }

  export interface EthernetDeviceInfos {
    ifaceName: string;
    deviceName: string;
    connectionMode: DeviceConnectionType;
    supplierName: string;
    supplierId: string;
    productName: string;
    maximumRate: string;
  }

  export class EthernetDeviceInfosInner implements EthernetDeviceInfos {
    ifaceName: string;
    deviceName: string;
    connectionMode: DeviceConnectionType;
    supplierName: string;
    supplierId: string;
    productName: string;
    maximumRate: string;
  }
}
