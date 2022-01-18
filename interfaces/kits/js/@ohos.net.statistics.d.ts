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

import {AsyncCallback} from "./basic";

/**
 * Obtains traffic statistics.
 *
 * @since 8
 * @sysCap SystemCapability.Communication.NetManager
 * @devices phone, tablet, tv, wearable, car
 */
declare namespace statistics {
  /**
   * Queries the data traffic (including all TCP and UDP data packets) received through a specified NIC.
   *
   * @param nic Indicates the NIC name.
   * @param callback Returns the data traffic received through the specified NIC.
   */
  function getIfaceRxBytes(nic: string, callback: AsyncCallback<number>): void;
  function getIfaceRxBytes(nic: string): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) sent through a specified NIC.
   *
   * @param nic Indicates the NIC name.
   * @param callback Returns the data traffic sent through the specified NIC.
   */
  function getIfaceTxBytes(nic: string, callback: AsyncCallback<number>): void;
  function getIfaceTxBytes(nic: string): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) received through the cellular network.
   *
   * @param callback Returns the data traffic received through the cellular network.
   */
  function getCellularRxBytes(callback: AsyncCallback<number>): void;
  function getCellularRxBytes(): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) sent through the cellular network.
   *
   * @param callback Returns the data traffic sent through the cellular network.
   */
  function getCellularTxBytes(callback: AsyncCallback<number>): void;
  function getCellularTxBytes(): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) sent through all NICs.
   *
   * @param callback Returns the data traffic sent through all NICs.
   */
  function getAllTxBytes(callback: AsyncCallback<number>): void;
  function getAllTxBytes(): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) received through all NICs.
   *
   * @param callback Returns the data traffic received through all NICs.
   */
  function getAllRxBytes(callback: AsyncCallback<number>): void;
  function getAllRxBytes(): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) received by a specified application.
   * This method applies only to system applications and your own applications.
   *
   * @param uid Indicates the process ID of the application.
   * @param callback Returns the data traffic received by the specified application.
   */
  function getUidRxBytes(uid: number, callback: AsyncCallback<number>): void;
  function getUidRxBytes(uid: number): Promise<number>;

  /**
   * Queries the data traffic (including all TCP and UDP data packets) sent by a specified application.
   * This method applies only to system applications and your own applications.
   *
   * @param uid Indicates the process ID of the application.
   * @param callback Returns the data traffic sent by the specified application.
   */
  function getUidTxBytes(uid: number, callback: AsyncCallback<number>): void;
  function getUidTxBytes(uid: number): Promise<number>;
}

export default statistics;