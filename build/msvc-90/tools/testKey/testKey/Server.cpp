// Server.cpp : implementation file
//

#include "stdafx.h"
#include "testKey.h"
#include "Server.h"
#include "Client.h"


// CServer

CServer::CServer()
{
}

CServer::~CServer()
{
}

void CServer::OnAccept(int nErrorCode)
{
    CClient * c = new CClient();
    this->Accept(*c);
}


// CServer member functions
