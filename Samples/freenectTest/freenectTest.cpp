#include <iostream>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "FreenectDriver.cpp"

#include <OpenNI.h>


FreenectDriver* f_driver = new FreenectDriver(NULL);
oni::driver::DeviceBase* f_device;
oni::driver::StreamBase* f_depth;


void shutdown(int s) {
	std::cout << "Caught signal " << s << std::endl;

	if (f_device != NULL) {
		f_device->destroyStream(f_depth);
		f_driver->deviceClose(f_device);
	}
	
	exit(1);
}

int alt();
void analyzeFrame(const openni::VideoFrameRef& frame);


int main(int argc, char **argv)
{
	// handle SIGINT (Ctrl+c)
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = shutdown;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	
	
	return alt();
	

	f_driver->initialize(NULL, NULL, NULL, NULL);
	f_device = f_driver->deviceOpen("freenect:0");
	f_depth = f_device->createStream(ONI_SENSOR_DEPTH);
		
		
	// wait for Ctrl+c
	while (true);
	

	shutdown(0);
	return 0;
}


using namespace openni;

int alt()
{	
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	Device device;
	rc = device.open(ANY_DEVICE);
	//rc = device.open("freenect:0");
	if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}
	
	VideoStream depth, color;

	if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc == STATUS_OK)
		{
			rc = depth.start();
			if (rc != STATUS_OK)
			{
				printf("Couldn't start depth stream\n%s\n", OpenNI::getExtendedError());
			}
		}
		else
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
		}
	}

	if (device.getSensorInfo(SENSOR_COLOR) != NULL)
	{
		rc = color.create(device, SENSOR_COLOR);
		if (rc == STATUS_OK)
		{
			rc = color.start();
			if (rc != STATUS_OK)
			{
				printf("Couldn't start color stream\n%s\n", OpenNI::getExtendedError());
			}
		}
		else
		{
			printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
		}
	}
	
	
	VideoFrameRef frame;

	VideoStream* streams[] = {&depth, &color};
	
	while (true)
	{
		int readyStream = -1;
		rc = OpenNI::waitForAnyStream(streams, 2, &readyStream);
		if (rc != STATUS_OK)
		{
			printf("Wait failed\n");
			continue;
		}

		switch (readyStream)
		{
		case 0:
			// Depth
			depth.readFrame(&frame);
			break;
		case 1:
			// Color
			color.readFrame(&frame);
			break;
		default:
			printf("Unxpected stream\n");
		}

		analyzeFrame(frame);
	}


	depth.stop();
	color.stop();
	depth.destroy();
	color.destroy();
	device.close();
	OpenNI::shutdown();	
	
	return 0;
}


void analyzeFrame(const VideoFrameRef& frame)
{
	DepthPixel* pDepth;
	RGB888Pixel* pColor;

	int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;
	
	
	
	std::cout << "frame width = " << frame.getWidth() << "; frame height = " << frame.getHeight() << std::endl;
	/*
	pDepth = (DepthPixel*)frame.getData();
	for (int x = 0; x < frame.getWidth(); x++) {
		for (int y = 0; y < frame.getHeight(); y++) {
			std::cout << pDepth[x*y+x] << " ";
		}
		std::cout << std::endl;
	}
	*/



	switch (frame.getVideoMode().getPixelFormat())
	{
	case PIXEL_FORMAT_DEPTH_1_MM:
	case PIXEL_FORMAT_DEPTH_100_UM:
		pDepth = (DepthPixel*)frame.getData();
		printf("[%08llu] %8d\n", (long long)frame.getTimestamp(),
			pDepth[middleIndex]);
		break;
	case PIXEL_FORMAT_RGB888:
		pColor = (RGB888Pixel*)frame.getData();
		printf("[%08llu] 0x%02x%02x%02x\n", (long long)frame.getTimestamp(),
			pColor[middleIndex].r&0xff,
			pColor[middleIndex].g&0xff,
			pColor[middleIndex].b&0xff);
		break;
	default:
		printf("Unknown format\n");
	}
}
