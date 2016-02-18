/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include "OniRecorder.h"

#include "XnLockGuard.h"

ONI_NAMESPACE_IMPLEMENTATION_BEGIN

Recorder::Recorder(OniRecorderHandle handle) : m_handle(handle), m_running(FALSE), m_started(FALSE), m_wasStarted(FALSE)
{}

Recorder::~Recorder()
{
	stop();
	detachAllStreams();
}

OniStatus Recorder::initialize(const char* /*path*/)
{
	return ONI_STATUS_OK;
}

OniStatus Recorder::attachStream(VideoStream& stream, OniBool /*allowLossyCompression*/)
{
	if (m_wasStarted)
	{
		return ONI_STATUS_ERROR;
	}

	xnl::LockGuard<StreamFrameIDList> guard(m_frameIds);
	VideoStream* pStream = &stream;
	if (m_frameIds.Find(pStream) == m_frameIds.End())
	{
		if (ONI_STATUS_OK == pStream->addRecorder(*this))
		{
			m_frameIds[pStream] = 0;
			return ONI_STATUS_OK;
		}
	}
	return ONI_STATUS_ERROR;
}

OniStatus Recorder::detachStream(VideoStream& stream)
{
	xnl::LockGuard<StreamFrameIDList> guard(m_frameIds);
	VideoStream* pStream = &stream;
	if (m_frameIds.End() != m_frameIds.Find(pStream))
	{
		stream.removeRecorder(*this);
		m_frameIds.Remove(pStream);
		return ONI_STATUS_OK;
	}
	return ONI_STATUS_BAD_PARAMETER;
}

OniStatus Recorder::detachAllStreams()
{
	xnl::LockGuard<StreamFrameIDList> guard(m_frameIds);
	while (!m_frameIds.IsEmpty())
	{
		detachStream(*m_frameIds.Begin()->Key());
	}

	return ONI_STATUS_OK;
}

OniStatus Recorder::start()
{
	m_wasStarted = true;
	return ONI_STATUS_OK;
}

void Recorder::stop()
{
	m_started = false;
}

ONI_NAMESPACE_IMPLEMENTATION_END
