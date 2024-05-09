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

import usbManager from '@ohos.usbManager';
import featureAbility from '@ohos.ability.featureAbility';
import common from '@ohos.app.ability.common';
import window from '@ohos.window';
import vpnext from '@ohos.net.vpnExtension';
import Want from '@ohos.app.ability.Want';
import GlobalContext from '../MainAbility/GlobalContext.ts';

// allow,notAllow button style
@Extend(Text)
function controlBtn(fontColor : string) {
  .fontColor(fontColor)
  .backgroundColor(0xffffff)
  .width(100)
  .height(56)
  .fontSize(20)
  .textAlign(TextAlign.Center);
}

@Extend(Text)
function controlText() {
    .backgroundColor(0xffffff)
    .height(56)
    .fontSize(20)
    .textAlign(TextAlign.Center);
}

// content style
@Extend(Text)
function controlFontSize(fontSize : number) {
  .fontSize(fontSize)
  .width('180%')
  .fontColor('rgba(10, 10, 10, 1.00)')
  .textAlign(TextAlign.Center)
  .padding({ top: 30, left: 50, right: 50 })
  .alignSelf(ItemAlign.Center)
  .margin({ top: 10 });
}

@CustomDialog
export default struct ConfirmDialog {
  content: string = '';
  notAllowFontColor: string = '#E84026';
  allowFontColor: string = '#ff1a0af7';
  controller: CustomDialogController;
  private extensionWin: window.Window = null;
  private extensionContext: common.ServiceExtensionContext = null;
  private context: common.UIAbilityContext = getContext(this) as common.UIAbilityContext;

  // title style
  @Builder
  TipTextStyle(tip: string, fontSize: number) {
    Text(tip)
      .controlFontSize(fontSize)
      .visibility(tip.length > 0 ? Visibility.Visible : Visibility.None);
  }

  build() {
    Column() {
      this.TipTextStyle(this.content, 20);
      Text($r('app.string.title')).controlText();
      Text($r('app.string.warning')).fontSize(16).margin({ top: 0, bottom: 8, left: 24, right: 24 });
      Flex({ justifyContent: FlexAlign.SpaceAround }) {
        Text($r('app.string.notAllowButton'))
          .controlBtn(this.notAllowFontColor)
          .onClick(() => {
            let bundleName = globalThis.bundleName;
            vpnext.updateVpnAuthorizedState(bundleName + '**vpndialog**');
            this.destruction();
          });
        Text('|')
          .fontSize(20);
        Text($r('app.string.allowButton'))
          .controlBtn(this.allowFontColor)
          .onClick(() => {
            let bundleName = globalThis.bundleName;
            let abilityName = globalThis.abilityName;
            if (bundleName == null || abilityName == null) {
              this.destruction();
            } else {
              vpnext.updateVpnAuthorizedState(bundleName);
              let want: Want = {
                deviceId: '',
                bundleName: bundleName,
                abilityName: abilityName,
              };
              vpnext.startVpnExtensionAbility(want);
              this.destruction();
            }
          })

      }.margin({ bottom: 16, left: 24, right: 24});
    };
  }

  destruction() {
    this.controller.close();
    try {
      globalThis.vpnWin.destroy();
      globalThis.extensionContext.terminateSelf();
    } catch {
      console.info('Vpn ConfirmDialog destruction error!');
    }
    this.context.terminateSelf();
  }
};

@Entry
@Component
struct IndexHapComponent {
  private content: string = '';
  private allow: string = '';
  private use: string = '';
  private extensionWin: window.Window = null;
  private extensionContext: common.ServiceExtensionContext = null;

  ConfirmDialogController: CustomDialogController = new CustomDialogController({
    builder: ConfirmDialog({ content: this.content }),
    autoCancel: false,
    alignment: DialogAlignment.Center,
  });

  async aboutToAppear() {
    this.extensionWin = globalThis.vpnWin;
    this.extensionContext = globalThis.extensionContext;

    this.allow = await JSON.stringify(GlobalContext.getContext().getObject('app.string.allow'));
    this.use = await JSON.stringify(GlobalContext.getContext().getObject('app.string.use'));
    this.content = this.allow + GlobalContext.getContext()
      .getObject('bundleName') + this.use + GlobalContext.getContext().getObject('deviceName');
    this.ConfirmDialogController.open();
  }

  build() {
  }
}
