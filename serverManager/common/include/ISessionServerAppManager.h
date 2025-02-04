/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Sky UK
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RIALTO_SERVERMANAGER_COMMON_I_SESSION_SERVER_APP_MANAGER_H_
#define RIALTO_SERVERMANAGER_COMMON_I_SESSION_SERVER_APP_MANAGER_H_

#include "LoggingLevels.h"
#include "SessionServerCommon.h"
#include <string>
#include <sys/types.h>

namespace rialto::servermanager::common
{
class ISessionServerAppManager
{
public:
    ISessionServerAppManager() = default;
    virtual ~ISessionServerAppManager() = default;

    ISessionServerAppManager(const ISessionServerAppManager &) = delete;
    ISessionServerAppManager(ISessionServerAppManager &&) = delete;
    ISessionServerAppManager &operator=(const ISessionServerAppManager &) = delete;
    ISessionServerAppManager &operator=(ISessionServerAppManager &&) = delete;

    virtual void preloadSessionServers(unsigned numOfPreloadedServers) = 0;
    virtual bool initiateApplication(const std::string &appName,
                                     const firebolt::rialto::common::SessionServerState &state,
                                     const firebolt::rialto::common::AppConfig &appConfig) = 0;
    virtual bool setSessionServerState(const std::string &appName,
                                       const firebolt::rialto::common::SessionServerState &newState) = 0;
    virtual void onSessionServerStateChanged(int serverId,
                                             const firebolt::rialto::common::SessionServerState &newState) = 0;
    virtual void sendPingEvents(int pingId) const = 0;
    virtual void onAck(int serverId, int pingId, bool success) = 0;
    virtual std::string getAppConnectionInfo(const std::string &appName) const = 0;
    virtual bool setLogLevels(const service::LoggingLevels &logLevels) const = 0;
};
} // namespace rialto::servermanager::common

#endif // RIALTO_SERVERMANAGER_COMMON_I_SESSION_SERVER_APP_MANAGER_H_
