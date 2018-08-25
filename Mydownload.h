#ifndef Mydownload___
#define Mydownload___

#include "stdafx.h"
#define MAX_RECV_LEN           100   // 每次接收最大字符串长度.
#define MAX_PENDING_CONNECTS   4     // 等待队列的长度.

class  CHttpSect
{
public:
    CString  szProxyAddr;     // 理服务器地址.
    CString  szHostAddr;      // Host地址.
    int      nProxyPort;      // 代理服务端口号.
    int      nHostPort;       // Host端口号.
    CString  szHttpAddr;      // Http文件地址.
    CString  szHttpFilename;  // Http文件名.
    CString  szDesFilename;   // 下载后的文件名.
	DWORD    nStart;          // 分割的起始位置.
    DWORD    nEnd;            // 分割的起始位置.
	DWORD    bProxyMode;      // 下载模态. 
};

class  CHttpGet  
{
public:
	CHttpGet();
	virtual ~CHttpGet();
	//static unsigned long m_downloaded;

private:
	CHttpSect *sectinfo;
	static int m_nCount;
	static UINT ThreadDownLoad(void* pParam);

public:
	static DWORD m_nFileLength;

private:
	static SOCKET ConnectHttpProxy(CString strProxyAddr,int nPort);
	static SOCKET ConnectHttpNonProxy(CString strHostAddr,int nPort);
	static BOOL SendHttpHeader(SOCKET hSocket,CString strHostAddr,
				CString strHttpAddr,CString strHttpFilename,DWORD nPos);
	static DWORD GetHttpHeader(SOCKET sckDest,char *str);
	static DWORD GetFileLength(char *httpHeader);
	static BOOL SocketSend(SOCKET sckDest,CString szHttp);

	BOOL FileCombine(CHttpSect *pInfo, FILE *fpwrite);

public:
	BOOL HttpDownLoadProxy(
			CString strProxyAddr,
			int nProxyPort,
		    CString strHostAddr,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			DWORD &totalSize);

	BOOL HttpDownLoadNonProxy(
		    CString strHostAddr,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			DWORD &totalSize);

	BOOL HttpDownLoad(
			CString strProxyAddr,
			int nProxyPort,
		    CString strHostAddr,
			int nHostPort,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			BOOL bProxy);
};



class CDealSocket  
{
public:
	CDealSocket();
	virtual ~CDealSocket();

public:
	SOCKET GetConnect(CString host ,int port);
	SOCKET Listening(int port);
	CString GetResponse(SOCKET hSock);
};

class CMyFile  
{
public:
	CMyFile();
	virtual ~CMyFile();

public:
	BOOL FileExists(LPCTSTR lpszFileName);
	FILE* GetFilePointer(LPCTSTR lpszFileName);
	DWORD GetFileSizeByName(LPCTSTR lpszFileName);
	CString GetShortFileName(LPCSTR lpszFullPathName);
};

extern CMyFile myfile;

extern CDealSocket dealsocket;


//使用接口，函数中自动使用join，等待下载完成。下载完成后会停止一段时间
bool fnMyDownload(CString strUrl,
								 CString strWriteFileName,
								 unsigned long *& downloaded,
								 unsigned long & totalSize,
                                 CString strProxy = "",
								 int nProxyPort = 8080,
								 int nThread = 5
								 );


#endif