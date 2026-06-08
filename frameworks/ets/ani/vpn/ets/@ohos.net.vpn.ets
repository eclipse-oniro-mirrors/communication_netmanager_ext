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
import { BusinessError }  from '@ohos.base';

function toBusinessError(err: Error): BusinessError {
  let businessErr = new BusinessError();
  businessErr.code = err.code ?? 0;
  businessErr.message = err.message ?? '';
  businessErr.stack = err.stack ?? '';
  businessErr.cause = err;
  return businessErr;
}

export default namespace vpn {
  export type AbilityContext = Object;
  export type NetAddress = connection.NetAddress;
  export type LinkAddress = connection.LinkAddress;
  export type RouteInfo = connection.RouteInfo;

  loadLibrary("vpn_ani")

  export native function createVpnConnectionSync():int;

  export function createVpnConnection(context: AbilityContext): VpnConnection {
    let ret = createVpnConnectionSync();
    if (ret != 0) {
      throw new BusinessError(ret, 'System internal error.');
    }
    return new VpnConnection();
  }

  export native function setUpSync(config: VpnConfig):int;

  export function setUp(config: VpnConfig, callback: AsyncCallback<int>): void {
    let p1 = taskpool.execute((): int => {
      return setUpSync(config);
    })
    p1.then((content: Any) => {
      callback(null, content as int);
    }).catch((err: Error) => {
      callback(toBusinessError(err), 0);
    });
  }

  export function setUp(config: VpnConfig): Promise<int> {
    return new Promise<int>((resolve, reject) => {
      taskpool.execute((): int => {
        return setUpSync(config);
      }).then((content: Any) => {
        resolve(content as int);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function protectSync(socketFd: int):int;

  export function protect(socketFd: int, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute(() => {
      protectSync(socketFd);
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function protect(socketFd: int): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute(() => {
        protectSync(socketFd);
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function destroySync():int;

  export function destroy(callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute(() => {
      destroySync();
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function destroy(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute(() => {
        destroySync();
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function addSysVpnConfigSync(config: SysVpnConfig):int;

  export function addSysVpnConfig(config: SysVpnConfig, callback: AsyncCallback<void>): void {
    let password = config.password;
    let pkcs12Password = config.pkcs12Password;
    if (password && password.length > 256) {
      callback(new BusinessError(401, 'Parameter error: password too long'), undefined);
      return;
    }
    if (pkcs12Password && pkcs12Password.length > 256) {
      callback(new BusinessError(401, 'Parameter error: pkcs12Password too long'), undefined);
      return;
    }
    let p1 = taskpool.execute(() => {
      addSysVpnConfigSync(config);
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function addSysVpnConfig(config: SysVpnConfig): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      let password = config.password;
      let pkcs12Password = config.pkcs12Password;
      if (password && password.length > 256) {
        reject(new BusinessError(401, 'Parameter error: password too long'));
      } else if (pkcs12Password && pkcs12Password.length > 256) {
        reject(new BusinessError(401, 'Parameter error: pkcs12Password too long'));
      } else {
        taskpool.execute(() => {
          addSysVpnConfigSync(config);
        }).then(() => {
          resolve(undefined);
        }).catch((err: Error) => {
          reject(toBusinessError(err));
        });
      }
    });
  }

  export native function deleteSysVpnConfigSync(vpnId: string):int;

  export function deleteSysVpnConfig(vpnId: string, callback: AsyncCallback<void>): void {
    if (!vpnId || vpnId.length === 0) {
      let err = new BusinessError(401, 'Invalid parameter: vpnId is empty or undefined');
      callback(err, undefined);
      return;
    }
    let p1 = taskpool.execute(() => {
      deleteSysVpnConfigSync(vpnId);
    })
    p1.then(() => {
      callback(null, undefined);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function deleteSysVpnConfig(vpnId: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      if (!vpnId || vpnId.length === 0) {
        let err = new BusinessError(401, 'Invalid parameter: vpnId is empty or undefined');
        reject(err);
        return;
      }
      taskpool.execute(() => {
        deleteSysVpnConfigSync(vpnId);
      }).then(() => {
        resolve(undefined);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function getSysVpnConfigListSync():Array<SysVpnConfig>;

  export function getSysVpnConfigList(callback: AsyncCallback<Array<SysVpnConfig>>): void {
    let p1 = taskpool.execute((): Array<SysVpnConfig> => {
      return getSysVpnConfigListSync();
    })
    p1.then((content: Any) => {
      callback(null, content as Array<SysVpnConfig>);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getSysVpnConfigList(): Promise<Array<SysVpnConfig>> {
    return new Promise<Array<SysVpnConfig>>((resolve, reject) => {
      taskpool.execute((): Array<SysVpnConfig> => {
        return getSysVpnConfigListSync();
      }).then((content: Any) => {
        resolve(content as Array<SysVpnConfig>);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function getSysVpnConfigSync(vpnId: string):SysVpnConfig;

  export function getSysVpnConfig(vpnId: string, callback: AsyncCallback<SysVpnConfig>): void {
    if (!vpnId || vpnId.length === 0) {
      let err = new BusinessError(401, 'Invalid parameter: vpnId is empty or undefined');
      callback(err, undefined);
      return;
    }
    let p1 = taskpool.execute((): SysVpnConfig => {
      return getSysVpnConfigSync(vpnId);
    })
    p1.then((content: Any) => {
      callback(null, content as SysVpnConfig);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getSysVpnConfig(vpnId: string): Promise<SysVpnConfig> {
    return new Promise<SysVpnConfig>((resolve, reject) => {
      if (!vpnId || vpnId.length === 0) {
        let err = new BusinessError(401, 'Invalid parameter: vpnId is empty or undefined');
        reject(err);
        return;
      }
      taskpool.execute((): SysVpnConfig => {
        return getSysVpnConfigSync(vpnId);
      }).then((content: Any) => {
        resolve(content as SysVpnConfig);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function getConnectedSysVpnConfigSync():SysVpnConfig;

  export function getConnectedSysVpnConfig(callback: AsyncCallback<SysVpnConfig>): void {
    let p1 = taskpool.execute((): SysVpnConfig => {
      return getConnectedSysVpnConfigSync();
    })
    p1.then((content: Any) => {
      callback(null, content as SysVpnConfig);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getConnectedSysVpnConfig(): Promise<SysVpnConfig> {
    return new Promise<SysVpnConfig>((resolve, reject) => {
      taskpool.execute((): SysVpnConfig => {
        return getConnectedSysVpnConfigSync();
      }).then((content: Any) => {
        resolve(content as SysVpnConfig);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export native function getConnectedVpnAppInfoSync():Array<string>;

  export function getConnectedVpnAppInfo(callback: AsyncCallback<Array<string>>): void {
    let p1 = taskpool.execute((): Array<string> => {
      return getConnectedVpnAppInfoSync();
    })
    p1.then((content: Any) => {
      callback(null, content as Array<string>);
    }).catch((err: Error) => {
      callback(toBusinessError(err), undefined);
    });
  }

  export function getConnectedVpnAppInfo(): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
        return getConnectedVpnAppInfoSync();
      }).then((content: Any) => {
        resolve(content as Array<string>);
      }).catch((err: Error) => {
        reject(toBusinessError(err));
      });
    });
  }

  export function on(type: 'connect' | 'connectMulti', callback: Callback<VpnConnectState | MultiVpnConnectState>): void {
    if (type == 'connect') {
      onConnect(callback as Callback<VpnConnectState>);
    } else if (type == 'connectMulti') {
      onConnectMulti(callback as Callback<MultiVpnConnectState>);
    }
  }

  export function off(type: 'connect' | 'connectMulti', callback?: Callback<VpnConnectState | MultiVpnConnectState>): void {
    if (type == 'connect') {
      offConnect(callback as Callback<VpnConnectState>);
    } else if (type == 'connectMulti') {
      offConnectMulti(callback as Callback<MultiVpnConnectState>);
    }
  }

  export native function onConnect(callback: Callback<VpnConnectState>): void;

  export native function offConnect(callback?: Callback<VpnConnectState>): void;

  export native function onConnectMulti(callback: Callback<MultiVpnConnectState>): void;

  export native function offConnectMulti(callback?: Callback<MultiVpnConnectState>): void;

  export class VpnConnection {
    setUp(config: VpnConfig, callback: AsyncCallback<int>): void {
      let p1 = taskpool.execute((): int => {
        return setUpSync(config);
      })
      p1.then((content: Any) => {
        callback(null, content as int);
      }).catch((err: Error) => {
        callback(toBusinessError(err), 0);
      });
    }

    setUp(config: VpnConfig): Promise<int> {
      return new Promise<int>((resolve, reject) => {
        taskpool.execute((): int => {
          return setUpSync(config);
        }).then((content: Any) => {
          resolve(content as int);
        }).catch((err: Error) => {
          reject(toBusinessError(err));
        });
      });
    }

    protect(socketFd: int, callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute(() => {
        protectSync(socketFd);
      })
      p1.then(() => {
        callback(null, undefined);
      }).catch((err: Error) => {
        callback(toBusinessError(err), undefined);
      });
    }

    protect(socketFd: int): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          protectSync(socketFd);
        }).then(() => {
          resolve(undefined);
        }).catch((err: Error) => {
          reject(toBusinessError(err));
        });
      });
    }

    destroy(callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute(() => {
        destroySync();
      })
      p1.then(() => {
        callback(null, undefined);
      }).catch((err: Error) => {
        callback(toBusinessError(err), undefined);
      });
    }

    destroy(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
          destroySync();
        }).then(() => {
          resolve(undefined);
        }).catch((err: Error) => {
          reject(toBusinessError(err));
        });
      });
    }
  }

  export enum VpnConnectState {
    CONNECTED = 0,
    DISCONNECTED = 1,
  }

  export interface MultiVpnConnectState {
    isConnected: boolean;
    bundleName: string;
    vpnId: string;
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
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
  }

  export interface SysVpnConfig extends VpnConfig {
    vpnId?: string;
    vpnName?: string;
    vpnType?: SysVpnType;
    userName?: string;
    password?: string;
    saveLogin?: boolean;
    userId?: int;
    forwardingRoutes?: string;
    remoteAddresses?: Array<string>;
    localAddresses?: Array<LinkAddress>;
    pkcs12Password?: string;
    pkcs12FileData?: Uint8Array;
  }

  export interface OpenVpnConfig extends SysVpnConfig {
    ovpnPort?: string;
    ovpnProtocol?: int;
    ovpnConfig?: string;
    ovpnAuthType?: int;
    askpass?: string;
    ovpnConfigFilePath?: string;
    ovpnCaCertFilePath?: string;
    ovpnUserCertFilePath?: string;
    ovpnPrivateKeyFilePath?: string;
  }

  export interface IpsecVpnConfig extends SysVpnConfig {
    ipsecPreSharedKey?: string;
    ipsecIdentifier?: string;
    swanctlConfig?: string;
    strongSwanConfig?: string;
    ipsecCaCertConfig?: string;
    ipsecPrivateUserCertConfig?: string;
    ipsecPublicUserCertConfig?: string;
    ipsecPrivateServerCertConfig?: string;
    ipsecPublicServerCertConfig?: string;
    ipsecCaCertFilePath?: string;
    ipsecPrivateUserCertFilePath?: string;
    ipsecPublicUserCertFilePath?: string;
    ipsecPrivateServerCertFilePath?: string;
    ipsecPublicServerCertFilePath?: string;
  }

  export interface L2tpVpnConfig extends SysVpnConfig {
    ipsecPreSharedKey?: string;
    ipsecIdentifier?: string;
    strongSwanConfig?: string;
    ipsecCaCertConfig?: string;
    ipsecPrivateUserCertConfig?: string;
    ipsecPublicUserCertConfig?: string;
    ipsecPrivateServerCertConfig?: string;
    ipsecPublicServerCertConfig?: string;
    ipsecCaCertFilePath?: string;
    ipsecPrivateUserCertFilePath?: string;
    ipsecPublicUserCertFilePath?: string;
    ipsecPrivateServerCertFilePath?: string;
    ipsecPublicServerCertFilePath?: string;
    ipsecConfig?: string;
    ipsecSecrets?: string;
    optionsL2tpdClient?: string;
    xl2tpdConfig?: string;
    l2tpSharedKey?: string;
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
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
  }

  export class SysVpnConfigInner implements SysVpnConfig {
    vpnId?: string;
    vpnName?: string;
    vpnType?: SysVpnType;
    userName?: string;
    password?: string;
    saveLogin?: boolean;
    userId?: int;
    forwardingRoutes?: string;
    remoteAddresses?: Array<string>;
    localAddresses?: Array<LinkAddress>;
    pkcs12Password?: string;
    pkcs12FileData?: Uint8Array;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
  }

  export class OpenVpnConfigInner implements SysVpnConfig {
    vpnId?: string;
    vpnName?: string;
    vpnType?: SysVpnType;
    userName?: string;
    password?: string;
    saveLogin?: boolean;
    userId?: int;
    forwardingRoutes?: string;
    remoteAddresses?: Array<string>;
    localAddresses?: Array<LinkAddress>;
    pkcs12Password?: string;
    pkcs12FileData?: Uint8Array;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
    ovpnPort?: string;
    ovpnProtocol?: int;
    ovpnConfig?: string;
    ovpnAuthType?: int;
    askpass?: string;
    ovpnConfigFilePath?: string;
    ovpnCaCertFilePath?: string;
    ovpnUserCertFilePath?: string;
    ovpnPrivateKeyFilePath?: string;
  }

  export class IpsecVpnConfigInner implements SysVpnConfig {
    vpnId?: string;
    vpnName?: string;
    vpnType?: SysVpnType;
    userName?: string;
    password?: string;
    saveLogin?: boolean;
    userId?: int;
    forwardingRoutes?: string;
    remoteAddresses?: Array<string>;
    localAddresses?: Array<LinkAddress>;
    pkcs12Password?: string;
    pkcs12FileData?: Uint8Array;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
    ipsecPreSharedKey?: string;
    ipsecIdentifier?: string;
    swanctlConfig?: string;
    strongSwanConfig?: string;
    ipsecCaCertConfig?: string;
    ipsecPrivateUserCertConfig?: string;
    ipsecPublicUserCertConfig?: string;
    ipsecPrivateServerCertConfig?: string;
    ipsecPublicServerCertConfig?: string;
    ipsecCaCertFilePath?: string;
    ipsecPrivateUserCertFilePath?: string;
    ipsecPublicUserCertFilePath?: string;
    ipsecPrivateServerCertFilePath?: string;
    ipsecPublicServerCertFilePath?: string;
  }

  export class L2tpVpnConfigInner implements SysVpnConfig {
    vpnId?: string;
    vpnName?: string;
    vpnType?: SysVpnType;
    userName?: string;
    password?: string;
    saveLogin?: boolean;
    userId?: int;
    forwardingRoutes?: string;
    remoteAddresses?: Array<string>;
    localAddresses?: Array<LinkAddress>;
    pkcs12Password?: string;
    pkcs12FileData?: Uint8Array;
    addresses: Array<LinkAddress>;
    routes?: Array<RouteInfo>;
    dnsAddresses?: Array<string>;
    searchDomains?: Array<string>;
    mtu?: int;
    isIPv4Accepted?: boolean;
    isIPv6Accepted?: boolean;
    isLegacy?: boolean;
    isBlocking?: boolean;
    trustedApplications?: Array<string>;
    blockedApplications?: Array<string>;
    ipsecConfig?: string;
    ipsecSecrets?: string;
    optionsL2tpdClient?: string;
    xl2tpdConfig?: string;
    l2tpSharedKey?: string;
  }

  export enum SysVpnType {
    IKEV2_IPSEC_MSCHAPV2 = 1,
    IKEV2_IPSEC_PSK = 2,
    IKEV2_IPSEC_RSA = 3,
    L2TP_IPSEC_PSK = 4,
    L2TP_IPSEC_RSA = 5,
    IPSEC_XAUTH_PSK = 6,
    IPSEC_XAUTH_RSA = 7,
    IPSEC_HYBRID_RSA = 8,
    OPENVPN = 9,
  }
}
