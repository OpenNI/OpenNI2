#ifndef __ILINKOUTPUTSTREAM_H__
#define __ILINKOUTPUTSTREAM_H__

#include "XnLinkProtoLibDefs.h"
#include "XnLinkProtoUtils.h"
#include <XnPlatform.h>
#include <XnStatus.h>

namespace xn
{

class LinkOutputDataEndpoint;

class ILinkOutputStream
{
public:
	virtual ~ILinkOutputStream() {}

	virtual XnStatus Init(XnUInt16 nStreamID, 
	                      XnUInt32 nMaxMsgSize, 
						  XnUInt16 nMaxPacketSize, 
						  XnLinkCompressionType compression, 
						  XnUInt16 nInitialPacketID,
						  LinkOutputDataEndpoint* pOutputDataEndpoint) = 0;

	virtual XnBool IsInitialized() const = 0;
	virtual void Shutdown() = 0;
	virtual XnLinkCompressionType GetCompression() const = 0;
	
	virtual XnStatus SendData(XnUInt16 nMsgType, 
	                          XnUInt16 nCID, 
							  XnLinkFragmentation fragmentation,
							  const void* pData, 
							  XnUInt32 nDataSize) const = 0;

};

}

#endif // __ILINKOUTPUTSTREAM_H__
