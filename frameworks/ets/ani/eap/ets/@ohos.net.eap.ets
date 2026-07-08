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

import type { Callback } from '@ohos.base';

export default namespace eap {
  loadLibrary("eap_ani");

  // ===== regCustomEapHandler =====
  // 注册监听回调，用于接收后续的 EapData 数据
  native function regCustomEapHandlerSync(netType: int, eapCode: int, eapType: int,
                                           callback: Callback<EapData>): void;

  export function regCustomEapHandler(netType: int, eapCode: int, eapType: int,
                                       callback: Callback<EapData>): void {
    regCustomEapHandlerSync(netType, eapCode, eapType, callback);
  }

  // ===== unregCustomEapHandler =====
  native function unregCustomEapHandlerSync(netType: int, eapCode: int, eapType: int,
                                             callback: Callback<EapData>): void;

  export function unregCustomEapHandler(netType: int, eapCode: int, eapType: int,
                                         callback: Callback<EapData>): void {
    unregCustomEapHandlerSync(netType, eapCode, eapType, callback);
  }

  // ===== replyCustomEapData =====
  native function replyCustomEapDataSync(result: CustomResult, data: EapData): void;

  export function replyCustomEapData(result: CustomResult, data: EapData): void {
    replyCustomEapDataSync(result, data);
  }

  // ===== startEthEap =====
  native function startEthEapSync(netId: int, profile: EthEapProfile): void;

  export function startEthEap(netId: int, profile: EthEapProfile): void {
    startEthEapSync(netId, profile);
  }

  // ===== logOffEthEap =====
  native function logOffEthEapSync(netId: int): void;

  export function logOffEthEap(netId: int): void {
    logOffEthEapSync(netId);
  }

  // ===== Enums =====

  export enum EapMethod {
    EAP_NONE = 0,
    EAP_PEAP = 1,
    EAP_TLS = 2,
    EAP_TTLS = 3,
    EAP_PWD = 4,
    EAP_SIM = 5,
    EAP_AKA = 6,
    EAP_AKA_PRIME = 7,
    EAP_UNAUTH_TLS = 8
  }

  export enum Phase2Method {
    PHASE2_NONE = 0,
    PHASE2_PAP = 1,
    PHASE2_MSCHAP = 2,
    PHASE2_MSCHAPV2 = 3,
    PHASE2_GTC = 4,
    PHASE2_SIM = 5,
    PHASE2_AKA = 6,
    PHASE2_AKA_PRIME = 7
  }

  export enum CustomResult {
    RESULT_FAIL = 0,
    RESULT_NEXT = 1,
    RESULT_FINISH = 2
  }

  // ===== Interfaces =====

  export interface EapData {
    msgId: int;
    eapBuffer: Uint8Array;
    bufferLen: int;
  }

  export class EapDataInner implements EapData {
    msgId: int = -1;
    eapBuffer: Uint8Array = new Uint8Array(0);
    bufferLen: int = 0;
  }

  export interface EthEapProfile {
    eapMethod: EapMethod;
    phase2Method: Phase2Method;
    identity: string;
    anonymousIdentity: string;
    password: string;
    caCertAliases: string;
    caPath: string;
    clientCertAliases: string;
    certEntry: Uint8Array;
    certPassword: string;
    altSubjectMatch: string;
    domainSuffixMatch: string;
    realm: string;
    plmn: string;
    eapSubId: int;
  }

  export class EthEapProfileInner implements EthEapProfile {
    eapMethod: EapMethod = EapMethod.EAP_NONE;
    phase2Method: Phase2Method = Phase2Method.PHASE2_NONE;
    identity: string = "";
    anonymousIdentity: string = "";
    password: string = "";
    caCertAliases: string = "";
    caPath: string = "";
    clientCertAliases: string = "";
    certEntry: Uint8Array = new Uint8Array(0);
    certPassword: string = "";
    altSubjectMatch: string = "";
    domainSuffixMatch: string = "";
    realm: string = "";
    plmn: string = "";
    eapSubId: int = 0;
  }
}