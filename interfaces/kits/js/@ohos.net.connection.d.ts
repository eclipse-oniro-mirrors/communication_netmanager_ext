/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback, Callback} from "./basic";
import http from "./@ohos.net.http";
import socket from "./@ohos.net.socket";

/**
 * Provides interfaces to manage and use data networks.
 *
 * @since 8
 * @sysCap SystemCapability.Communication.NetManager
 * @devices phone, tablet, tv, wearable, car
 */
declare namespace connection {
  type HttpRequest = http.HttpRequest;
  type TCPSocket = socket.TCPSocket;
  type UDPSocket = socket.UDPSocket;

  function on(type: 'netAvailable', callback: Callback<NetHandle>): void;
  function off(type: 'netAvailable', callback?: Callback<NetHandle>): void;

  function on(type: 'netBlockStatusChange', callback: Callback<{ netHandle: NetHandle, blocked: boolean }>): void;
  function off(type: 'netBlockStatusChange', callback?: Callback<{ netHandle: NetHandle, blocked: boolean }>): void;

  function on(type: 'netCapabilitiesChange', callback: Callback<{ netHandle: NetHandle, netCap: NetCapabilities }>): void;
  function off(type: 'netCapabilitiesChange', callback?: Callback<{ netHandle: NetHandle, netCap: NetCapabilities }>): void;

  function on(type: 'netConnectionPropertiesChange', callback: Callback<{ netHandle: NetHandle,connectionProperties: ConnectionProperties }>): void;
  function off(type: 'netConnectionPropertiesChange', callback?: Callback<{ netHandle: NetHandle, connectionProperties: ConnectionProperties }>): void;

  function on(type: 'netLosing', callback: Callback<{ netHandle: NetHandle, maxMsToLive: number }>): void;
  function off(type: 'netLosing', callback?: Callback<{ netHandle: NetHandle, maxMsToLive: number }>): void;

  function on(type: 'netLost', callback: Callback<NetHandle>): void;
  function off(type: 'netLost', callback?: Callback<NetHandle>): void;

  function on(type: 'netUnavailable', callback: Callback<void>): void;
  function off(type: 'netUnavailable', callback?: Callback<void>): void;

  /**
   * Receives status change notifications of a specified network.
   *
   * <p>{@link removeNetStatusCallback} can be used to cancel receiving status change notifications.
   * This method requires the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param netSpecifier Indicates the network specifier. See {@link NetSpecifier}.
   * @param callback Indicates the callback to notify the application of network status changes.
   * @permission ohos.permission.GET_NETWORK_INFO or ohos.permission.SET_NETWORK_INFO
   */
  function addNetStatusCallback(callback: AsyncCallback<void>): void;
  function addNetStatusCallback(netSpecifier: NetSpecifier, callback: AsyncCallback<void>): void;
  function addNetStatusCallback(netSpecifier?: NetSpecifier): Promise<void>;

  /**
   * Cancels listening for network status changes.
   *
   * @param netSpecifier Indicates the network specifier. See {@link NetSpecifier}.
   * @param callback Indicates the callback to cancel.
   */
  function removeNetStatusCallback(callback: AsyncCallback<void>): void;
  function removeNetStatusCallback(netSpecifier: NetSpecifier, callback: AsyncCallback<void>): void;
  function removeNetStatusCallback(netSpecifier?: NetSpecifier): Promise<void>;

  /**
   * Obtains the {@link NetHandle} bound to a process using {@link setAppNet}.
   *
   * @param callback Returns the {@link NetHandle} bound to the process;
   *      returns {@code null} if no {@link NetHandle} is bound to the process.
   *      For details, see {@link NetHandle}.
   */
  function getAppNet(callback: AsyncCallback<NetHandle>): void;
  function getAppNet(): Promise<NetHandle>;

  /**
   * Binds a process to {@code handle}.
   *
   * <p>All the sockets created from the process will be bound to the {@code handle},
   * and the resolution of all host names will be managed by the {@code handle}. You can unbind the process
   * from {@code handle} by setting {@code nethanlde} to {@code null}.
   *
   * @param netHandle Indicates the handle. For details, see {@link NetHandle}.
   */
  function setAppNet(netHandle: NetHandle, callback: AsyncCallback<void>): void;
  function setAppNet(netHandle: NetHandle): Promise<void>;

  /**
   * Obtains the data network that is activated by default.
   *
   * <p>To call this method, you must have the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param callback Returns the {@link NetHandle} object;
   *      returns {@code null} if the default network is not activated.
   * @permission ohos.permission.GET_NETWORK_INFO
   */
  function getDefaultNet(callback: AsyncCallback<NetHandle>): void;
  function getDefaultNet(): Promise<NetHandle>;

  /**
   * Obtains the list of data networks that are activated.
   *
   * <p>To invoke this method, you must have the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param callback Returns the {@link NetHandle} object; returns {@code null} if no network is activated.
   * @permission ohos.permission.GET_NETWORK_INFO
   */
  function getAllNets(callback: AsyncCallback<Array<NetHandle>>): void;
  function getAllNets(): Promise<Array<NetHandle>>;

  /**
   * Obtains the default {@link HttpProxy} proxy settings.
   *
   * <p>If a global proxy is set, the global proxy parameters are returned.
   * If the process is bound to a {@link NetHandle} using {@link setAppNet},
   * the {@link NetHandle} proxy settings are returned.
   * In other cases, the default proxy settings of {@link NetHandle} are returned.
   *
   * @param callback Returns the proxy settings. For details, see {@link HttpProxy}.
   */
  function getDefaultHttpProxy(callback: AsyncCallback<HttpProxy>): void;
  function getDefaultHttpProxy(): Promise<HttpProxy>;

  /**
   * Queries the connection properties of a network.
   *
   * <p>This method requires the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param netHandle Indicates the network to be queried.
   * @param callback Returns the {@link ConnectionProperties} object.
   * @permission ohos.permission.GET_NETWORK_INFO
   */
  function getConnectionProperties(netHandle: NetHandle, callback: AsyncCallback<ConnectionProperties>): void;
  function getConnectionProperties(netHandle: NetHandle): Promise<ConnectionProperties>;

  /**
   * Obtains {@link NetCapabilities} of a {@link NetHandle} object.
   *
   * <p>To invoke this method, you must have the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param netHandle Indicates the handle. See {@link NetHandle}.
   * @param callback Returns {@link NetCapabilities}; returns {@code null} if {@code handle} is invalid.
   * @permission ohos.permission.GET_NETWORK_INFO
   */
  function getNetCapabilities(netHandle: NetHandle, callback: AsyncCallback<NetCapabilities>): void;
  function getNetCapabilities(netHandle: NetHandle): Promise<NetCapabilities>;

  /**
   * Queries the background policy of the current application.
   *
   * @param callback Returns the background policy of the current application.
   *      For details, see {@link BackgroundPolicy#BACKGROUND_POLICY_DISABLE}.
   */
  function getBackgroundPolicy(callback: AsyncCallback<BackgroundPolicy>): void;
  function getBackgroundPolicy(): Promise<BackgroundPolicy>;

  /**
   * Checks whether data traffic usage on the current network is metered.
   *
   * <p>To call this method, you must have the {@code ohos.permission.GET_NETWORK_INFO} permission.
   *
   * @param callback Returns {@code true} if data traffic usage on the current network is metered;
   *      returns {@code false} otherwise.
   */
  function isDefaultNetMetered(callback: AsyncCallback<boolean>): void;
  function isDefaultNetMetered(): Promise<boolean>;

  /**
   * Checks whether the default data network is activated.
   *
   * @param callback Returns {@code true} if the default data network is activated; returns {@code false} otherwise.
   */
  function hasDefaultNet(callback: AsyncCallback<boolean>): void;
  function hasDefaultNet(): Promise<boolean>;

  /**
   * Enables the airplane mode for a device.
   *
   * @systemapi Hide this for inner system use. Only used for system app.
   */
  function enableAirplaneMode(callback: AsyncCallback<void>): void;
  function enableAirplaneMode(): Promise<void>;

  /**
   * Disables the airplane mode for a device.
   *
   * @systemapi Hide this for inner system use. Only used for system app.
   */
  function disableAirplaneMode(callback: AsyncCallback<void>): void;
  function disableAirplaneMode(): Promise<void>;

  /**
   * Enables the distributed cellular data for a device.
   *
   * @systemapi Hide this for inner system use.
   * @permission ohos.permission.SET_NETWORK_INFO
   */
  function enableDistributedCellularData(callback: AsyncCallback<void>): void;
  function enableDistributedCellularData(): Promise<void>;

  /**
   * Disables the distributed cellular data for a device.
   *
   * @systemapi Hide this for inner system use.
   * @permission ohos.permission.SET_NETWORK_INFO
   */
  function disableDistributedCellularData(callback: AsyncCallback<void>): void;
  function disableDistributedCellularData(): Promise<void>;

  /**
   * Reports the network state is connected.
   *
   * @param netHandle Indicates the network whose state is to be reported.
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function reportNetConnected(netHandle: NetHandle, callback: AsyncCallback<void>): void;
  function reportNetConnected(netHandle: NetHandle): Promise<void>;

  /**
   * Reports the network state is disconnected.
   *
   * @param netHandle Indicates the network whose state is to be reported.
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function reportNetDisconnected(netHandle: NetHandle, callback: AsyncCallback<void>): void;
  function reportNetDisconnected(netHandle: NetHandle): Promise<void>;

  export interface NetSpecifier {
    netCapabilities: NetCapabilities;
    bearerPrivateIdentifier?: string;
  }

  export interface NetHandle {
    netIdVec: number;

    /**
     * Binds a TCPSocket or UDPSocket to the current network. All data flows from
     * the socket will use this network, without being subject to {@link setAppNet}.
     * Before using this method, ensure that the socket is disconnected.
     *
     * @param socketParam Indicates the TCPSocket or UDPSocket object.
     */
    bindSocket(socketParam: TCPSocket | UDPSocket, callback: AsyncCallback<void>): void;
    bindSocket(socketParam: TCPSocket | UDPSocket): Promise<void>;

    /**
     * Accesses a specified URL.
     *
     * @param url Indicates a URL connection.
     * @param callback Returns a {@code URLConnection} object matching the given {@code url}.
     */
    openConnection(url: string, callback: AsyncCallback<HttpRequest>): void;
    openConnection(url: string, proxy: NetProxy, callback: AsyncCallback<HttpRequest>): void;
    openConnection(url: string, proxy?: NetProxy): Promise<HttpRequest>;

    /**
     * Resolves a host name to obtain all IP addresses.
     *
     * @param host Indicates the host name.
     * @param callback Returns the NetAddress list.
     */
    getAddressesByName(host: string, callback: AsyncCallback<Array<NetAddress>>): void;
    getAddressesByName(host: string): Promise<Array<NetAddress>>;

    /**
     * Resolves a host name to obtain the first IP address.
     *
     * @param host Indicates the host name.
     * @return Returns the first NetAddress.
     */
    getAddressByName(host: string, callback: AsyncCallback<NetAddress>): void;
    getAddressByName(host: string): Promise<NetAddress>;
  }

  export interface NetCapabilities {
    linkUpBandwidthKbps?: number;
    linkDownBandwidthKbps?: number;
    networkCap?: Array<NetCap>;
    bearerTypes: Array<NetBearType>;
  }

  export enum NetCap {
    /**
     * Indicates that the network can access the carrier's MMSC to send and receive multimedia messages.
     */
    NET_CAPABILITY_MMS = 0,

    /**
     * Indicates that the network can access the carrier's SUPL server.
     */
    NET_CAPABILITY_SUPL = 1,

    /**
     * Indicates that the network can access the carrier's DUN or Tethering gateway.
     */
    NET_CAPABILITY_DUN = 2,

    /**
     * Indicates that the network can access the FOTA server for remote device upgrade.
     */
    NET_CAPABILITY_FOTA = 3,

    /**
     * Indicates that the network can access the IMS server.
     */
    NET_CAPABILITY_IMS = 4,

    /**
     * Indicates that the network can access the carrier's CBS server.
     */
    NET_CAPABILITY_CBS = 5,

    /**
     * Indicates that the network can be used for Wi-Fi Direct.
     */
    NET_CAPABILITY_WIFI_P2P = 6,

    /**
     * Indicates that the network can access the carrier's Initial Attach server.
     */
    NET_CAPABILITY_IA = 7,

    /**
     * Indicates that the network can access the carrier's RCS server.
     */
    NET_CAPABILITY_RCS = 8,

    /**
     * Indicates that the network can access the carrier's XCAP server.
     */
    NET_CAPABILITY_XCAP = 9,

    /**
     * Indicates that the network can access the carrier's IMS emergency call server.
     */
    NET_CAPABILITY_EIMS = 10,

    /**
     * Indicates that the network traffic is not metered.
     */
    NET_CAPABILITY_NOT_METERED = 11,

    /**
     * Indicates that the network can access the Internet.
     */
    NET_CAPABILITY_INTERNET = 12,

    /**
     * Indicates that the network is not restricted.
     */
    NET_CAPABILITY_NOT_RESTRICTED = 13,

    /**
     * Indicates that the network is trusted.
     */
    NET_CAPABILITY_TRUSTED = 14,

    /**
     * Indicates that the network does not use a VPN.
     */
    NET_CAPABILITY_NOT_VPN = 15,

    /**
     * Indicates that the network is available.
     */
    NET_CAPABILITY_VALIDATED = 16,

    /**
     * Indicates that this network was found to have a captive portal in place last time it was
     * probed.
     */
    NET_CAPABILITY_CAPTIVE_PORTAL = 17,

    /**
     * Indicates that the network is unavailable during roaming.
     */
    NET_CAPABILITY_NOT_ROAMING = 18,

    /**
     * Indicates that the network is available only for foreground applications.
     */
    NET_CAPABILITY_FOREGROUND = 19,

    /**
     * Indicates that the network is not congested.
     */
    NET_CAPABILITY_NOT_CONGESTED = 20,

    /**
     * Indicates that the network is not suspended.
     */
    NET_CAPABILITY_NOT_SUSPENDED = 21,

    /**
     * Indicates that traffic that goes through this network is paid by oem. For example,
     * this network can be used by system apps to upload telemetry data.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_OEM_PAID = 22,

    /**
     * Indicates that the network can access the Mission Critical server of the carrier.
     */
    NET_CAPABILITY_MCX = 23,

    /**
     * Indicates that the network was tested to only provide partial connectivity.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_PARTIAL_CONNECTIVITY = 24,

    /**
     * Indicates that the network extends cap
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_HW_BASE = NET_CAPABILITY_PARTIAL_CONNECTIVITY,

    /**
     * Indicates that the network can access the BIP0 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP0 = NET_CAPABILITY_HW_BASE + 1,

    /**
     * Indicates that the network can access the BIP1 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP1 = NET_CAPABILITY_HW_BASE + 2,

    /**
     * Indicates that the network can access the BIP2 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP2 = NET_CAPABILITY_HW_BASE + 3,

    /**
     * Indicates that the network can access the BIP3 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP3 = NET_CAPABILITY_HW_BASE + 4,

    /**
     * Indicates that the network can access the BIP4 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP4 = NET_CAPABILITY_HW_BASE + 5,

    /**
     * Indicates that the network can access the BIP5 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP5 = NET_CAPABILITY_HW_BASE + 6,

    /**
     * Indicates that the network can access the BIP6 server.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_BIP6 = NET_CAPABILITY_HW_BASE + 7,

    /**
     * Indicates that the network can access internal default servers.
     *
     * @systemapi Hide this for inner system use.
     */
    NET_CAPABILITY_INTERNAL_DEFAULT
  }

  export enum NetBearType {
    /**
     * Indicates that the network is based on a cellular network.
     */
    BEARER_CELLULAR = 0,

    /**
     * Indicates that the network is based on a Wi-Fi network.
     */
    BEARER_WIFI = 1,

    /**
     * Indicates that the network is based on a Bluetooth network.
     */
    BEARER_BLUETOOTH = 2,

    /**
     * Indicates that the network is an Ethernet network.
     */
    BEARER_ETHERNET = 3,

    /**
     * Indicates that the network is a VPN.
     */
    BEARER_VPN = 4,

    /**
     * Indicates that the network is a Wi-Fi Aware network.
     */
    BEARER_WIFI_AWARE = 5,

    /**
     * Indicates that the network is a LoWPAN network.
     */
    BEARER_LOWPAN = 6
  }

  export interface ConnectionProperties {
    interfaceName: string;
    isUsePrivateDns: boolean;
    privateDnsServerName: string;
    domains: string;
    httpProxy: HttpProxy;
    linkAddresses: Array<LinkAddress>;
    dnses: Array<NetAddress>;
    routes: Array<RouteInfo>;
    mtu: number;
  }

  export interface HttpProxy {
    host: string;
    port: number;
    parsedExclusionList: Array<string>;
  }

  export interface RouteInfo {
    interface: string;
    destination: LinkAddress;
    gateway: NetAddress;
    hasGateway: boolean;
    isDefaultRoute: boolean;
  }

  export interface LinkAddress {
    address: NetAddress;
    prefixLength: number;
  }

  /**
   * @since 7
   */
  export interface NetAddress {
    address: string;
    family?: number; // IPv4 = 1; IPv6 = 2, default is IPv4
    port?: number; // [0, 65535]
  }

  export interface NetProxy {
    type: ProxyType;
    address: NetAddress;
  }

  export enum ProxyType {
    /**
     * Represents proxy for high level protocols such as HTTP or FTP.
     */
    HTTP,
    /**
     * Represents a SOCKS (V4 or V5) proxy.
     */
    SOCKS
  }

  export enum BackgroundPolicy {
    /**
     * Indicates that applications can use metered networks.
     */
    BACKGROUND_POLICY_DISABLE = 1,

    /**
     * Indicates that only applications in the allowlist can use metered networks.
     */
    BACKGROUND_POLICY_ALLOWLISTED = 2,

    /**
     * Indicates that applications cannot use metered networks.
     */
    BACKGROUND_POLICY_ENABLED = 3
  }
}

export default connection;