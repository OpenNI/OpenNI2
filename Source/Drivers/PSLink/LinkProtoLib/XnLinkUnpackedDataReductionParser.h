#ifndef __XNLINKUNPACKEDDATAREDUCTIONPARSER_H__
#define __XNLINKUNPACKEDDATAREDUCTIONPARSER_H__

#include <XnStatus.h>
#include "XnLinkDefs.h"
#include "XnLinkMsgParser.h"

enum XnLinkFragmentation;
namespace xn
{

class LinkUnpackedDataReductionParser : public LinkMsgParser
{
public:
	LinkUnpackedDataReductionParser ();
	virtual ~LinkUnpackedDataReductionParser();

protected:
	virtual XnStatus ParsePacketImpl(XnLinkFragmentation fragmentation,
									 const XnUInt8* pSrc, 
	                                 const XnUInt8* pSrcEnd, 
									 XnUInt8*& pDst, 
									 const XnUInt8* pDstEnd);


	static const XnUInt16 FACTOR;

};

}

#endif // __XNLINKUNPACKEDDATAREDUCTIONPARSER_H__
