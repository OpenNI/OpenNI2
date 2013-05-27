#ifndef __IASYNCINPUTCONNECTION_H__
#define __IASYNCINPUTCONNECTION_H__

#include "IConnection.h"
#include <XnPlatform.h>

namespace xn
{

class IDataDestination
{
public:
	virtual ~IDataDestination() {}
	virtual XnStatus IncomingData(const void* pData, XnUInt32 nSize) = 0;
	virtual void HandleDisconnection() = 0;
};

class IAsyncInputConnection : virtual public IConnection
{
public:
	virtual ~IAsyncInputConnection() {}
	virtual XnStatus SetDataDestination(IDataDestination* pDataDestination) = 0;
};

}

#endif // __IASYNCINPUTCONNECTION_H__