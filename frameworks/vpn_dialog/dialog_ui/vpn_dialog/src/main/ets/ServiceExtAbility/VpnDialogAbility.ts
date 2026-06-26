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
import osAccount from '@ohos.account.osAccount';
import rpc from '@ohos.rpc';
import type Want from '@ohos.app.ability.Want';
import appManager from '@ohos.app.ability.appManager';
import bundleManager from '@ohos.bundle.bundleManager';
import bundleResourceManager from '@ohos.bundle.bundleResourceManager';
import dataShare from '@ohos.data.dataShare';
import dataSharePredicates from '@ohos.data.dataSharePredicates';
import GlobalContext from '../MainAbility/GlobalContext';
import display from '@ohos.display';

class VpnDialogStub extends rpc.RemoteObject {
  constructor(des) {
    super(des);
  }
  onRemoteRequest(code, data, reply, option): boolean {
    // reject all requests since it is a stub
    return false;
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
  private windowNum: number = 0;
  /**
   * Lifecycle function, called back when a service extension is started for initialization.
   */
  onCreate(want: Want): void {
    console.info('onCreate is triggered');
    if (want && want.parameters) {
      console.info('onCreate is triggered');
      globalThis.vpnCallingPid = want.parameters['ohos.aafwk.param.callerPid'];
      globalThis.vpnCallingUid = want.parameters['ohos.aafwk.param.callerUid'];
      try {
        const bundleName = bundleManager.getBundleNameByUidSync(globalThis.vpnCallingUid);
        globalThis.bundleName = bundleName;
        const bundleResourceInfo = bundleResourceManager.getBundleResourceInfo(bundleName,
          bundleResourceManager.ResourceFlag.GET_RESOURCE_INFO_WITH_LABEL);
        globalThis.vpnCallingAppName = bundleResourceInfo.label;
      } catch (err) {
        console.error('get bundle info failed!' + JSON.stringify(err));
      }
    }
    this.windowNum = 0;
  }

  onConnect(want: Want): rpc.RemoteObject {
    console.log('onConnect is triggered');
  
    globalThis.extensionContext = this.context;
    const stub = new VpnDialogStub('VpnRightDialog');
    if (want?.parameters) {
 	    globalThis.parameters = want.parameters;
 	  }
    if (want?.parameters?.abilityName) {
      let abilityName: string = want.parameters.abilityName.toString();
      globalThis.abilityName = abilityName;
    }
    if (want?.parameters?.bundleName) {
      if (want.parameters.bundleName !== globalThis.bundleName) {
        console.error('Not allowed to start other bundleName Vpn!');
        return stub;
      }
    } else {
      console.error('bundleName is not set!');
      return stub;
    }

    this.checkPermission().then(hasPermission => {
      if (hasPermission) {
        console.info('No need to get permission again: ' + globalThis.bundleName);
        return;
      }
      appManager.getForegroundApplications().then(data => {
        if (data &&data.some(item => item.uid === globalThis.vpnCallingUid)) {
          console.info('Current calling uid is in foreground.');
          this.showDialog();
        } else {
          console.error('App is not in foreground, not allowed to show dialog!');
          return;
        }
      }).catch(err => {
        console.error(`getForegroundApplications failed! error code: ${err.code}, error msg: ${err.message}`);
      })
    });
    return stub;
  }

  private async queryValue(dataShareHelper: dataShare.DataShareHelper, uri: string, keyword: string)
      : Promise<string> {
    const da = new dataSharePredicates.DataSharePredicates();
    da.equalTo('KEYWORD', keyword);
    try {
      const resultSet = await dataShareHelper.query(uri, da, ['*']);
      if (!resultSet || !resultSet.goToFirstRow()) {
        console.error('goToFirstRow failed!');
        return '';
      }
      const columnIndex = resultSet.getColumnIndex('VALUE');
      return resultSet.getString(columnIndex);
    } catch (err) {
      throw err;
    }
  }

  private async checkPermission(): Promise<boolean> {
    try {
      const uri = 'datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=vpnext_mode';
      const dataShareHelper = await dataShare.createDataShareHelper(this.context, uri);
      if (!dataShareHelper) {
        console.error('createDataShareHelper failed!');
        return false;
      }
      const accountManager = osAccount.getAccountManager();
      const localId = accountManager.getOsAccountLocalIdForUidSync(globalThis.vpnCallingUid);
      let result = await this.queryValue(dataShareHelper, uri, globalThis.bundleName + '_' + localId);
      if (result === '1')
        return true;
      result = await this.queryValue(dataShareHelper, uri, globalThis.bundleName);
      return result === '1';
    } catch (err) {
      console.error('checkPermission failed!');
      return false;
    }
  }

  private showDialog() : void {
    let dis;
    try {
      dis = display.getDefaultDisplaySync();
    } catch (err) {
      console.error('getDefaultDisplaySync failed!');
    }
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
    try {
      display.off('foldStatusChange');
    } catch (err) {
      console.error('VpnDialogAbility display off foldStatusChange failed!');
    }
  }

  onfoldStatusChange(): void {
    display.on('foldStatusChange', () => {
      let foldStatus = display.getFoldStatus();
      if(foldStatus === 2 || foldStatus === 11 || foldStatus === 1){
        setTimeout(() => {
          let dis = display.getDefaultDisplaySync();
          this.vpnWin.resize(dis.width, dis.height);
        }, 70);
      }
    });
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
      this.onfoldStatusChange();
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

