#ifndef __XNSERVERLINUXUSBCONNECTIONFACTORY_H__
#define __XNSERVERLINUXUSBCONNECTIONFACTORY_H__

#include "IConnectionFactory.h"
#include "XnServerUSBLinuxControlEndpoint.h"
#include <XnOS.h>

struct XnUSBDevice;

namespace xn
{

class ServerUSBLinuxConnectionFactory : public IConnectionFactory
{
public:
	ServerUSBLinuxConnectionFactory();
	virtual ~ServerUSBLinuxConnectionFactory();
	
	virtual XnStatus Init(const XnChar* strConnString);
	virtual void Shutdown();
	virtual XnBool IsInitialized() const;
	virtual XnUInt16 GetNumInputDataConnections() const;
	virtual XnUInt16 GetNumOutputDataConnections() const;

	/** The pointer returned by GetControlConnection() belongs to the connection factory and 
	    must not be deleted by caller. **/
	virtual XnStatus GetControlConnection(ISyncIOConnection*& pConn);

	virtual XnStatus CreateOutputDataConnection(XnUInt16 nID, IOutputConnection*& pConn);
	virtual XnStatus CreateInputDataConnection(XnUInt16 nID, IAsyncInputConnection*& pConn);

private:
	ServerUSBLinuxControlEndpoint m_controlEndpoint;
	XnUSBDevice* m_pDevice;
	XnBool m_bInitialized;
};

}

#endif // __XNSERVERLINUXUSBCONNECTIONFACTORY_H__
