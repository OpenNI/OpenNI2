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
#include <MWClosestPoint.h>
#include <OpenNI.h>

#ifdef WIN32
#include <conio.h>
int wasKeyboardHit()
{
        return (int)_kbhit();
}

#else // linux

#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
int wasKeyboardHit()
{
        struct termios oldt, newt;
        int ch;
        int oldf;

        // don't echo and don't wait for ENTER
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

        // make it non-blocking (so we can check without waiting)
        if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK))
        {
                return 0;
        }

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (0 != fcntl(STDIN_FILENO, F_SETFL, oldf))
        {
                return 0;
        }

        if(ch != EOF)
        {
                ungetc(ch, stdin);
                return 1;
        }

        return 0;
}

void Sleep(int ms)
{
        usleep(ms*1000);
}

#endif // WIN32



class MyMwListener : public closest_point::ClosestPoint::Listener
{
public:
	void readyForNextData(closest_point::ClosestPoint* pClosestPoint)
	{
		openni::VideoFrameRef frame;
		closest_point::IntPoint3D closest;
		openni::Status rc = pClosestPoint->getNextData(closest, frame);

		if (rc == openni::STATUS_OK)
		{
			printf("%d, %d, %d\n", closest.X, closest.Y, closest.Z);
		}
		else
		{
			printf("Update failed\n");
		}
	}
};


int main()
{

	closest_point::ClosestPoint closestPoint;

	if (!closestPoint.isValid())
	{
		printf("ClosestPoint: error in initialization\n");
		return 1;
	}

	MyMwListener myListener;

	closestPoint.setListener(myListener);

	while (!wasKeyboardHit())
	{
		Sleep(1000);
	}

	closestPoint.resetListener();

	return 0;
}
