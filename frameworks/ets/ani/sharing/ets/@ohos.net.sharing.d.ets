/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

export default namespace sharing {
  type NetHandle = connection.NetHandle;

  loadLibrary("sharing_ani")

  export native function isSharingSupportedSync():boolean;

  export function isSharingSupported(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
        return isSharingSupportedSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
        callback(err as BusinessError, false);
    });
  }

  export function isSharingSupported(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
          taskpool.execute((): boolean => {
              return isSharingSupportedSync();
          }).then((content: NullishType) => {
              resolve(content as boolean);
          }, (err: Error): void => {
              reject(err as BusinessError);
          });
      });
  }

  export native function isSharingSync():boolean;

  export function isSharing(callback: AsyncCallback<boolean>): void {
    let p1 = taskpool.execute((): boolean => {
        return isSharingSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as boolean);
    }, (err: Error): void => {
        callback(err as BusinessError, false);
    });
  }

  export function isSharing(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
          taskpool.execute((): boolean => {
              return isSharingSync();
          }).then((content: NullishType) => {
              resolve(content as boolean);
          }, (err: Error): void => {
              reject(err as BusinessError);
          });
      });
  }

  export native function startSharingSync(type: SharingIfaceType): void;

  export function startSharing(type: SharingIfaceType, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
       return startSharingSync(type);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }

  export function startSharing(type: SharingIfaceType): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
          return startSharingSync(type);
        }).then(() => {
            resolve(undefined);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export native function stopSharingSync(type: SharingIfaceType): void;

  export function stopSharing(type: SharingIfaceType, callback: AsyncCallback<void>): void {
    let p1 = taskpool.execute((): undefined => {
       return stopSharingSync(type);
    })
    p1.then(() => {
        callback(new BusinessError(), undefined);
    }, (err: Error): void => {
        callback(err as BusinessError, undefined);
    });
  }

  export function stopSharing(type: SharingIfaceType): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute((): undefined => {
          return stopSharingSync(type);
        }).then(() => {
            resolve(undefined);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export native function getStatsRxBytesSync():long;

  export function getStatsRxBytes(callback: AsyncCallback<long>): void {
    let p1 = taskpool.execute((): long => {
        return getStatsRxBytesSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as long);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }

  export function getStatsRxBytes(): Promise<long> {
    return new Promise<long>((resolve, reject) => {
        taskpool.execute((): long => {
            return getStatsRxBytesSync();
        }).then((content: NullishType) => {
            resolve(content as long);
        }, (err: Error): void => {
            reject(err as BusinessError);
        });
    });
  }

  export native function getStatsTxBytesSync():long;

  function getStatsTxBytes(callback: AsyncCallback<number>): void {
    let p1 = taskpool.execute((): long => {
        return getStatsTxBytesSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as long);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }

  function getStatsTxBytes(): Promise<long> {
    return new Promise<long>((resolve, reject) => {
      taskpool.execute((): long => {
          return getStatsTxBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export native function getStatsTotalBytesSync():long;

  export function getStatsTotalBytes(callback: AsyncCallback<number>): void {
    let p1 = taskpool.execute((): long => {
        return getStatsTotalBytesSync();
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as long);
    }, (err: Error): void => {
        callback(err as BusinessError, 0);
    });
  }

  export function getStatsTotalBytes(): Promise<long> {
    return new Promise<long>((resolve, reject) => {
      taskpool.execute((): long => {
          return getStatsTotalBytesSync();
      }).then((content: NullishType) => {
          resolve(content as long);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export native function getSharingIfacesSync(state: SharingIfaceState): Array<string>;

  export function getSharingIfaces(state: SharingIfaceState, callback: AsyncCallback<Array<string>>): void {
      let p1 = taskpool.execute((): Array<string> => {
        return getSharingIfacesSync(state);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as Array<string>);
    }, (err: Error): void => {
        callback(err as BusinessError, new Array<string>());
    });
  }

  export function getSharingIfaces(state: SharingIfaceState): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
          return getSharingIfacesSync(state);
      }).then((content: NullishType) => {
          resolve(content as Array<string>);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export native function getSharingStateSync(type: SharingIfaceType): SharingIfaceState;

  export function getSharingState(type: SharingIfaceType, callback: AsyncCallback<SharingIfaceState>): void {
    let p1 = taskpool.execute((): SharingIfaceState => {
        return getSharingStateSync(type);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as SharingIfaceState);
    }, (err: Error): void => {
        callback(err as BusinessError, SharingIfaceState.SHARING_NIC_SERVING);
    });
  }

  export function getSharingState(type: SharingIfaceType): Promise<SharingIfaceState> {
    return new Promise<SharingIfaceState>((resolve, reject) => {
      taskpool.execute((): SharingIfaceState => {
          return getSharingStateSync(type);
      }).then((content: NullishType) => {
          resolve(content as SharingIfaceState);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export native function getSharableRegexesSync(type: SharingIfaceType): Array<string>;

  export function getSharableRegexes(type: SharingIfaceType, callback: AsyncCallback<Array<string>>): void {
    let p1 = taskpool.execute((): Array<string> => {
        return getSharableRegexesSync(type);
    })
    p1.then((content: NullishType) => {
        callback(new BusinessError(), content as Array<string>);
    }, (err: Error): void => {
        callback(err as BusinessError, new Array<string>());
    });
  }

  export function getSharableRegexes(type: SharingIfaceType): Promise<Array<string>> {
    return new Promise<Array<string>>((resolve, reject) => {
      taskpool.execute((): Array<string> => {
          return getSharableRegexesSync(type);
      }).then((content: NullishType) => {
          resolve(content as Array<string>);
      }, (err: Error): void => {
          reject(err as BusinessError);
      });
    });
  }

  export function on(type: 'sharingStateChange' | 'interfaceSharingStateChange' | 'sharingUpstreamChange', callback: Object): void {
    if (type == 'sharingStateChange') {
      onSharingStateChange(callback as Callback<boolean>);
    } else if (type == 'interfaceSharingStateChange') {
      onInterfaceSharingStateChange(callback as Callback<InterfaceSharingStateInfo>)
    } else if (type == 'sharingUpstreamChange') {
      onSharingUpstreamChange(callback as Callback<NetHandle>);
    }
  }

  export function off(type: 'sharingStateChange' | 'interfaceSharingStateChange' | 'sharingUpstreamChange', callback?: Object): void {
    if (type == 'sharingStateChange') {
      offSharingStateChange(callback as Callback<boolean> | undefined);
    } else if (type == 'interfaceSharingStateChange') {
      offInterfaceSharingStateChange(callback as Callback<InterfaceSharingStateInfo> | undefined)
    } else if (type == 'sharingUpstreamChange') {
      offSharingUpstreamChange(callback as Callback<NetHandle> | undefined);
    }
  }

  export native function onSharingStateChange(callback: Callback<boolean>): void;

  export native function offSharingStateChange(callback?: Callback<boolean>): void;

  export native function onInterfaceSharingStateChange(callback: Callback<InterfaceSharingStateInfo>): void;

  export native function offInterfaceSharingStateChange(callback?: Callback<InterfaceSharingStateInfo>): void;

  export native function onSharingUpstreamChange(callback: Callback<NetHandle>): void;

  export native function offSharingUpstreamChange(callback?: Callback<NetHandle>): void;

   export enum SharingIfaceState {
    SHARING_NIC_SERVING = 1,

    SHARING_NIC_CAN_SERVER = 2,

    SHARING_NIC_ERROR = 3
    }
  export interface InterfaceSharingStateInfo {
     type: SharingIfaceType;
     iface: string;
     state: SharingIfaceState;
   }

   export enum SharingIfaceType {
     SHARING_WIFI = 0,

     SHARING_USB = 1,

     SHARING_BLUETOOTH = 2
   }
}
