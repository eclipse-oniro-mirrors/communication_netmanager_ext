/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import type common from '@ohos.app.ability.common';
import rpc from '@ohos.rpc';
import type Want from '@ohos.app.ability.Want';
import GlobalContext from '../MainAbility/GlobalContext';
import display from '@ohos.display';

class VpnDialogStub extends rpc.RemoteObject {
  constructor(des) {
    super(des);
  }
  onRemoteRequest(code, data, reply, option): boolean {
    return true;
  }
}

const BG_COLOR = '#fffcf9f9';

interface NavigationBarRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

export default class VpnDialogAbility extends extension {
  private vpnWin: window.Window | undefined = undefined;
  // private mContext: common.ServiceExtensionContext | undefined = undefined;
  private windowNum: number = 0;
  /**
   * Lifecycle function, called back when a service extension is started for initialization.
   */
  onCreate(want: Want): void {
    console.log('onCreate want: ' + JSON.stringify(want));
    this.windowNum = 0;
  }

  onConnect(want: Want): rpc.RemoteObject {
    console.log('onConnect want : ' + JSON.stringify(want));
  
    globalThis.extensionContext = this.context;

    let abilityName: string = want.parameters.abilityName.toString();
    let bundleName: string = want.parameters.bundleName.toString();
    globalThis.abilityName = abilityName;
    globalThis.bundleName = bundleName;

    let dis = display.getDefaultDisplaySync();
    let navigationBarRect: NavigationBarRect = {
      left: 0,
      top: 0,
      width: dis.width,
      height: dis.height
    };
    let windowConfig: window.Configuration = {
      name: 'Vpn Dialog',
      windowType: window.WindowType.TYPE_FLOAT,
      ctx: this.context
    };
    this.createWindow(windowConfig, navigationBarRect);

    return new VpnDialogStub('VpnRightDialog');
  }

  onDisconnect(want: Want): void {
    console.log('onDisconnect');
  }

  /**
   * Lifecycle function, called back when a service extension is started or recall.
   */
  onRequest(want: Want, startId: number): void {
    console.log('onRequest');
  }
  /**
   * Lifecycle function, called back before a service extension is destroyed.
   */
  onDestroy(): void {
    console.info('VpnDialogAbility onDestroy.');
  }

  private async createWindow(config: window.Configuration, rect: NavigationBarRect): Promise<void> {
    console.log('create windows execute');
    try {
      this.vpnWin = await window.createWindow(config);
      GlobalContext.getContext().setObject('vpnWin', this.vpnWin);
      globalThis.vpnWin = this.vpnWin;
      await this.vpnWin.resize(rect.width, rect.height);
      await this.vpnWin.setUIContent('pages/VpnDialog');
      await this.vpnWin.setWindowBackgroundColor('#00000000');
      await this.vpnWin.showWindow();
      try {
        await this.vpnWin.hideNonSystemFloatingWindows(true);
      } catch (err) {
        console.error('window hideNonSystemFloatingWindows failed!');
      }
      console.log('VpnDialogAbility window create successfully');
    } catch (exception) {
      console.error('VpnDialogAbility Failed to create the window. Cause: ' + JSON.stringify(exception));
    }
  }
};

