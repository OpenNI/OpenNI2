#ifndef __ICONNECTIONFACTORY_H__
#define __ICONNECTIONFACTORY_H__

#include <XnStatus.h>
#include <XnPlatform.h>

namespace xn
{

class ISyncIOConnection;
class IOutputConnection;
class IAsyncInputConnection;

class IConnectionFactory
{
public:
	virtual ~IConnectionFactory() {}
	virtual XnStatus Init(const XnChar* strConnString) = 0;
	virtual void Shutdown() = 0;
	virtual XnBool IsInitialized() const = 0;
	virtual XnUInt16 GetNumInputDataConnections() const = 0;
	virtual XnUInt16 GetNumOutputDataConnections() const = 0;

	/** The pointer returned by GetControlConnection() belongs to the connection factory and 
	    must not be deleted by caller. **/
	virtual XnStatus GetControlConnection(ISyncIOConnection*& pConnection) = 0;

	virtual XnStatus CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConnection) = 0;
	virtual XnStatus CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConnection) = 0;
};

}

#endif // __ICONNECTIONFACTORY_H__
