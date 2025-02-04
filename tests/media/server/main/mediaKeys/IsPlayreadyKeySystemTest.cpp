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

#include "MediaKeysTestBase.h"

class RialtoServerMediaKeysIsPlayreadyKeySystemTest : public MediaKeysTestBase
{
protected:
    RialtoServerMediaKeysIsPlayreadyKeySystemTest()
    {
        createMediaKeys(kWidevineKeySystem);
        createKeySession(kWidevineKeySystem);
    }
    ~RialtoServerMediaKeysIsPlayreadyKeySystemTest() { destroyMediaKeys(); }
};

/**
 * Test that isPlayreadyKeySystem returns true.
 */
TEST_F(RialtoServerMediaKeysIsPlayreadyKeySystemTest, ReturnTrue)
{
    mainThreadWillEnqueueTaskAndWait();
    EXPECT_CALL(*m_mediaKeySessionMock, isPlayreadyKeySystem()).WillOnce(Return(true));

    EXPECT_TRUE(m_mediaKeys->isPlayreadyKeySystem(m_kKeySessionId));
}

/**
 * Test that isPlayreadyKeySystem returns false if the key session does not exsist.
 */
TEST_F(RialtoServerMediaKeysIsPlayreadyKeySystemTest, ReturnFalseWhenSessionDoesNotExist)
{
    mainThreadWillEnqueueTaskAndWait();
    EXPECT_FALSE(m_mediaKeys->isPlayreadyKeySystem(m_kKeySessionId + 1));
}

/**
 * Test that isPlayreadyKeySystem returns false
 */
TEST_F(RialtoServerMediaKeysIsPlayreadyKeySystemTest, ReturnFalse)
{
    mainThreadWillEnqueueTaskAndWait();
    EXPECT_CALL(*m_mediaKeySessionMock, isPlayreadyKeySystem()).WillOnce(Return(false));

    EXPECT_FALSE(m_mediaKeys->isPlayreadyKeySystem(m_kKeySessionId));
}
