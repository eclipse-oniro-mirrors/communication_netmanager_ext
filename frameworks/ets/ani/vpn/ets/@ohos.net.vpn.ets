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
import UIAbilityContext from 'application.UIAbilityContext';
import { BusinessError } from '@ohos.base';

export default namespace vpn {
  loadLibrary('vpn_ani');

  export type NetAddress = connection.NetAddress;
  export type LinkAddress = connection.LinkAddress;
  export type RouteInfo = connection.RouteInfo;

  export class Cleaner {
    private nativePtr: long = 0;

    constructor(nativePtr: long) {
      this.nativePtr = nativePtr;
    }

    native clean(): void;
  }

  let destroyRegister: FinalizationRegistry<Cleaner> =
    new FinalizationRegistry<Cleaner>((cleaner: Cleaner): void => {
      cleaner.clean();
    });
  let unregisterToken: object = new Object();

  export native function createVpnConnectionSync(context: UIAbilityContext): VpnConnection;

  export function createVpnConnection(context: UIAbilityContext): VpnConnection {
    return createVpnConnectionSync(context);
  }

  export native function onConnectSync(callback: Callback<VpnConnectState>): void;

  export function onConnect(callback: Callback<VpnConnectState>): void {
    return onConnectSync(callback);
  }

  export native function onConnectMultiSync(callback: Callback<MultiVpnConnectState>): void;

  export function onConnectMulti(callback: Callback<MultiVpnConnectState>): void {
    return onConnectMultiSync(callback);
  }

  export native function offConnectSync(callback?: Callback<VpnConnectState>): void;

  export function offConnect(callback?: Callback<VpnConnectState>): void {
    if (callback === undefined) {
      offConnectSync();
    } else {
      offConnectSync(callback);
    }
  }

  export native function offConnectMultiSync(callback?: Callback<MultiVpnConnectState>): void;

  export function offConnectMulti(callback?: Callback<MultiVpnConnectState>): void {
    if (callback === undefined) {
      offConnectMultiSync();
    } else {
      offConnectMultiSync(callback);
    }
  }

  export native function addSysVpnConfigSync(config: SysVpnConfig): void;

  export function addSysVpnConfig(config: SysVpnConfig): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return addSysVpnConfigSync(config);
      }).then((content: Any) => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function deleteSysVpnConfigSync(vpnId: string): void;

  export function deleteSysVpnConfig(vpnId: string): Promise<void> {
    return new Promise<void>((resolve, reject) => {
      taskpool.execute((): void => {
        return deleteSysVpnConfigSync(vpnId);
      }).then((content: Any) => {
        resolve(undefined);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getSysVpnConfigListSync(): Array<SysVpnConfig>;

  export function getSysVpnConfigList(): Promise<Array<SysVpnConfig>> {
    return new Promise<Array<SysVpnConfig>>((resolve, reject) => {
      taskpool.execute((): Array<SysVpnConfig> => {
        return getSysVpnConfigListSync();
      }).then((content: Any) => {
        resolve(content as Array<SysVpnConfig>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getSysVpnConfigSync(vpnId: string): SysVpnConfig;

  export function getSysVpnConfig(vpnId: string): Promise<SysVpnConfig> {
    return new Promise<SysVpnConfig>((resolve, reject) => {
      taskpool.execute((): SysVpnConfig => {
        return getSysVpnConfigSync(vpnId);
      }).then((content: Any) => {
        resolve(content as SysVpnConfig);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getConnectedSysVpnConfigSync(): SysVpnConfig;

  export function getConnectedSysVpnConfig(): Promise<SysVpnConfig> {
    return new Promise<SysVpnConfig>((resolve, reject) => {
      taskpool.execute((): SysVpnConfig => {
        return getConnectedSysVpnConfigSync();
      }).then((content: Any) => {
        resolve(content as SysVpnConfig);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export native function getConnectedVpnAppInfoSync(): Array<string>;

  export function getConnectedVpnAppInfo(): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
        return getConnectedVpnAppInfoSync();
      }).then((content: Any) => {
        resolve(content as Array<string>);
      }, (err: Error): void => {
        reject(err as BusinessError);
      });
    });
  }

  export interface VpnConnection {
    setUp(config: VpnConfig, callback: AsyncCallback<int>): void;
    setUp(config: VpnConfig): Promise<int>;
    protect(socketFd: int, callback: AsyncCallback<void>): void;
    protect(socketFd: int): Promise<void>;
    destroy(callback: AsyncCallback<void>): void;
    destroy(): Promise<void>;
  }

  export class VpnConnectionInner implements VpnConnection {
    private nativePtr: long = 0;
    private cleaner: Cleaner | null = null;

    constructor(context: long) {
      if (this.nativePtr === 0) {
        this.nativePtr = context;
      }
      this.registerCleaner(this.nativePtr);
    }

    registerCleaner(nativePtr: long): void {
      this.cleaner = new Cleaner(nativePtr);
      destroyRegister.register(this, this.cleaner!, unregisterToken);
    }

    unregisterCleaner(): void {
      destroyRegister.unregister(unregisterToken);
    }

    native setUpSync(config: VpnConfig): int;

    setUp(config: VpnConfig, callback: AsyncCallback<int>): void {
      let p1 = taskpool.execute((): int => {
        return this.setUpSync(config);
      });
      p1.then((content: Any) => {
        callback(new BusinessError(), content as int);
      }, (err: Error): void => {
        callback(err as BusinessError, 0);
      });
    }

    setUp(config: VpnConfig): Promise<int> {
      return new Promise<int>((resolve, reject) => {
        taskpool.execute((): int => {
          return this.setUpSync(config);
        }).then((content: Any) => {
          resolve(content as int);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    native protectSync(socketFd: int): void;

    protect(socketFd: int, callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute((): void => {
        return this.protectSync(socketFd);
      });
      p1.then((content: Any) => {
        callback(new BusinessError(), undefined);
      }, (err: Error): void => {
        callback(err as BusinessError, undefined);
      });
    }

    protect(socketFd: int): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.protectSync(socketFd);
        }).then((content: Any) => {
          resolve(undefined);
        }, (err: Error): void => {
          reject(err as BusinessError);
        });
      });
    }

    native destroySync(): void;

    destroy(callback: AsyncCallback<void>): void {
      let p1 = taskpool.execute((): void => {
        return this.destroySync();
      });
      p1.then((content: Any) => {
        callback(new BusinessError(), undefined);
      }, (err: Error): void => {
        callback(err as BusinessError, undefined);
      });
    }

    destroy(): Promise<void> {
      return new Promise<void>((resolve, reject) => {
        taskpool.execute((): void => {
          return this.destroySync();
        }).then((content: Any) => {
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
    isLegacy?: boolean;
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

  export class SysVpnConfigInner implements SysVpnConfig {
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

  export class OpenVpnConfigInner implements OpenVpnConfig {
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

  export class IpsecVpnConfigInner implements IpsecVpnConfig {
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

  export class L2tpVpnConfigInner implements L2tpVpnConfig {
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

  export enum SysVpnType {
    IKEV2_IPSEC_MSCHAPV2 = 1,
    IKEV2_IPSEC_PSK = 2,
    IKEV2_IPSEC_RSA = 3,
    L2TP_IPSEC_PSK = 4,
    L2TP_IPSEC_RSA = 5,
    IPSEC_XAUTH_PSK = 6,
    IPSEC_XAUTH_RSA = 7,
    IPSEC_HYBRID_RSA = 8,
    OPENVPN = 9
  }

  export enum VpnConnectState {
    VPN_CONNECTED = 1,
    VPN_DISCONNECTED,
  }

  export interface MultiVpnConnectState {
    isConnected: boolean;
    bundleName: string;
    vpnId: string;
  }

  export class MultiVpnConnectStateInner implements MultiVpnConnectState {
    isConnected: boolean;
    bundleName: string;
    vpnId: string;
  }
}
