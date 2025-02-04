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

#include "PlaybackServiceTestsFixture.h"

TEST_F(PlaybackServiceTests, shouldFailToGetSharedMemoryInInactiveState)
{
    createPlaybackServiceShouldSuccess();
    getSharedMemoryShouldFail();
    getShmBufferShouldFail();
}

TEST_F(PlaybackServiceTests, shouldGetSharedMemory)
{
    createPlaybackServiceShouldSuccess();
    triggerSetMaxPlaybacks();
    triggerSetMaxWebAudioPlayers();
    sharedMemoryBufferWillBeInitialized();
    triggerSwitchToActive();
    sharedMemoryBufferWillReturnFdAndSize();
    getSharedMemoryShouldSucceed();
    getShmBufferShouldSucceed();
}

TEST_F(PlaybackServiceTests, shouldSetMaxPlaybacks)
{
    createPlaybackServiceShouldSuccess();
    triggerSetMaxPlaybacks();
    getMaxPlaybacksShouldSucceed();
}

TEST_F(PlaybackServiceTests, shouldSetMaxWebAudioPlayers)
{
    createPlaybackServiceShouldSuccess();
    triggerSetMaxWebAudioPlayers();
    getMaxWebAudioPlayersShouldSucceed();
}

TEST_F(PlaybackServiceTests, shouldSetClientDisplayName)
{
    createPlaybackServiceShouldSuccess();
    triggerSetClientDisplayName();
    clientDisplayNameShouldBeSet();
}
