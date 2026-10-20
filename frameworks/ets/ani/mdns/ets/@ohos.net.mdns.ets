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
  loadLibrary("mdns_ani");

  export class Cleaner {
    private ptr: long = 0
    constructor(ptr: long) {
        this.ptr = ptr
    }
    native clean(): void
  }

  let destroyRegister: FinalizationRegistry<Cleaner> = new FinalizationRegistry<Cleaner>((cleaner: Cleaner) => {cleaner.clean()})
  let unregisterToken: object = new object()

  type NetAddress = connection.NetAddress;

  export interface ServiceAttribute {
    key: string;
    value: Array<int>;
  }

  class ServiceAttributeInner implements ServiceAttribute {
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

  class LocalServiceInfoInner implements LocalServiceInfo {
    serviceType: string;
    serviceName: string;
    port?: int;
    host?: NetAddress;
    serviceAttribute?: Array<ServiceAttribute>;
  }

  export enum MdnsError {
    INTERNAL_ERROR = 0,
    ALREADY_ACTIVE = 1,
    MAX_LIMIT = 2,
  }

  export interface DiscoveryEventInfo {
    serviceInfo: LocalServiceInfo;
    errorCode?: MdnsError;
  }

  class DiscoveryEventInfoInner implements DiscoveryEventInfo {
    serviceInfo: LocalServiceInfo = new LocalServiceInfoInner();
    errorCode?: MdnsError;
  }

  export native function addLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function addLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    let p1 = taskpool.execute((): LocalServiceInfo => {
      return addLocalServiceSync(context, serviceInfo);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(err as BusinessError, new LocalServiceInfoInner());
    });
  }

  export function addLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return addLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export native function removeLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function removeLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    let p1 = taskpool.execute((): LocalServiceInfo => {
      return removeLocalServiceSync(context, serviceInfo);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(err as BusinessError, new LocalServiceInfoInner());
    });
  }

  export function removeLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return removeLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export native function createDiscoveryServiceSync(context: Context, serviceType: string): DiscoveryService;

  export function createDiscoveryService(context: Context, serviceType: string): DiscoveryService {
    return createDiscoveryServiceSync(context, serviceType);
  }

  export native function resolveLocalServiceSync(context: Context, serviceInfo: LocalServiceInfo): LocalServiceInfo;

  export function resolveLocalService(context: Context, serviceInfo: LocalServiceInfo,
    callback: AsyncCallback<LocalServiceInfo>): void {
    let p1 = taskpool.execute((): LocalServiceInfo => {
      return resolveLocalServiceSync(context, serviceInfo);
    })
    p1.then((content: Any) => {
      callback(new BusinessError(), content as LocalServiceInfo);
    }, (err: Error): void => {
      callback(err as BusinessError, new LocalServiceInfoInner());
    });
  }

  export function resolveLocalService(context: Context, serviceInfo: LocalServiceInfo): Promise<LocalServiceInfo> {
    return new Promise<LocalServiceInfo>((resolve, reject) => {
        taskpool.execute(() => {
            return resolveLocalServiceSync(context, serviceInfo);
        }).then((content: Any) => {
            resolve(content as LocalServiceInfo);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export interface DiscoveryService {
    onDiscoveryStart(callback: Callback<DiscoveryEventInfo>): void;
    offDiscoveryStart(callback?: Callback<DiscoveryEventInfo>): void;
    onDiscoveryStop(callback: Callback<DiscoveryEventInfo>): void;
    offDiscoveryStop(callback?: Callback<DiscoveryEventInfo>): void;
    onServiceFound(callback: Callback<LocalServiceInfo>): void;
    offServiceFound(callback?: Callback<LocalServiceInfo>): void;
    onServiceLost(callback: Callback<LocalServiceInfo>): void;
    offServiceLost(callback?: Callback<LocalServiceInfo>): void;
    startSearchingMDNS(): void;
    stopSearchingMDNS(): void;
  }

  export class DiscoveryServiceInner implements DiscoveryService {
    private nativePtr: long = 0;
    private cleaner: Cleaner | null = null;

    constructor(context: long) {
        if (this.nativePtr == 0) {
            this.nativePtr = context;
        }
        this.registerCleaner(this.nativePtr);
    }

    registerCleaner(ptr: long): void {
        this.cleaner = new Cleaner(ptr);
        destroyRegister.register(this, this.cleaner!, unregisterToken);
    }
    unregisterCleaner(): void {
        destroyRegister.unregister(unregisterToken);
    }

    native onDiscoveryStartSync(callback: Callback<DiscoveryEventInfo>): void;
    native offDiscoveryStartSync(callback?: Callback<DiscoveryEventInfo>): void;
    native onDiscoveryStopSync(callback: Callback<DiscoveryEventInfo>): void;
    native offDiscoveryStopSync(callback?: Callback<DiscoveryEventInfo>): void;
    native onServiceFoundSync(callback: Callback<LocalServiceInfo>): void;
    native offServiceFoundSync(callback?: Callback<LocalServiceInfo>): void;
    native onServiceLostSync(callback: Callback<LocalServiceInfo>): void;
    native offServiceLostSync(callback?: Callback<LocalServiceInfo>): void;
    native startSearchingMDNSSync(): void;
    native stopSearchingMDNSSync(): void;

    onDiscoveryStart(callback: Callback<DiscoveryEventInfo>): void {
      this.onDiscoveryStartSync(callback);
    }

    offDiscoveryStart(callback?: Callback<DiscoveryEventInfo>): void {
      if (callback == undefined) {
        this.offDiscoveryStartSync();
      } else {
        this.offDiscoveryStartSync(callback);
      }
    }

    onDiscoveryStop(callback: Callback<DiscoveryEventInfo>): void {
      this.onDiscoveryStopSync(callback);
    }

    offDiscoveryStop(callback?: Callback<DiscoveryEventInfo>): void {
      if (callback == undefined) {
        this.offDiscoveryStopSync();
      } else {
        this.offDiscoveryStopSync(callback);
      }
    }

    onServiceFound(callback: Callback<LocalServiceInfo>): void {
      this.onServiceFoundSync(callback);
    }

    offServiceFound(callback?: Callback<LocalServiceInfo>): void {
      if (callback == undefined) {
        this.offServiceFoundSync();
      } else {
        this.offServiceFoundSync(callback);
      }
    }

    onServiceLost(callback: Callback<LocalServiceInfo>): void {
      this.onServiceLostSync(callback);
    }

    offServiceLost(callback?: Callback<LocalServiceInfo>): void {
      if (callback == undefined) {
        this.offServiceLostSync();
      } else {
        this.offServiceLostSync(callback);
      }
    }

    startSearchingMDNS(): void {
      return this.startSearchingMDNSSync();
    }

    stopSearchingMDNS(): void {
      return this.stopSearchingMDNSSync();
    }
  }

}
