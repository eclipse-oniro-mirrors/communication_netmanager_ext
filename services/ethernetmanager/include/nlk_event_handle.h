/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NLK_EVENT_HANDLE_H
#define NLK_EVENT_HANDLE_H

#include <string>

#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
struct NlkEventInfo {
    std::string iface_;
    uint64_t ifiFlags_ = 0;
};

class NlkEventHandle : public virtual RefBase {
public:
    virtual void Handle(const struct NlkEventInfo &info) = 0;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NLK_EVENT_HANDLE_H