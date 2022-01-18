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

  /**
   * Set the specified network interface parameters.
   *
   * @param iface Indicates the network interface name of the network parameter.
   * @param ic Indicates the ic. See {@link InterfaceConfiguration}.   
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function setIfaceConfig(iface: string, ic: InterfaceConfiguration, callback: AsyncCallback<void>):void
  function setIfaceConfig(iface: string, ic: InterfaceConfiguration): Promise<void>
  
   /**
   * Get the specified network interface information.
   *
   * @param iface Indicates the network interface name.
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function getIfaceConfig(iface: string, callback: AsyncCallback<InterfaceConfiguration>):void
  function getIfaceConfig(iface: string): Promise<InterfaceConfiguration>
  
  /**
   * Gets whether the specified network is activated.
   *
   * @param iface Indicates the network interface name.
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function isIfaceActive(iface: string, callback: AsyncCallback<number\>\): void
  function isIfaceActive(iface: string): Promise<number>
  /**
   * Gets the names of all active network interfaces.
   *
   * @pemission ohos.permission.GET_NETWORK_INFO and ohos.permission.INTERNET
   */
  function getAllActiveIfaces(callback: AsyncCallback<Array<string>>): void
  function getAllActiveIfaces(): Promise<Array<string>>
 
  export interface InterfaceConfiguration {
	mode: IPSetMode;  /**static or DHCP**/
	ipAddr: string;
	route: string;
	gate: string;
	netMask: string;
	dnsServers: string;
	domain: string;
  }
  export enum IPSetMode {
	STATIC = 0,  /****/
	DHCP = 1 
  }

}

export default ethernet;