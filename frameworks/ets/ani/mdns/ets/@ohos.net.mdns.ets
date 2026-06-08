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
import type Context from 'application.Context';
import { BusinessError } from '@ohos.base';

export default namespace mdns {
  type NetAddress = connection.NetAddress;

  loadLibrary("mdns_ani");

  function toBusinessError(err: Error): BusinessError {
    let businessErr = new BusinessError();
    businessErr.code = err.code ?? 0;
    businessErr.message = err.message ?? '';
    businessErr.stack = err.stack ?? '';
    businessErr.cause = err;
    return businessErr;
  }

  export interface ServiceAttribute {
    key: string;
    value: Array<int>;
  }

  export interface LocalServiceInfo {
    serviceType: string;
    serviceName: string;
    port?: int;
    host?: NetAddress;
    serviceAttribute?: Array<ServiceAttribute>;
  }

  export interface DiscoveryEventInfo {
    serviceInfo: LocalServiceInfo;
    errorCode?: MdnsError;
  }

  export enum MdnsError {
    INTERNAL_ERROR = 0,
    ALREADY_ACTIVE = 1,
    MAX_LIMIT = 2,
  }

  // ---------- addLocalService ----------
  export native function addLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function addLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    taskpool.execute(() => {
      return addLocalServiceSync(context, serviceInfo);
    }).then((content: Any) => {
      callback(null, content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(toBusinessError(err), null!);
    });
  }

  export function addLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return addLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- removeLocalService ----------
  export native function removeLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function removeLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    taskpool.execute(() => {
      return removeLocalServiceSync(context, serviceInfo);
    }).then((content: Any) => {
      callback(null, content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(toBusinessError(err), null!);
    });
  }

  export function removeLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return removeLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- resolveLocalService ----------
  export native function resolveLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function resolveLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    taskpool.execute(() => {
      return resolveLocalServiceSync(context, serviceInfo);
    }).then((content: Any) => {
      callback(null, content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(toBusinessError(err), null!);
    });
  }

  export function resolveLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return resolveLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- DiscoveryService ----------
  export class DiscoveryService {
    private serviceType: string;

    constructor(serviceType: string) {
      this.serviceType = serviceType;
    }

    // The on/off type parameter acts as a discriminated union; the if-else chain ensures
    // each branch dispatches to the correctly-typed native function.
    on(type: 'discoveryStart' | 'discoveryStop' | 'serviceFound' | 'serviceLost', callback: Callback<Any>): void {
      if (type == 'discoveryStart') {
        onDiscoveryStart(callback as Callback<DiscoveryEventInfo>);
      } else if (type == 'discoveryStop') {
        onDiscoveryStop(callback as Callback<DiscoveryEventInfo>);
      } else if (type == 'serviceFound') {
        onServiceFound(callback as Callback<LocalServiceInfo>);
      } else if (type == 'serviceLost') {
        onServiceLost(callback as Callback<LocalServiceInfo>);
      }
    }

    off(type: 'discoveryStart' | 'discoveryStop' | 'serviceFound' | 'serviceLost', callback?: Callback<Any>): void {
      if (type == 'discoveryStart') {
        offDiscoveryStart(callback as Callback<DiscoveryEventInfo>);
      } else if (type == 'discoveryStop') {
        offDiscoveryStop(callback as Callback<DiscoveryEventInfo>);
      } else if (type == 'serviceFound') {
        offServiceFound(callback as Callback<LocalServiceInfo>);
      } else if (type == 'serviceLost') {
        offServiceLost(callback as Callback<LocalServiceInfo>);
      }
    }

    startSearchingMDNS(): void {
      try {
        startSearchingMDNSSync(this.serviceType);
      } catch (err) {
        throw toBusinessError(err);
      }
    }

    stopSearchingMDNS(): void {
      try {
        stopSearchingMDNSSync(this.serviceType);
      } catch (err) {
        throw toBusinessError(err);
      }
    }
  }

  // ---------- createDiscoveryService ----------
  export function createDiscoveryService(context: Context, serviceType: string): DiscoveryService {
    let result = new DiscoveryService(serviceType);
    return result;
  }

  // ---------- DiscoveryService native helpers ----------
  export native function startSearchingMDNSSync(serviceType: string): void;
  export native function stopSearchingMDNSSync(serviceType: string): void;

  export native function onDiscoveryStart(callback: Callback<DiscoveryEventInfo>): void;
  export native function offDiscoveryStart(callback?: Callback<DiscoveryEventInfo>): void;
  export native function onDiscoveryStop(callback: Callback<DiscoveryEventInfo>): void;
  export native function offDiscoveryStop(callback?: Callback<DiscoveryEventInfo>): void;
  export native function onServiceFound(callback: Callback<LocalServiceInfo>): void;
  export native function offServiceFound(callback?: Callback<LocalServiceInfo>): void;
  export native function onServiceLost(callback: Callback<LocalServiceInfo>): void;
  export native function offServiceLost(callback?: Callback<LocalServiceInfo>): void;
}
