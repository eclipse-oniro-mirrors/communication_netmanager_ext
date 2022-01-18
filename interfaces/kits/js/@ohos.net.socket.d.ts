/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback, ErrorCallback} from "./basic";
import connection from "./@ohos.net.connection";

/**
 * Provides TCP and UDP Socket APIs.
 *
 * @since 6
 */
declare namespace socket {
  export import NetAddress = connection.NetAddress;

  function createUDPSocket(): UDPSocket;
  function createTCPSocket(): TCPSocket;

  export interface UDPSocket {
    bind(address: NetAddress, callback: AsyncCallback<void>): void;
    bind(address: NetAddress): Promise<void>;

    connect(options: UDPConnectOptions, callback: AsyncCallback<void>): void;
    connect(options: UDPConnectOptions): Promise<void>;

    send(options: UDPSendOptions, callback: AsyncCallback<void>): void;
    send(options: UDPSendOptions): Promise<void>;

    close(callback: AsyncCallback<void>): void;
    close(): Promise<void>;

    getRemoteAddress(callback: AsyncCallback<NetAddress>): void;
    getRemoteAddress(): Promise<NetAddress>;

    getState(callback: AsyncCallback<SocketStateBase>): void;
    getState(): Promise<SocketStateBase>;

    setExtraOptions(options: UDPExtraOptions, callback: AsyncCallback<void>): void;
    setExtraOptions(options: UDPExtraOptions): Promise<void>;

    on(type: 'message', callback: AsyncCallback<{message: ArrayBuffer, remoteInfo: SocketRemoteInfo}>): void;
    off(type: 'message', callback?: AsyncCallback<{message: ArrayBuffer, remoteInfo: SocketRemoteInfo}>): void;

    on(type: 'listening' | 'connect' | 'close', callback: AsyncCallback<void>): void;
    off(type: 'listening' | 'connect ' | 'close', callback?: AsyncCallback<void>): void;

    on(type: 'error', callback: ErrorCallback): void;
    off(type: 'error', callback?: ErrorCallback): void;
  }

  export interface UDPConnectOptions {
    // connect(InetAddress address, int port) --- Connects the socket to a remote address for this socket.
    // connect(SocketAddress addr) --- Connects this socket to a remote socket address (IP address + port number).
    address: NetAddress;
  }

  export interface UDPSendOptions {
    data: string | ArrayBuffer;
  }

  export interface ExtraOptionsBase {
    receiveBufferSize?: number;
    sendBufferSize?: number;
    reuseAddress?: number;
    socketTimeout?: number;
  }

  export interface UDPExtraOptions extends ExtraOptionsBase {
    broadcast?: boolean;
  }

  export interface SocketStateBase {
    isBound: boolean;
    isClose: boolean;
    isConnected: boolean;
  }

  export interface SocketRemoteInfo {
    address: string;
    family: 'IPv4' | 'IPv6';
    port: number;
    size: number;
  }

  export interface TCPSocket {
    bind(address: NetAddress, callback: AsyncCallback<void>): void;
    bind(address: NetAddress): Promise<void>;

    connect(options: TCPConnectOptions, callback: AsyncCallback<void>): void;
    connect(options: TCPConnectOptions): Promise<void>;

    send(options: TCPSendOptions, callback: AsyncCallback<boolean>): void;
    send(options: TCPSendOptions): Promise<boolean>;

    close(callback: AsyncCallback<void>): void;
    close(): Promise<void>;

    getRemoteAddress(callback: AsyncCallback<NetAddress>): void;
    getRemoteAddress(): Promise<NetAddress>;

    getState(callback: AsyncCallback<SocketStateBase>): void;
    getState(): Promise<SocketStateBase>;

    setExtraOptions(options: TCPExtraOptions, callback: AsyncCallback<void>): void;
    setExtraOptions(options: TCPExtraOptions): Promise<void>;

    on(type: 'message', callback: AsyncCallback<{message: ArrayBuffer, remoteInfo: SocketRemoteInfo}>): void;
    off(type: 'message', callback?: AsyncCallback<{message: ArrayBuffer, remoteInfo: SocketRemoteInfo}>): void;

    on(type: 'connect' | 'close', callback: AsyncCallback<void>): void;
    off(type: 'connect ' | 'close', callback?: AsyncCallback<void>): void;

    on(type: 'error', callback: ErrorCallback): void;
    off(type: 'error', callback?: ErrorCallback): void;
  }

  export interface TCPConnectOptions {
    address: NetAddress;
    timeout?: number;
  }

  export interface TCPSendOptions {
    data: string | ArrayBuffer;
    encoding?: string; // Only used when data is string. Default is 'utf8'
  }

  export interface TCPExtraOptions extends ExtraOptionsBase {
    keepAlive?: boolean;
    OOBInline?: boolean;
    TCPNoDelay?: boolean;
    socketLinger: {on: boolean, linger: number};
    performancePreferences: {connectionTime: number, latency: number, bandwidth: number};
  }
}

export default socket;