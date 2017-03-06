#!/usr/bin/env python
# coding=utf-8

import socket
import urllib2
import json
import base64
import urlparse 
import string
import threading
import struct
import random

URL = 'http://remote.mobile.pptv.com/query?ver=2.0&from=1&uuid=356551040107528%7C78%3Ad6%3Af0%3A0d%3A3c%3Abb&platform=android3&osv=2.2.2&sv=2.3.7'
UUID = '356551040107528|78:d6:f0:0d:3c:bb'

def MatchServerRequest(matchurl):
    request = urllib2.Request(matchurl)
    response = urllib2.urlopen(request)
    resptext = response.read()
    print resptext, '\r\n'
    s = json.loads(resptext)
    print s, "\r\n"
    return s
def ProcessDeviceList(s):
    print s.keys(), "\r\n"
    print s["data"]["chathost"], "\r\n"
    print s["data"]["list"][2], "\r\n"
    result = s["result"]
    msg = s["message"]
    data = s["data"]
    if result == 0:
        chathost = s["data"]["chathost"]
        list = s["data"]["list"]
        for dev in list:
            print "---------------"
            print dev["state"]
            print dev["uuid"]
            print dev["name"]
            print "---------------"
        up = urlparse.urlparse(chathost)
        print up 
        host = up.netloc
        query = up.query
        print host, query
        print host.split(':')
        print query.split('=')
        tmp = host.split(':')
        host = tmp[0]
        port = tmp[1]
        tmp = query.split('=')
        roomid = tmp[1]
        return {"host":host, "port":port, "roomid":roomid}
 
def HandShake(hostip, port, roomid):
    print hostip
    print roomid
    reqstr = 'POST /socket.io/1/?roomid='+roomid+' HTTP/1.0\r\n'
    reqstr = reqstr + 'Host: '+hostip+'\r\n'
    reqstr = reqstr + 'Accept: */*\r\n'
    reqstr = reqstr + 'Connection: close\r\n\r\n'
    print reqstr
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect((hostip, string.atoi(port)))
    s.sendall(reqstr)
    data = s.recv(102400)
    s.close()
    print data
    return data
    
def ConnectChat(hostip, port, data):
    tmp = data.split("\r\n")
    tmp = tmp[len(tmp)-1]
    tmp = tmp.split(":")
    key = tmp[0]
    timeout = tmp[1]
    
    wskey0 = random.randint(1, 65545)
    wskey1 = random.randint(1, 65545)
    wskey2 = random.randint(1, 65545)
    wskey3 = random.randint(1, 65545)
    
    wskey  = chr((wskey0 & 0xff000000) >> 24)
    wskey += chr((wskey0 & 0x00ff0000) >> 16)
    wskey += chr((wskey0 & 0x0000ff00) >> 8)
    wskey += chr((wskey0 & 0x000000ff))
    
    wskey += chr((wskey1 & 0xff000000) >> 24)
    wskey += chr((wskey1 & 0x00ff0000) >> 16)
    wskey += chr((wskey1 & 0x0000ff00) >> 8)
    wskey += chr((wskey1 & 0x000000ff))
    
    wskey += chr((wskey2 & 0xff000000) >> 24)
    wskey += chr((wskey2 & 0x00ff0000) >> 16)
    wskey += chr((wskey2 & 0x0000ff00) >> 8)
    wskey += chr((wskey2 & 0x000000ff))
    
    wskey += chr((wskey3 & 0xff000000) >> 24)
    wskey += chr((wskey3 & 0x00ff0000) >> 16)
    wskey += chr((wskey3 & 0x0000ff00) >> 8)
    wskey += chr((wskey3 & 0x000000ff))
    
    wskey = base64.b64encode(wskey)
    reqstr1 = 'GET /socket.io/1/websocket/'+key+' HTTP/1.1\r\n'
    reqstr1 = reqstr1 + 'Connection: Upgrade\r\n'
    reqstr1 = reqstr1 + 'Host: '+hostip+":"+str(port)+'\r\n'
    reqstr1 = reqstr1 + 'Upgrade: websocket\r\n'
    reqstr1 = reqstr1 + 'Sec-WebSocket-Key: '+wskey+'\r\n'
    reqstr1 = reqstr1 + 'Sec-WebSocket-Version: 13\r\n'
    reqstr1 = reqstr1 + 'User-Agent: WebSocket++/0.2.2dev\r\n\r\n'
    print reqstr1
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect((hostip, string.atoi(port)))
    s.sendall(reqstr1)
    data = s.recv(102400)
    print data
    tmp = data.split("\r\n")
    print tmp
    tmp = tmp[len(tmp)-1]
    print tmp
    return s

#接收消息,并且解包
def RecvData(nNum,client):
    rawstr=""
    try:
        pData = client.recv(nNum)
        if not len(pData):
            return False
    except:
        return False
    else:
        if ord(pData[0]) != 129:
            if  ord(pData[0]) != 136:
                return False
            return False

        code_length = ord(pData[1]) & 127
        if (ord(pData[1]) & 0x80) == 0:
            if code_length <= 125:
                for i in range(code_length):
                    rawstr += pData[2 + i]
            elif code_length == 126:
                code_length = (ord(pData[2]) << 8) | ord(pData[3])
                for i in range(code_length):
                    rawstr += pData[4 + i]
            elif code_length == 127:
                code_length = (ord(pData[2])<<56)|(ord(pData[3])<<48)|(ord(pData[4])<<40)|(ord(pData[5])<<32)|(ord(pData[6])<<24)|(ord(pData[7])<<16)|(ord(pData[8])<<8)|(ord(pData[9])<<56);
                for i in range(code_length):
                    rawstr += pData[10 + i]
            if code_length > len(pData):
                return False
            return rawstr
        index_first_mask = 0
        mask0 = 0
        mask1 = 0
        mask2 = 0
        mask3 = 0
        if code_length <= 125:
            index_first_mask = 2
            mask0 = ord(pData[2])
            mask1 = ord(pData[3])
            mask2 = ord(pData[4])
            mask3 = ord(pData[5])
        elif code_length == 126:
            index_first_mask = 4;
            code_length = (ord(pData[2]) << 8)|(pData[3])
            mask0 = ord(pData[4])
            mask1 = ord(pData[5])
            mask2 = ord(pData[6])
            mask3 = ord(pData[7])
        elif code_length == 127:
            index_first_mask = 10;
            code_length = (ord(pData[2]) << 56)|(ord(pData[3])<<48)|(ord(pData[4])<<40)|(ord(pData[5])<<32)|(ord(pData[6])<<24)|(ord(pData[7])<<16)|(ord(pData)[8]<<8)|(ord(pData[9])) ;
            mask0 = ord(pData[10]);
            mask1 = ord(pData[11]);
            mask2 = ord(pData[12]);
            mask3 = ord(pData[13]);

        if code_length > len(pData):
        	return False;	

        index_first_data_byte = index_first_mask + 4
        mask = [mask0, mask1, mask2,mask3]
        j = 0
        for i in range(index_first_data_byte, code_length):
            rawstr += chr(ord(pData[ i ]) ^ mask[ j % 4])
            j = j + 1
        return rawstr
   
#打包发送数据
def SendData(pData,client):
    mask = random.randint(1, 65535)
#    print mask
    msk0 = (mask & 0xFF000000) >> 24;
    msk1 = (mask & 0x00FF0000) >> 16;
    msk2 = (mask & 0x0000FF00) >> 8;
    msk3 = (mask & 0x000000FF);
    msk = [msk0, msk1, msk2, msk3]
#    print str(msk0), str(msk1), str(msk2), str(msk3)
    if(pData == False):
        return False
    else:
        pData = str(pData)
        
    token = "\x81"
    length = len(pData)
    if length < 126:
        token += chr(length | 0x80)
        token += chr(msk0)
        token += chr(msk1)
        token += chr(msk2)
        token += chr(msk3)
    elif length <= 0xFFFF:
        token += chr(126 | 0x80)
        token += chr(((length >> 8) & 255))
        token += chr((length & 255))
        token += chr(msk0)
        token += chr(msk1)
        token += chr(msk2)
        token += chr(msk3)
        token += struct.pack("!BHi", 126, length, mask)
    else:
        token += chr(127 | 0x80)
        token += chr(((length >> 56) & 255))
        token += chr(((length >> 48) & 255))
        token += chr(((length >> 40) & 255))
        token += chr(((length >> 32) & 255))
        token += chr(((length >> 24) & 255))
        token += chr(((length >> 16) & 255))
        token += chr(((length >> 8) & 255))
        token += chr((length & 255))
        token += chr(msk0)
        token += chr(msk1)
        token += chr(msk2)
        token += chr(msk3)
        token += struct.pack("!BHi", 126, length, mask)
    i = 0
    for c in pData:
        token += chr(ord(c)^msk[i%4])
        i = i + 1
#    print "SEND:"
#    print token

    client.send(token)
    
    return True


#处理数据的线程
def DoRemoteCommand(connection):
    while 1:
        szBuf = RecvData(8196,connection)
        if(szBuf == False):
            break
        print "RECV thread:" 
        print szBuf 
        if szBuf == "2::":
            tmp = "2::"
            SendData(tmp, connection)

dic_hpr = ProcessDeviceList(MatchServerRequest(URL))
hostip = socket.gethostbyname(dic_hpr["host"])
data = HandShake(hostip, dic_hpr["port"], dic_hpr["roomid"])
s = ConnectChat(hostip, dic_hpr["port"], data)
tmp = '1::/chat'

print "SEND:"
print tmp
SendData(tmp, s)
data = s.recv(102400)
print "RECV:"
print data

t = threading.Thread(target=DoRemoteCommand,args=(s,))
t.start()

tmp = "5::/chat:{\"args\":{\"deviceid\":\""+UUID+"\",\"type\":\"member\"},\"name\":\"join\"}"
print "SEND:"
print tmp
SendData(tmp, s)
#data = s.recv(102400)
#print "RECV:"
#print data

t.join()
