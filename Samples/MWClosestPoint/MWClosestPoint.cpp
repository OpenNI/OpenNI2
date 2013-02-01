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
#include <OpenNI.h>
#include "MWClosestPoint.h"

using namespace openni;

namespace closest_point
{

class StreamListener;

struct ClosestPointInternal
{
	ClosestPointInternal(ClosestPoint* pClosestPoint) :
		m_pDevice(NULL), m_pDepthStream(NULL), m_pListener(NULL), m_pStreamListener(NULL), m_pClosesPoint(pClosestPoint)
		{}

	void Raise()
	{
		if (m_pListener != NULL)
			m_pListener->readyForNextData(m_pClosesPoint);
	}
	bool m_oniOwner;
	Device* m_pDevice;
	VideoStream* m_pDepthStream;

	ClosestPoint::Listener* m_pListener;

	StreamListener* m_pStreamListener;

	ClosestPoint* m_pClosesPoint;
};

class StreamListener : public VideoStream::NewFrameListener
{
public:
	StreamListener(ClosestPointInternal* pClosestPoint) : m_pClosestPoint(pClosestPoint)
	{}
	virtual void onNewFrame(VideoStream& stream)
	{
		m_pClosestPoint->Raise();
	}
private:
	ClosestPointInternal* m_pClosestPoint;
};

ClosestPoint::ClosestPoint(const char* uri)
{
	m_pInternal = new ClosestPointInternal(this);

	m_pInternal->m_pDevice = new Device;
	m_pInternal->m_oniOwner = true;

	OpenNI::initialize();
	Status rc = m_pInternal->m_pDevice->open(uri);
	if (rc != STATUS_OK)
	{
		printf("Open device failed:\n%s\n", OpenNI::getExtendedError());
		return;
	}
	initialize();
}

ClosestPoint::ClosestPoint(openni::Device* pDevice)
{
	m_pInternal = new ClosestPointInternal(this);

	m_pInternal->m_pDevice = pDevice;
	m_pInternal->m_oniOwner = false;

	OpenNI::initialize();

	if (pDevice != NULL)
	{
		initialize();
	}
}

void ClosestPoint::initialize()
{
	m_pInternal->m_pStreamListener = NULL;
	m_pInternal->m_pListener = NULL;

	m_pInternal->m_pDepthStream = new VideoStream;
	Status rc = m_pInternal->m_pDepthStream->create(*m_pInternal->m_pDevice, SENSOR_DEPTH);
	if (rc != STATUS_OK)
	{
		printf("Created failed\n%s\n", OpenNI::getExtendedError());
		return;
	}

	m_pInternal->m_pStreamListener = new StreamListener(m_pInternal);

	rc = m_pInternal->m_pDepthStream->start();
	if (rc != STATUS_OK)
	{
		printf("Start failed:\n%s\n", OpenNI::getExtendedError());
	}

	m_pInternal->m_pDepthStream->addNewFrameListener(m_pInternal->m_pStreamListener);
}

ClosestPoint::~ClosestPoint()
{
	if (m_pInternal->m_pDepthStream != NULL)
	{
		m_pInternal->m_pDepthStream->removeNewFrameListener(m_pInternal->m_pStreamListener);

		m_pInternal->m_pDepthStream->stop();
		m_pInternal->m_pDepthStream->destroy();

		delete m_pInternal->m_pDepthStream;
	}

	if (m_pInternal->m_pStreamListener != NULL)
	{
		delete m_pInternal->m_pStreamListener;
	}

	if (m_pInternal->m_oniOwner)
	{
		if (m_pInternal->m_pDevice != NULL)
		{
			m_pInternal->m_pDevice->close();

			delete m_pInternal->m_pDevice;
		}
	}

	OpenNI::shutdown();


	delete m_pInternal;
}

bool ClosestPoint::isValid() const
{
	if (m_pInternal == NULL)
		return false;
	if (m_pInternal->m_pDevice == NULL)
		return false;
	if (m_pInternal->m_pDepthStream == NULL)
		return false;
	if (!m_pInternal->m_pDepthStream->isValid())
		return false;

	return true;
}

Status ClosestPoint::setListener(Listener& listener)
{
	m_pInternal->m_pListener = &listener;
	return STATUS_OK;
}
void ClosestPoint::resetListener()
{
	m_pInternal->m_pListener = NULL;
}

Status ClosestPoint::getNextData(IntPoint3D& closestPoint, VideoFrameRef& rawFrame)
{
	Status rc = m_pInternal->m_pDepthStream->readFrame(&rawFrame);
	if (rc != STATUS_OK)
	{
		printf("readFrame failed\n%s\n", OpenNI::getExtendedError());
	}

	DepthPixel* pDepth = (DepthPixel*)rawFrame.getData();
	bool found = false;
	closestPoint.Z = 0xffff;
	int width = rawFrame.getWidth();
	int height = rawFrame.getHeight();

	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x, ++pDepth)
		{
			if (*pDepth < closestPoint.Z && *pDepth != 0)
			{
				closestPoint.X = x;
				closestPoint.Y = y;
				closestPoint.Z = *pDepth;
				found = true;
			}
		}

	if (!found)
	{
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

}


