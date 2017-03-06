#pragma once

// CClient command target

class CClient : public CSocket
{
public:
	CClient();
	virtual ~CClient();

    virtual void OnReceive(int nErrorCode);
    virtual void OnClose(int nErrorCode);
};


