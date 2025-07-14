/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef ETH_EAP_PROFILE_H
#define ETH_EAP_PROFILE_H
 
#include <string>
#include <vector>
 
#include "parcel.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
typedef enum {
    EAP_NONE       = 0,
    EAP_PEAP       = 1,
    EAP_TLS        = 2,
    EAP_TTLS       = 3,
    EAP_PWD        = 4,
    EAP_SIM        = 5,
    EAP_AKA        = 6,
    EAP_AKA_PRIME  = 7,
    EAP_UNAUTH_TLS = 8
} EapMethod;
 
typedef enum {
    PHASE2_NONE      = 0,
    PHASE2_PAP       = 1,  // only EAP-TTLS support this mode
    PHASE2_MSCHAP    = 2,  // only EAP-TTLS support this mode
    PHASE2_MSCHAPV2  = 3,  // only EAP-PEAP/EAP-TTLS support this mode
    PHASE2_GTC       = 4,  // only EAP-PEAP/EAP-TTLS support this mode
    PHASE2_SIM       = 5,  // only EAP-PEAP support this mode
    PHASE2_AKA       = 6,  // only EAP-PEAP support this mode
    PHASE2_AKA_PRIME = 7   // only EAP-PEAP support this mode
} Phase2Method;
 
struct EthEapProfile : public Parcelable {
    EapMethod eapMethod;            /* EAP authentication mode:PEAP/TLS/TTLS/PWD/SIM/AKA/AKA' */
    Phase2Method phase2Method;      /* Second stage authentication method */
    std::string identity;           /* Identity information */
    std::string anonymousIdentity;  /* Anonymous identity information */
    std::string password;           /* EAP mode password */
    std::string caCertAliases;      /* CA certificate alias */
    std::string caPath;             /* CA certificate path */
    std::string clientCertAliases;  /* Client certificate alias */
    std::vector<uint8_t> certEntry; /* CA certificate entry */
    std::string certPassword;       /* Certificate password */
    std::string altSubjectMatch;    /* Alternative topic matching */
    std::string domainSuffixMatch;  /* Domain suffix matching */
    std::string realm;              /* The field of passport credentials */
    std::string plmn;               /* Public Land Mobile Network of the provider of Passpoint credential */
    int32_t eapSubId;               /* Sub ID of SIM card */
    
    virtual bool Marshalling(Parcel& parcel) const override;
    static EthEapProfile* Unmarshalling(Parcel &parcel);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_PROFILE_H