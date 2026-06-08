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

import { BusinessError }  from '@ohos.base';

export default namespace netFirewall {
  loadLibrary("netFirewall_ani")

  function toBusinessError(err: Error): BusinessError {
    let businessErr = new BusinessError();
    businessErr.code = err.code ?? 0;
    businessErr.message = err.message ?? '';
    businessErr.stack = err.stack ?? '';
    businessErr.cause = err;
    return businessErr;
  }

  // ==================== Enums ====================

  export enum NetFirewallRuleDirection {
    RULE_IN = 1,
    RULE_OUT = 2
  }

  export enum FirewallRuleAction {
    RULE_ALLOW = 0,
    RULE_DENY = 1
  }

  export enum NetFirewallRuleType {
    RULE_IP = 1,
    RULE_DOMAIN = 2,
    RULE_DNS = 3
  }

  export enum NetFirewallOrderField {
    ORDER_BY_RULE_NAME = 1,
    ORDER_BY_RECORD_TIME = 100
  }

  export enum NetFirewallOrderType {
    ORDER_ASC = 1,
    ORDER_DESC = 100
  }

  // ==================== Interfaces ====================

  export interface RequestParam {
    page: int;
    pageSize: int;
    orderField: NetFirewallOrderField;
    orderType: NetFirewallOrderType;
  }

  export interface NetFirewallDnsParams {
    primaryDns: string;
    standbyDns?: string;
  }

  export interface NetFirewallIpParams {
    type: int;
    family?: int;
    address?: string;
    mask?: int;
    startIp?: string;
    endIp?: string;
  }

  export interface NetFirewallDomainParams {
    isWildcard: boolean;
    domain: string;
  }

  export interface NetFirewallPortParams {
    startPort: int;
    endPort: int;
  }

  export interface NetFirewallRule {
    userId: int;
    name: string;
    direction: NetFirewallRuleDirection;
    action: FirewallRuleAction;
    type: NetFirewallRuleType;
    isEnabled: boolean;
    id?: int;
    description?: string;
    appUid?: int;
    localIps?: Array<NetFirewallIpParams>;
    remoteIps?: Array<NetFirewallIpParams>;
    protocol?: int;
    localPorts?: Array<NetFirewallPortParams>;
    remotePorts?: Array<NetFirewallPortParams>;
    domains?: Array<NetFirewallDomainParams>;
    dns?: NetFirewallDnsParams;
  }

  export interface NetFirewallPolicy {
    isOpen: boolean;
    inAction: FirewallRuleAction;
    outAction: FirewallRuleAction;
  }

  export interface InterceptedRecord {
    time: int;
    localIp?: string;
    remoteIp?: string;
    localPort?: int;
    remotePort?: int;
    protocol?: int;
    appUid?: int;
    domain?: string;
  }

  export interface FirewallRulePage {
    page: int;
    pageSize: int;
    totalPage: int;
    data: Array<NetFirewallRule>;
  }

  export interface InterceptedRecordPage {
    page: int;
    pageSize: int;
    totalPage: int;
    data: Array<InterceptedRecord>;
  }

  // ==================== Native Sync Functions ====================

  export native function setNetFirewallPolicySync(userId: int, policy: NetFirewallPolicy): void;
  export native function getNetFirewallPolicySync(userId: int): NetFirewallPolicy;
  export native function addNetFirewallRuleSync(rule: NetFirewallRule): int;
  export native function updateNetFirewallRuleSync(rule: NetFirewallRule): void;
  export native function removeNetFirewallRuleSync(userId: int, ruleId: int): void;
  export native function getNetFirewallRulesSync(userId: int, requestParam: RequestParam): FirewallRulePage;
  export native function getNetFirewallRuleSync(userId: int, ruleId: int): NetFirewallRule;
  export native function getInterceptedRecordsSync(userId: int, requestParam: RequestParam): InterceptedRecordPage;

  // ==================== Async Functions ====================

  // ---------- setNetFirewallPolicy ----------
  export function setNetFirewallPolicy(userId: int, policy: NetFirewallPolicy): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
            setNetFirewallPolicySync(userId, policy);
        }).then(() => {
            resolve(undefined);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- getNetFirewallPolicy ----------
  export function getNetFirewallPolicy(userId: int): Promise<NetFirewallPolicy> {
    return new Promise<NetFirewallPolicy>((resolve, reject) => {
        taskpool.execute(() => {
            return getNetFirewallPolicySync(userId);
        }).then((content: Any) => {
            resolve(content as NetFirewallPolicy);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- addNetFirewallRule ----------
  export function addNetFirewallRule(rule: NetFirewallRule): Promise<int> {
    return new Promise<int>((resolve, reject) => {
        taskpool.execute(() => {
            return addNetFirewallRuleSync(rule);
        }).then((content: Any) => {
            resolve(content as int);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- updateNetFirewallRule ----------
  export function updateNetFirewallRule(rule: NetFirewallRule): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
            updateNetFirewallRuleSync(rule);
        }).then(() => {
            resolve(undefined);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- removeNetFirewallRule ----------
  export function removeNetFirewallRule(userId: int, ruleId: int): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        taskpool.execute(() => {
            removeNetFirewallRuleSync(userId, ruleId);
        }).then(() => {
            resolve(undefined);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- getNetFirewallRules ----------
  export function getNetFirewallRules(userId: int, requestParam: RequestParam): Promise<FirewallRulePage> {
    return new Promise<FirewallRulePage>((resolve, reject) => {
        taskpool.execute(() => {
            return getNetFirewallRulesSync(userId, requestParam);
        }).then((content: Any) => {
            resolve(content as FirewallRulePage);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- getNetFirewallRule ----------
  export function getNetFirewallRule(userId: int, ruleId: int): Promise<NetFirewallRule> {
    return new Promise<NetFirewallRule>((resolve, reject) => {
        taskpool.execute(() => {
            return getNetFirewallRuleSync(userId, ruleId);
        }).then((content: Any) => {
            resolve(content as NetFirewallRule);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }

  // ---------- getInterceptedRecords ----------
  export function getInterceptedRecords(userId: int, requestParam: RequestParam): Promise<InterceptedRecordPage> {
    return new Promise<InterceptedRecordPage>((resolve, reject) => {
        taskpool.execute(() => {
            return getInterceptedRecordsSync(userId, requestParam);
        }).then((content: Any) => {
            resolve(content as InterceptedRecordPage);
        }, (err: Error): void => {
            reject(toBusinessError(err));
        });
    });
  }
}
