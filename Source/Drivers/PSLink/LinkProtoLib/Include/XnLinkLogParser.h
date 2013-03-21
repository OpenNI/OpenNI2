#ifndef _XNLINKLOGPARSER_H_
#define _XNLINKLOGPARSER_H_

#include "XnLinkMsgParser.h"
#include <XnHash.h>

namespace xn
{

class LinkLogParser : public LinkMsgParser
{
public:
	LinkLogParser();
	virtual ~LinkLogParser();

	void GenerateOutputBuffer(bool toCreate);

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);
	
	XnStatus WriteToLogFile( XnUInt8 fileID, const void* pData, XnUInt32 dataLength );
	XnStatus CloseLogFile( XnUInt8 fileID );
	XnStatus OpenLogFile( XnUInt8 fileID, const XnChar* fileName );
private:
	xnl::Hash<XnUInt8, XnDumpFile*> m_activeLogs;
	bool m_copyDataToOutput;
};

}

#endif // _XNLINKLOGPARSER_H_
