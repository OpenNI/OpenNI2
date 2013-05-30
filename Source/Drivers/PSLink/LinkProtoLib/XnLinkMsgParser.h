#ifndef __XNLINKMSGPARSER_H__
#define __XNLINKMSGPARSER_H__

#include <XnStatus.h>
#include "XnLinkProtoUtils.h"

namespace xn
{

class LinkMsgParser 
{
public:
	LinkMsgParser();
	virtual ~LinkMsgParser();
	virtual XnStatus Init();
	virtual void Shutdown();
	
	XnStatus BeginParsing(void* pDestBuffer, XnUInt32 nDestBufferSize);
	XnStatus ParsePacket(const LinkPacketHeader& header, const XnUInt8* pData);

	const void* GetParsedData() const;
	XnUInt32 GetParsedSize() const;
	XnUInt32 GetBufferSize() const;

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation, 
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);

private:
	XnUInt8* m_pDestBuffer;
	XnUInt8* m_pCurrDest;
	XnUInt8* m_pDestEnd;
};

}

#endif // __XNLINKMSGPARSER_H__