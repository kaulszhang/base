#pragma once

// CServer command target

class CServer : public CSocket
{
public:
	CServer();
	virtual ~CServer();


    virtual void OnAccept(int nErrorCode);
};


