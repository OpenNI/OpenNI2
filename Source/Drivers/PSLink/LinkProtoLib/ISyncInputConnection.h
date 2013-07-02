#ifndef __ISYNCINPUTCONNECTION_H__
#define __ISYNCINPUTCONNECTION_H__

#include <XnPlatform.h>
#include "IConnection.h"

namespace xn
{

class ISyncInputConnection : virtual public IConnection
{
public:
	virtual ~ISyncInputConnection() {}
	virtual XnStatus Receive(void* pData, XnUInt32& nSize) = 0;
};

}

#endif // __ISYNCINPUTCONNECTION_H__