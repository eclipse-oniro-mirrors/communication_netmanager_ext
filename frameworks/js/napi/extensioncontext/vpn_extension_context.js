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

let ExtensionContext = requireNapi('application.ExtensionContext');
let Caller = requireNapi('application.Caller');

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor() {
    super(ERROR_MSG_INVALID_PARAM);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class VpnExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }
  startVpnExtensionAbility(want, callback) {
    console.log('startVpnExtensionAbility');
    return this.__context_impl__.startVpnExtensionAbility(want, callback);
  }


  stopVpnExtensionAbility(want, callback) {
    console.log('stopVpnExtensionAbility');
    return this.__context_impl__.stopVpnExtensionAbility(want, callback);
  }

}

export default VpnExtensionContext;
