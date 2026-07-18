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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import type { AsyncCallback, Callback } from '@ohos.base';
import type connection from '@ohos.net.connection';
import type VpnExtensionContext from 'application.VpnExtensionContext';
import type Want from '@ohos.app.ability.Want';
import { BusinessError }  from '@ohos.base';

export default namespace vpnExtension {
  loadLibrary("vpn_extension_ani")

  export type LinkAddress = connection.LinkAddress;
  export type RouteInfo = connection.RouteInfo;

  export native function startVpnExtensionAbilitySync(want: Want): void;

  export function startVpnExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return startVpnExtensionAbilitySync(want);
      }).then((content: Any) => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function stopVpnExtensionAbilitySync(want: Want): void;

  export function stopVpnExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return stopVpnExtensionAbilitySync(want);
      }).then((content: Any) => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function setAlwaysOnVpnEnabledSync(enable: boolean, bundleName: string): void;

  export function setAlwaysOnVpnEnabled(enable: boolean, bundleName: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return setAlwaysOnVpnEnabledSync(enable, bundleName);
      }).then((content: Any) => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function isAlwaysOnVpnEnabledSync(bundleName: string):boolean;

  export function isAlwaysOnVpnEnabled(bundleName: string): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      taskpool.execute((): boolean => {
        return isAlwaysOnVpnEnabledSync(bundleName);
      }).then((content: Any) => {
        resolve(content as boolean);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function updateVpnAuthorizedStateSync(bundleName: string):boolean;

  export function updateVpnAuthorizedState(bundleName: string): boolean {
    return updateVpnAuthorizedStateSync(bundleName);
  }

  export native function createVpnConnectionSync(context: VpnExtensionContext): VpnConnection;

  export function createVpnConnection(context: VpnExtensionContext): VpnConnection {
    return createVpnConnectionSync(context);
  }

  export native function createVpnObserverSync(): VpnObserver;

  export function createVpnObserver(): VpnObserver {
    return createVpnObserverSync();
  }

  class VpnObserverCleaner {
    private nativePtr: long = 0
    constructor(ptr:long) {
      this.nativePtr = ptr
    }
    native native_clean(): void
  }

  let vpnObserverDestroyRegister: FinalizationRegistry<VpnObserverCleaner> =
    new FinalizationRegistry<VpnObserverCleaner>((cleaner: VpnObserverCleaner) => {cleaner.native_clean()})
  let vpnObserverUnregisterToken: Object = new Object()

  export interface VpnObserver {
    onAuthorizationResult(callback: Callback<boolean>): void;
    offAuthorizationResult(callback?: Callback<boolean>): void;
  }

  class VpnObserverInner implements VpnObserver {
    private nativePtr: long = 0
    private cleaner: VpnObserverCleaner | null = null;

    constructor(nativePtr: long) {
        if(this.nativePtr == 0){
            this.nativePtr = nativePtr;
        }
        this.registerCleaner(this.nativePtr)
    }

    registerCleaner(ptr: long): void {
        this.cleaner = new VpnObserverCleaner(ptr)
        vpnObserverDestroyRegister.register(this, this.cleaner!, vpnObserverUnregisterToken);
    }

    unregisterCleaner(): void {
        vpnObserverDestroyRegister.unregister(vpnObserverUnregisterToken);
    }

    native onAuthorizationResultSync(callback: Callback<boolean>): void;
    native offAuthorizationResultSync(callback?: Callback<boolean>): void;

    onAuthorizationResult(callback: Callback<boolean>): void {
      this.onAuthorizationResultSync(callback);
    }

    offAuthorizationResult(callback?: Callback<boolean>): void {
      if (callback == undefined) {
        this.offAuthorizationResultSync();
      } else {
        this.offAuthorizationResultSync(callback);
      }
    }
  }

  class VpnConnectionCleaner {
    private nativePtr: long = 0
    constructor(ptr:long) {
      this.nativePtr = ptr
    }
    native native_clean(): void
  }

  let vpnConnectionDestroyRegister: FinalizationRegistry<VpnConnectionCleaner> =
    new FinalizationRegistry<VpnConnectionCleaner>((cleaner: VpnConnectionCleaner) => {cleaner.native_clean()})
  let vpnConnectionUnregisterToken: Object = new Object()

  export interface VpnConnection {
    create(config: VpnConfig): Promise<int>;
    generateVpnId(): Promise<string>;
    protect(socketFd: int): Promise<void>;
    destroy(): Promise<void>;
    destroy(vpnId: string): Promise<void>;
    protectProcessNet(): Promise<void>;  
  }

  export class VpnConnectionInner implements VpnConnection {
    private nativePtr: long = 0;
    private cleaner: VpnConnectionCleaner | null = null;

    constructor(context: long) {
        if (this.nativePtr == 0) {
            this.nativePtr = context;
        }
        this.registerCleaner(this.nativePtr);
    }

    registerCleaner(ptr: long): void {
        this.cleaner = new VpnConnectionCleaner(ptr);
        vpnConnectionDestroyRegister.register(this, this.cleaner!, vpnConnectionUnregisterToken);
    }
    unregisterCleaner(): void {
        vpnConnectionDestroyRegister.unregister(vpnConnectionUnregisterToken);
    }

    native createSync(config: VpnConfig): int;

    native generateVpnIdSync(): string;

    native protectSync(socketFd: int): void;

    native destroySync(vpnId?: string): void;

    native protectProcessNetSync(): void;

    create(config: VpnConfig): Promise<int> {
      return new Promise<int>((resolve, reject) => {
        taskpool.execute((): int => {
          return this.createSync(config);
        }).then((content: Any) => {
          resolve(content as int);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    generateVpnId(): Promise<string> {
      return new Promise<string>((resolve, reject) => {
        taskpool.execute((): string => {
          return this.generateVpnIdSync();
        }).then((content: Any) => {
          resolve(content as string);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    protect(socketFd: int): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.protectSync(socketFd);
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    destroy(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.destroySync();
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    destroy(vpnId: string): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.destroySync(vpnId);
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    protectProcessNet(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.protectProcessNetSync();
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }
  }

  export interface VpnConfig {
    vpnId?: string;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isInternal?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
  }

  export class VpnConfigInner implements VpnConfig {
    vpnId?: string;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isInternal?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
  }
}
