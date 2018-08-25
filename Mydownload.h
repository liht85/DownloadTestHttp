#ifndef Mydownload___
#define Mydownload___

#include "stdafx.h"
#define MAX_RECV_LEN           100   // ÿ�ν�������ַ�������.
#define MAX_PENDING_CONNECTS   4     // �ȴ����еĳ���.

class  CHttpSect
{
public:
    CString  szProxyAddr;     // ���������ַ.
    CString  szHostAddr;      // Host��ַ.
    int      nProxyPort;      // �������˿ں�.
    int      nHostPort;       // Host�˿ں�.
    CString  szHttpAddr;      // Http�ļ���ַ.
    CString  szHttpFilename;  // Http�ļ���.
    CString  szDesFilename;   // ���غ���ļ���.
	DWORD    nStart;          // �ָ����ʼλ��.
    DWORD    nEnd;            // �ָ����ʼλ��.
	DWORD    bProxyMode;      // ����ģ̬. 
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


//ʹ�ýӿڣ��������Զ�ʹ��join���ȴ�������ɡ�������ɺ��ֹͣһ��ʱ��
bool fnMyDownload(CString strUrl,
								 CString strWriteFileName,
								 unsigned long *& downloaded,
								 unsigned long & totalSize,
                                 CString strProxy = "",
								 int nProxyPort = 8080,
								 int nThread = 5
								 );


#endif