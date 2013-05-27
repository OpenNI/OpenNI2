#ifndef __IOUTPUTCONNECTION_H__
#define __IOUTPUTCONNECTION_H__

#include "IConnection.h"

namespace xn
{

class IOutputConnection : virtual public IConnection
{
public:
	virtual ~IOutputConnection() {}

	/**
	 * Sends a buffer on the connection.
	 *
	 * @param	pData		[out]	Buffer that holds data to send.
	 * @param	nDataSize	[in]	Size of data to send.
	 */
	virtual XnStatus Send(const void* pData, XnUInt32 nSize) = 0;
};

}

#endif // __IOUTPUTCONNECTION_H__