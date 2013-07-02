#ifndef __ISYNCIOCONNECTION_H__
#define __ISYNCIOCONNECTION_H__

#include <XnPlatform.h>
#include <XnStatus.h>

#include "IOutputConnection.h"
#include "ISyncInputConnection.h"

namespace xn
{

class ISyncIOConnection : virtual public ISyncInputConnection, 
                          virtual public IOutputConnection
{
public:
	virtual ~ISyncIOConnection() {}
	
	/**
	 * Receives data from the stream.
	 *
	 * @param	pData		[out]	Buffer to hold received data.
	 * @param	nSize		[in]	Size of data to receive.
	 */
	virtual XnStatus Receive(void* pData, XnUInt32& nSize) = 0;
	
	/**
	 * Sends a buffer on the connection.
	 *
	 * @param	pData		[out]	Buffer that holds data to send.
	 * @param	nDataSize	[in]	Size of data to send.
	 */
	virtual XnStatus Send(const void* pData, XnUInt32 nSize) = 0;
};

}

#endif // __ISYNCIOCONNECTION_H__