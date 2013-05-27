#ifndef __ICONNECTION_H__
#define __ICONNECTION_H__

#include <XnStatus.h>

namespace xn
{

class IConnection
{
public:
	virtual XnStatus Connect() = 0;
	virtual void Disconnect() = 0;
	virtual XnBool IsConnected() const = 0;
	virtual XnUInt16 GetMaxPacketSize() const = 0;
};

}

#endif // __ICONNECTION_H__