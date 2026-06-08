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
import type Want from '@ohos.app.ability.Want';
import { BusinessError }  from '@ohos.base';

function ensureBusinessError(err: Error): BusinessError {
  if (err instanceof BusinessError) {
    return new BusinessError(err.code, err.message || 'Unknown error');
  }
  return new BusinessError(2010001, err.message || 'Unknown error');
}

export default namespace vpnExtension {
  export type LinkAddress = connection.LinkAddress;
  export type RouteInfo = connection.RouteInfo;
  export type VpnExtensionContext = Object;
  loadLibrary("vpnextension_ani")

  export native function startVpnExtensionAbilitySync(bundleName: string, abilityName: string): int;

  export function startVpnExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (!want.bundleName || !want.abilityName) {
        reject(new BusinessError(401, 'Parameter error: bundleName and abilityName are required'));
      } else {
        taskpool.execute((): int => {
          return startVpnExtensionAbilitySync(want.bundleName ?? "", want.abilityName ?? "");
        }).then((content: Any) => {
          let ret = content as int;
          if (ret != 0) {
            reject(new BusinessError(ret, 'System internal error.'));
          } else {
            resolve(undefined);
          }
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      }
    });
  }

  export native function stopVpnExtensionAbilitySync(bundleName: string, abilityName: string):int;

  export function stopVpnExtensionAbility(want: Want): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (!want.bundleName || !want.abilityName) {
        reject(new BusinessError(401, 'Parameter error: bundleName and abilityName are required'));
      } else {
        taskpool.execute((): int => {
          return stopVpnExtensionAbilitySync(want.bundleName ?? "", want.abilityName ?? "");
        }).then((content: Any) => {
          let ret = content as int;
          if (ret != 0) {
            reject(new BusinessError(ret, 'System internal error.'));
          } else {
            resolve(undefined);
          }
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      }
    });
  }

  export native function setAlwaysOnVpnEnabledSync(enable: boolean, bundleName: string):int;

  export function setAlwaysOnVpnEnabled(enable: boolean, bundleName: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): int => {
        return setAlwaysOnVpnEnabledSync(enable, bundleName);
      }).then((content: Any) => {
        let ret = content as int;
        if (ret != 0) {
          reject(new BusinessError(ret, 'System internal error.'));
        } else {
          resolve(undefined);
        }
      }, (err: Error): void => {
        reject(ensureBusinessError(err));
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
        reject(ensureBusinessError(err));
      });
    });
  }

  export native function updateVpnAuthorizedStateSync(bundleName: string):boolean;

  export function updateVpnAuthorizedState(bundleName: string): boolean {
    return updateVpnAuthorizedStateSync(bundleName);
  }

  export native function createVpnConnectionSync():int;

  export function createVpnConnection(context: VpnExtensionContext): VpnConnection {
    let ret = createVpnConnectionSync();
    if (ret != 0) {
      throw new BusinessError(ret, 'System internal error.');
    }
    return new VpnConnection();
  }

  export function createVpnObserver(): VpnObserver {
    return new VpnObserver();
  }

  export native function createSync(config: VpnConfig):int;

  export native function generateVpnIdSync():string;

  export native function protectSync(socketFd: int):int;

  export native function destroySync():int;

  export native function destroyVpnSync(vpnId: string):int;

  export native function protectProcessNetSync():int;

  export class VpnConnection {
    create(config: VpnConfig): Promise<int> {
      return new Promise<int>((resolve, reject) => {
        if (!config.addresses || config.addresses.length === 0) {
          reject(new BusinessError(401, 'Parameter error: addresses is required'));
        } else {
          taskpool.execute((): int => {
            return createSync(config);
          }).then((content: Any) => {
            let fd = content as int;
            if (fd < 0) {
              reject(new BusinessError(fd, 'Failed to create VPN connection.'));
            } else {
              resolve(fd);
            }
          }, (err: Error): void => {
            reject(ensureBusinessError(err));
          });
        }
      });
    }

    generateVpnId(): Promise<string> {
      return new Promise<string>((resolve, reject) => {
        taskpool.execute((): string => {
          return generateVpnIdSync();
        }).then((content: Any) => {
          resolve(content as string);
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      });
    }

    protect(socketFd: int): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          protectSync(socketFd);
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      });
    }

    destroy(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          destroySync();
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      });
    }

    destroy(vpnId: string): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          destroyVpnSync(vpnId);
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      });
    }

    protectProcessNet(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          protectProcessNetSync();
        }).then(() => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(ensureBusinessError(err));
        });
      });
    }
  }

  export native function registOnAuthResult(callback: Callback<boolean>):void;

  export native function unregistOnAuthResult(callback?: Callback<boolean>):void;

  export class VpnObserver {
    onAuthorizationResult(callback: Callback<boolean>): void {
      registOnAuthResult(callback);
    }

    offAuthorizationResult(callback?: Callback<boolean>): void {
      unregistOnAuthResult(callback);
    }
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
}
