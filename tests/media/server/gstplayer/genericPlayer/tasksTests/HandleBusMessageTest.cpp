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

#include "tasks/generic/HandleBusMessage.h"
#include "GenericPlayerContext.h"
#include "GlibWrapperMock.h"
#include "GstGenericPlayerClientMock.h"
#include "GstGenericPlayerPrivateMock.h"
#include "GstWrapperMock.h"
#include "Matchers.h"
#include <gst/gst.h>
#include <gtest/gtest.h>

using testing::_;
using testing::DoAll;
using testing::Return;
using testing::SetArgPointee;
using testing::StrictMock;

MATCHER_P2(QosInfoMatcher, expectedProcessed, expectedSropped, "")
{
    return ((expectedProcessed == arg.processed) && (expectedSropped == arg.dropped));
}

class HandleBusMessageTest : public testing::Test
{
protected:
    firebolt::rialto::server::GenericPlayerContext m_context;
    StrictMock<firebolt::rialto::server::GstGenericPlayerPrivateMock> m_gstPlayer;
    StrictMock<firebolt::rialto::server::GstGenericPlayerClientMock> m_gstPlayerClient;
    std::shared_ptr<firebolt::rialto::server::GstWrapperMock> m_gstWrapper{
        std::make_shared<StrictMock<firebolt::rialto::server::GstWrapperMock>>()};
    std::shared_ptr<firebolt::rialto::server::GlibWrapperMock> m_glibWrapper{
        std::make_shared<StrictMock<firebolt::rialto::server::GlibWrapperMock>>()};
    GstElement m_pipeline{};
    GstAppSrc m_audioSrc{};
    GstAppSrc m_videoSrc{};

    HandleBusMessageTest()
    {
        m_context.pipeline = &m_pipeline;
        m_context.streamInfo.emplace(firebolt::rialto::MediaSourceType::AUDIO, GST_ELEMENT(&m_audioSrc));
        m_context.streamInfo.emplace(firebolt::rialto::MediaSourceType::VIDEO, GST_ELEMENT(&m_videoSrc));
    }
};

TEST_F(HandleBusMessageTest, shouldNotHandleMessageWithUnknownType)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleEosMessageForAnotherPipeline)
{
    GstMessage message{};
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_EOS;
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleMessageEosWhenPipelineIsNull)
{
    m_context.pipeline = nullptr;
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_EOS;
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleMessageEosWhenEosAlreadyNotified)
{
    m_context.eosNotified = true;
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_EOS;
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleEosMessage)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_EOS;
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::END_OF_STREAM));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();

    EXPECT_TRUE(m_context.eosNotified);
}

TEST_F(HandleBusMessageTest, shouldNotHandleStateChangedMessageForAnotherPipeline)
{
    GstMessage message{};
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleMessageStateChangedWhenPipelineIsNull)
{
    m_context.pipeline = nullptr;
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleStateChangedMessageWhenGstPlayerClientIsNull)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_READY;
    GstState newState = GST_STATE_NULL;
    GstState pending = GST_STATE_VOID_PENDING;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(oldState)).Times(2).WillRepeatedly(Return("Ready"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(newState)).Times(2).WillRepeatedly(Return("Null"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(pending)).WillOnce(Return("Void"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   nullptr,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleStateChangedToNullMessage)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_READY;
    GstState newState = GST_STATE_NULL;
    GstState pending = GST_STATE_VOID_PENDING;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(oldState)).Times(2).WillRepeatedly(Return("Ready"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(newState)).Times(2).WillRepeatedly(Return("Null"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(pending)).WillOnce(Return("Void"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::STOPPED));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleStateChangedToPausedMessage)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_READY;
    GstState newState = GST_STATE_PAUSED;
    GstState pending = GST_STATE_VOID_PENDING;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(oldState)).Times(2).WillRepeatedly(Return("Ready"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(newState)).Times(2).WillRepeatedly(Return("Paused"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(pending)).WillOnce(Return("Void"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::PAUSED));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleStateChangedToPausedAndPendingPausedMessage)
{
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_PAUSED;
    GstState newState = GST_STATE_PAUSED;
    GstState pending = GST_STATE_PAUSED;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(GST_STATE_PAUSED)).WillRepeatedly(Return("Paused"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleStateChangedToPlayingMessage)
{
    m_context.isPlaying = false;
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_READY;
    GstState newState = GST_STATE_PLAYING;
    GstState pending = GST_STATE_VOID_PENDING;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(oldState)).Times(2).WillRepeatedly(Return("Ready"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(newState)).Times(2).WillRepeatedly(Return("Playing"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(pending)).WillOnce(Return("Void"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(m_gstPlayer, startPositionReportingAndCheckAudioUnderflowTimer());
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::PLAYING));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();

    EXPECT_TRUE(m_context.isPlaying);
}

TEST_F(HandleBusMessageTest, shouldHandleStateChangedToPlayingMessageAndSetPendingPlaybackRate)
{
    m_context.pendingPlaybackRate = 1.25;
    GstMessage message{};
    GST_MESSAGE_SRC(&message) = GST_OBJECT(&m_pipeline);
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_STATE_CHANGED;
    GstState oldState = GST_STATE_READY;
    GstState newState = GST_STATE_PLAYING;
    GstState pending = GST_STATE_VOID_PENDING;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseStateChanged(&message, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(oldState), SetArgPointee<2>(newState), SetArgPointee<3>(pending)));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(oldState)).Times(2).WillRepeatedly(Return("Ready"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(newState)).Times(2).WillRepeatedly(Return("Playing"));
    EXPECT_CALL(*m_gstWrapper, gstElementStateGetName(pending)).WillOnce(Return("Void"));
    EXPECT_CALL(*m_gstWrapper, gstDebugBinToDotFileWithTs(GST_BIN(&m_pipeline), _, _));
    EXPECT_CALL(m_gstPlayer, startPositionReportingAndCheckAudioUnderflowTimer());
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::PLAYING));
    EXPECT_CALL(m_gstPlayer, setPendingPlaybackRate());
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleQosMessageForUnsupportedFormat)
{
    GstMessage message{};
    GstObject src{};
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_QOS;
    GST_MESSAGE_SRC(&message) = &src;

    guint64 dropped = 2u;
    guint64 processed = 5u;
    GstFormat format = GST_FORMAT_UNDEFINED;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseQos(&message, _, _, _, _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageParseQosStats(&message, _, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(format), SetArgPointee<2>(processed), SetArgPointee<3>(dropped)));
    EXPECT_CALL(*m_gstWrapper, gstFormatGetName(format)).WillOnce(Return("Undefined"));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));

    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldNotHandleQosMessageForUnknownSourceType)
{
    GstMessage message{};
    GstObject src{};
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_QOS;
    GST_MESSAGE_SRC(&message) = &src;

    guint64 dropped = 2u;
    guint64 processed = 5u;
    GstFormat format = GST_FORMAT_BUFFERS;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseQos(&message, _, _, _, _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageParseQosStats(&message, _, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(format), SetArgPointee<2>(processed), SetArgPointee<3>(dropped)));
    EXPECT_CALL(*m_gstWrapper, gstElementClassGetMetadata(GST_ELEMENT_GET_CLASS(GST_MESSAGE_SRC(&message)),
                                                          CharStrMatcher(GST_ELEMENT_METADATA_KLASS)))
        .WillOnce(Return("Unknown"));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));

    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleQosMessageForVideo)
{
    GstMessage message{};
    GstObject src{};
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_QOS;
    GST_MESSAGE_SRC(&message) = &src;

    guint64 dropped = 2u;
    guint64 processed = 5u;
    GstFormat format = GST_FORMAT_BUFFERS;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseQos(&message, _, _, _, _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageParseQosStats(&message, _, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(format), SetArgPointee<2>(processed), SetArgPointee<3>(dropped)));
    EXPECT_CALL(*m_gstWrapper, gstElementClassGetMetadata(GST_ELEMENT_GET_CLASS(GST_MESSAGE_SRC(&message)),
                                                          CharStrMatcher(GST_ELEMENT_METADATA_KLASS)))
        .WillOnce(Return("Video"));
    EXPECT_CALL(m_gstPlayerClient,
                notifyQos(firebolt::rialto::MediaSourceType::VIDEO, QosInfoMatcher(processed, dropped)));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

TEST_F(HandleBusMessageTest, shouldHandleQosMessageForAudio)
{
    GstMessage message{};
    GstObject src{};
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_QOS;
    GST_MESSAGE_SRC(&message) = &src;

    guint64 dropped = 2u;
    guint64 processed = 5u;
    GstFormat format = GST_FORMAT_DEFAULT;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseQos(&message, _, _, _, _, _));
    EXPECT_CALL(*m_gstWrapper, gstMessageParseQosStats(&message, _, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(format), SetArgPointee<2>(processed), SetArgPointee<3>(dropped)));
    EXPECT_CALL(*m_gstWrapper, gstElementClassGetMetadata(GST_ELEMENT_GET_CLASS(GST_MESSAGE_SRC(&message)),
                                                          CharStrMatcher(GST_ELEMENT_METADATA_KLASS)))
        .WillOnce(Return("Audio"));
    EXPECT_CALL(m_gstPlayerClient,
                notifyQos(firebolt::rialto::MediaSourceType::AUDIO, QosInfoMatcher(processed, dropped)));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

/**
 * Test HandleBusMessage notifies FAILURE for none stream errors.
 */
TEST_F(HandleBusMessageTest, shouldHandleErrorMessageNoEos)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_CORE_ERROR;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::FAILURE));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

/**
 * Test HandleBusMessage notifies FAILURE for none stream errors even when sources are EOS.
 */
TEST_F(HandleBusMessageTest, shouldHandleErrorMessageWhenEosAllSources)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_CORE_ERROR;

    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::AUDIO, GST_ELEMENT(&m_audioSrc));
    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::VIDEO, GST_ELEMENT(&m_videoSrc));

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::FAILURE));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

/**
 * Test HandleBusMessage notifies FAILURE for a stream error when no sources are EOS.
 */
TEST_F(HandleBusMessageTest, shouldHandleStreamErrorMessageNoEos)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_STREAM_ERROR;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::FAILURE));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

/**
 * Test HandleBusMessage notifies FAILURE for a stream error when not all sources are EOS.
 */
TEST_F(HandleBusMessageTest, shouldHandleStreamErrorMessageWhenEosSingleSource)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_STREAM_ERROR;

    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::AUDIO, GST_ELEMENT(&m_audioSrc));

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::FAILURE));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}

/**
 * Test HandleBusMessage notifies END_OF_STREAM for a stream error when all sources are EOS.
 */
TEST_F(HandleBusMessageTest, shouldHandleStreamErrorMessageWhenEosAllSources)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_STREAM_ERROR;

    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::AUDIO, GST_ELEMENT(&m_audioSrc));
    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::VIDEO, GST_ELEMENT(&m_videoSrc));

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(m_gstPlayerClient, notifyPlaybackState(firebolt::rialto::PlaybackState::END_OF_STREAM));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();

    EXPECT_TRUE(m_context.eosNotified);
}

/**
 * Test HandleBusMessage does not notifies END_OF_STREAM for a stream error when all sources are EOS but EOS has already
 * been notified.
 */
TEST_F(HandleBusMessageTest, shouldHandleStreamErrorMessageWhenEosAllSourcesAndEosAlreadyNotfied)
{
    GstMessage message{};
    GError err{};
    gchar *debug = "Error message";
    GST_MESSAGE_TYPE(&message) = GST_MESSAGE_ERROR;
    GST_MESSAGE_SRC(&message) = GST_OBJECT_CAST(&m_videoSrc);
    err.domain = GST_STREAM_ERROR;

    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::AUDIO, GST_ELEMENT(&m_audioSrc));
    m_context.endOfStreamInfo.emplace(firebolt::rialto::MediaSourceType::VIDEO, GST_ELEMENT(&m_videoSrc));
    m_context.eosNotified = true;

    EXPECT_CALL(*m_gstWrapper, gstMessageParseError(&message, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(&err), SetArgPointee<2>(debug)));
    EXPECT_CALL(*m_glibWrapper, gFree(debug));
    EXPECT_CALL(*m_glibWrapper, gErrorFree(&err));
    EXPECT_CALL(*m_gstWrapper, gstMessageUnref(&message));
    firebolt::rialto::server::tasks::generic::HandleBusMessage task{m_context,    m_gstPlayer,   &m_gstPlayerClient,
                                                                    m_gstWrapper, m_glibWrapper, &message};
    task.execute();
}
