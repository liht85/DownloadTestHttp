// Mydownload.cpp : ���� DLL Ӧ�ó������ڵ㡣

#include "MyDownload.h"
#include <sys/stat.h>   // ״̬��ʾͷ�ļ�.
#include "ChineseCode.h"

#include <iostream>
using namespace std;

#ifdef _MANAGED
#pragma managed(pop)
#endif


int CHttpGet::m_nCount;
DWORD CHttpGet::m_nFileLength = 0;
//unsigned long CHttpGet::m_downloaded = 0;

unsigned long rdownloaded = 0;
void ParseURL(CString URL,CString &host,CString &path,CString &filename)
{
   URL.TrimLeft();
   URL.TrimRight();
   CString str=URL;
   CString strFind=_T("http://");
   int n=str.Find(strFind);
   if(n!=-1){
      str.Delete(0, n+strFind.GetLength());
   }

   n=str.Find('/');
   host = str.Left(n);
   n=URL.ReverseFind('/');
   path = URL.Left(n+1);
   filename = URL.Right(URL.GetLength()-n-1);
}

// ���ǵ���������һ��ʾ����
bool fnMyDownload(CString strUrl,
								 CString strWriteFileName,
								 unsigned long *& downloaded,
								 unsigned long & totalSize,
                                 CString strProxy,
								 int nProxyPort,
								 int nThread
								 )
{
	CHttpGet b;
	CString strHostAddr;
	CString strHttpAddr;
	CString strHttpFilename;

	//���ڸ���ʵʱ��������
	downloaded = &rdownloaded;

	ParseURL(strUrl,strHostAddr,strHttpAddr,strHttpFilename);
	strWriteFileName += strHttpFilename;	//�����ļ������Ǳ���GB2312
	
	//httpĬ����UTF8���룬���԰����ı���ΪUTF8
	ChineseCode chineseCode;
	string remoteFileName;		//�ļ���
	chineseCode.GB2312ToUTF_8(remoteFileName, (char*)strHttpFilename.GetBuffer(strHttpFilename.GetLength()), strHttpFilename.GetLength());
	strHttpFilename = remoteFileName.c_str();
	string remoteFilePath;		//���������ļ�·��  "http://www.123.com/123/"
	chineseCode.GB2312ToUTF_8(remoteFilePath, (char*)strHttpAddr.GetBuffer(strHttpAddr.GetLength()), strHttpAddr.GetLength());
	strHttpAddr = remoteFilePath.c_str();

	//cout<<strHttpAddr.GetBuffer(strHttpAddr.GetLength())<<endl;
    
	if(strProxy != ""){
		if(!b.HttpDownLoadProxy(strProxy,nProxyPort,strHostAddr,strHttpAddr,strHttpFilename,strWriteFileName,nThread,totalSize))
			return false;
	}
	else{
		if(!b.HttpDownLoadNonProxy(strHostAddr,strHttpAddr,strHttpFilename,strWriteFileName,nThread,totalSize))
			return false;
	}
	return true;
}

   


//---------------------------------------------------------------------------
CHttpGet::CHttpGet()
{
	m_nFileLength=0;
}

//---------------------------------------------------------------------------
CHttpGet::~CHttpGet()
{
}

//---------------------------------------------------------------------------
BOOL CHttpGet::HttpDownLoadProxy(
			CString strProxyAddr,
			int nProxyPort,
		    CString strHostAddr,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			DWORD &totalSize
			)
{
	SOCKET hSocket;
	hSocket=ConnectHttpProxy(strProxyAddr,nProxyPort);
	if(hSocket == INVALID_SOCKET) return 1;

	// �����ļ�ͷ�������ļ���С.
	SendHttpHeader(hSocket,strHostAddr,strHttpAddr,strHttpFilename,0);
 	closesocket(hSocket);

	totalSize = CHttpGet::m_nFileLength;


    if(!HttpDownLoad(strProxyAddr,nProxyPort,strHostAddr,80,strHttpAddr,strHttpFilename,strWriteFileName,nSectNum,true))
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------
BOOL CHttpGet::HttpDownLoadNonProxy(
		    CString strHostAddr,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			DWORD &totalSize)
{
	int nHostPort=80;

	SOCKET hSocket;
	hSocket=ConnectHttpNonProxy(strHostAddr,nHostPort);
	if(hSocket == INVALID_SOCKET) return 1;
	// �����ļ�ͷ�������ļ���С.
	SendHttpHeader(hSocket,strHostAddr,strHttpAddr,strHttpFilename,0);
 	closesocket(hSocket);
	totalSize = CHttpGet::m_nFileLength;

    if(!HttpDownLoad(TEXT(""),80,strHostAddr,nHostPort,strHttpAddr,strHttpFilename,strWriteFileName,nSectNum,false))
			return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------
BOOL CHttpGet::HttpDownLoad(
			CString strProxyAddr,
			int nProxyPort,
		    CString strHostAddr,
			int nHostPort,
			CString strHttpAddr,
			CString strHttpFilename,
			CString strWriteFileName,
			int nSectNum,
			BOOL bProxy)
{
	ASSERT(nSectNum>0 && nSectNum<=50);

	m_nCount=0;                                    // �Ѽ���������.
	sectinfo=new CHttpSect[nSectNum];              // ����Ϣ�ṹ�����ڴ�.
	DWORD nSize= m_nFileLength/nSectNum;           // ����ָ�εĴ�С.

	int i;
	// �����߳�.���ܳ���50����
	CWinThread* pthread[50];
	for(i=0;i<nSectNum;i++)
	{
	    sectinfo[i].szProxyAddr=strProxyAddr;      // �����������ַ.
	    sectinfo[i].nProxyPort =nProxyPort;		   // Host��ַ.
	    sectinfo[i].szHostAddr =strHostAddr;       // Http�ļ���ַ.
	    sectinfo[i].nHostPort  =nHostPort;		   // Http�ļ���.
	    sectinfo[i].szHttpAddr =strHttpAddr;       // �������˿ں�.
	    sectinfo[i].szHttpFilename=strHttpFilename;// Host�˿ں�.
		sectinfo[i].bProxyMode=bProxy;		       // ����ģ̬. 


		// ������ʱ�ļ���.
		CString strTempFileName;
		strTempFileName.Format("%s_%d",strWriteFileName, i);
        sectinfo[i].szDesFilename=strTempFileName; // ���غ���ļ���.
	
		if(i<nSectNum-1){
			sectinfo[i].nStart=i*nSize;            // �ָ����ʼλ��.
			sectinfo[i].nEnd=(i+1)*nSize;          // �ָ����ֹλ��.
		}
		else{
			sectinfo[i].nStart=i*nSize;            // �ָ����ʼλ��.
			//sectinfo[i].nStart=1744896;            // �ָ����ʼλ��.
			sectinfo[i].nEnd=m_nFileLength;        // �ָ����ֹλ��.
		}

		// AfxBeginThread(ThreadDownLoad,&sectinfo[i],THREAD_PRIORITY_HIGHEST);
		pthread[i] = AfxBeginThread(ThreadDownLoad,&sectinfo[i]);

	}

	HANDLE hThread[50];
	for(int ii = 0 ; ii < nSectNum ; ii++)
		hThread[ii] = pthread[ii]->m_hThread;

	// �ȴ������߳̽���.
	WaitForMultipleObjects(nSectNum,hThread,TRUE,INFINITE);

	//��ĳ���ļ���δ�����꣬���ش���Ӧ�����ϲ�������������ء�
	if(m_nCount != nSectNum)
		return FALSE;
	
	FILE *fpwrite;

 	// ��д�ļ�.
	if((fpwrite=fopen(strWriteFileName,"wb"))==NULL){
  		return FALSE;
	}

	for(i=0;i<nSectNum;i++){
		FileCombine(&sectinfo[i],fpwrite);
	}

	fclose(fpwrite);

	delete[] sectinfo;
    
    return TRUE;
}

//---------------------------------------------------------------------------
BOOL CHttpGet::FileCombine(CHttpSect *pInfo, FILE *fpwrite)
{	
	FILE *fpread;
	
	// ���ļ�.
	if((fpread=fopen(pInfo->szDesFilename,"rb"))==NULL)
		return FALSE;

	DWORD nPos=pInfo->nStart;
	
	// �����ļ�дָ����ʼλ��.
	fseek(fpwrite,nPos,SEEK_SET);
	
	int c;
	// ���ļ�����д�뵽���ļ�.		
	while((c=fgetc(fpread))!=EOF)
	{
		fputc(c,fpwrite);
		nPos++;
		if(nPos==pInfo->nEnd) break;
	}
	
	fclose(fpread);
	DeleteFile(pInfo->szDesFilename);

	return TRUE;
}

//---------------------------------------------------------------------------
UINT CHttpGet::ThreadDownLoad(void* pParam)
{
	CHttpSect *pInfo=(CHttpSect*)pParam;
	SOCKET hSocket;

	if(pInfo->bProxyMode){	
		hSocket=ConnectHttpProxy(pInfo->szProxyAddr,pInfo->nProxyPort);
	}
	else{
		hSocket=ConnectHttpNonProxy(pInfo->szHostAddr,pInfo->nHostPort);
	}
	if(hSocket == INVALID_SOCKET) return 1;


	// ������ʱ�ļ���С��Ϊ�˶ϵ�����
	DWORD nFileSize=myfile.GetFileSizeByName(pInfo->szDesFilename);
    DWORD nSectSize=(pInfo->nEnd)-(pInfo->nStart);

	// �˶����������.
	if(nFileSize==nSectSize){
		//mj
		printf("�ļ����سɹ������ؽ�����\n");                //�����������д��Ϣ
		//mj

		TRACE("�ļ����������!\n");                                     
		CHttpGet::m_nCount++;  // ����.
		return 0;
	}

    FILE *fpwrite=myfile.GetFilePointer(pInfo->szDesFilename);
	if(!fpwrite) return 1;

    // �������ط�Χ.
	SendHttpHeader(hSocket,pInfo->szHostAddr,pInfo->szHttpAddr,
		      pInfo->szHttpFilename,pInfo->nStart+nFileSize);
	
	// �����ļ�дָ����ʼλ�ã��ϵ�����
	fseek(fpwrite,nFileSize,SEEK_SET);

	DWORD nLen; 
	DWORD nSumLen=0; 
	char szBuffer[1024];

	while(1)
	{
		if(nSumLen>=nSectSize-nFileSize) break;
		nLen=recv(hSocket,szBuffer,sizeof(szBuffer),0);
		
		//ԭ�Ӳ���������ͬ����
		rdownloaded += nLen;
		
		if (nLen == SOCKET_ERROR){
			TRACE("Read error!\n");
			fclose(fpwrite);
			return 1;
		}

  		if(nLen==0) break;
		nSumLen +=nLen;
		TRACE("%d\n",nLen);

		// ������д���ļ�.		
		fwrite(szBuffer,nLen,1,fpwrite);
	}

    
	fclose(fpwrite);      // �ر�д�ļ�.
	closesocket(hSocket); // �ر��׽���.
	CHttpGet::m_nCount++; // ����.
	return 0;
}

//---------------------------------------------------------------------------
SOCKET CHttpGet::ConnectHttpProxy(CString strProxyAddr,int nPort)
{
	TRACE("���ڽ�������\n");
	
  	CString sTemp;
	char cTmpBuffer[1024];
	SOCKET hSocket=dealsocket.GetConnect(strProxyAddr,nPort);

    if(hSocket == INVALID_SOCKET)
    {
		TRACE("����http������ʧ�ܣ�\n");
		return INVALID_SOCKET;
    }

	// ����CONNCET�������������������ںʹ��������Ӵ����������
	// ��ַ�Ͷ˿ڷ���strProxyAddr,nPort ����.
	sTemp.Format("CONNECT %s:%d HTTP/1.1\r\nUser-Agent:\
		           MyApp/0.1\r\n\r\n",strProxyAddr,nPort);
	
	if(!SocketSend(hSocket,sTemp))
	{
		TRACE("���Ӵ���ʧ��\n");
		return INVALID_SOCKET;
	}

	// ȡ�ô�����Ӧ��������Ӵ���ɹ��������������
	// ����"200 Connection established".
	int nLen=GetHttpHeader(hSocket,cTmpBuffer);
	sTemp=cTmpBuffer;
	if(sTemp.Find("HTTP/1.0 200 Connection established",0)==-1)
	{
		TRACE("���Ӵ���ʧ��\n");
		return INVALID_SOCKET;
	}

	TRACE(sTemp);
	TRACE("�����������\n");
	return hSocket; 
}

//---------------------------------------------------------------------------
SOCKET CHttpGet::ConnectHttpNonProxy(CString strHostAddr,int nPort)
{
	TRACE("���ڽ�������\n");
    SOCKET hSocket=dealsocket.GetConnect(strHostAddr,nPort);
	if(hSocket == INVALID_SOCKET)
		return INVALID_SOCKET;
    
	return hSocket;
}

//---------------------------------------------------------------------------
// ����: strHostAddr="www.aitenshi.com",
// strHttpAddr="http://www.aitenshi.com/bbs/images/",
// strHttpFilename="pics.jpg".
BOOL CHttpGet::SendHttpHeader(SOCKET hSocket,CString strHostAddr,
				CString strHttpAddr,CString strHttpFilename,DWORD nPos)
{
	// ��������. 
	static CString sTemp;
	char cTmpBuffer[1024];

	// Line1: �����·��,�汾.
	sTemp.Format("GET %s%s HTTP/1.1\r\n",strHttpAddr,strHttpFilename);
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// Line2:����.
	sTemp.Format("Host: %s\r\n",strHostAddr);
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// Line3:���յ���������.
	sTemp.Format("Accept: \r\n");
	if(!SocketSend(hSocket,sTemp)) return FALSE;
	
	// Line4:�ο���ַ.
    sTemp.Format("Referer: %s\r\n",strHttpAddr); 
	if(!SocketSend(hSocket,sTemp)) return FALSE;
		
	// Line5:���������.
	sTemp.Format("User-Agent: Mozilla/4.0 \
		(compatible; MSIE 5.0; Windows NT; DigExt; DTS Agent;)\r\n");

	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// ����. Range ��Ҫ���ص����ݷ�Χ������������Ҫ.
	sTemp.Format("Range: bytes=%d-\r\n",nPos);
	if(!SocketSend(hSocket,sTemp)) return FALSE;
	
	// LastLine: ����.
	sTemp.Format("\r\n");
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// ȡ��httpͷ.
	int i=GetHttpHeader(hSocket,cTmpBuffer);
	if(!i)
	{
		TRACE("��ȡHTTPͷ����!\n");
		return 0;
	}
	
	// ���ȡ�õ�httpͷ����404�����������ʾ���ӳ�����.
	sTemp=cTmpBuffer;
	if(sTemp.Find("404")!=-1) return FALSE;

	// �õ��������ļ��Ĵ�С.
	m_nFileLength=GetFileLength(cTmpBuffer);

	// ��ΪTRACE()���������ַ�������Ϊ255.
    TRACE(CString(cTmpBuffer).GetBuffer(200));

	return TRUE;
}

//---------------------------------------------------------------------------
DWORD CHttpGet::GetHttpHeader(SOCKET sckDest,char *str)
{
	BOOL bResponsed=FALSE;
	DWORD nResponseHeaderSize;
	
	if(!bResponsed)
	{
		char c = 0;
		int nIndex = 0;
		BOOL bEndResponse = FALSE;
		while(!bEndResponse && nIndex < 1024)
		{
			recv(sckDest,&c,1,0);
			str[nIndex++] = c;
			if(nIndex >= 4)
			{
				if( str[nIndex - 4] == '\r' && 
					str[nIndex - 3] == '\n' && 
					str[nIndex - 2] == '\r' && 
					str[nIndex - 1] == '\n')
					bEndResponse = TRUE;
			}
		}

		str[nIndex]=0;
		nResponseHeaderSize = nIndex;
		bResponsed = TRUE;
	}
	
	return nResponseHeaderSize;
}

//---------------------------------------------------------------------------
DWORD CHttpGet:: GetFileLength(char *httpHeader)
{
	CString strHeader;
	CString strFind=_T("Content-Length:");
	int local;
	strHeader=CString(httpHeader);
	local=strHeader.Find(strFind,0);
	local+=strFind.GetLength();
	strHeader.Delete(0,local);
	local=strHeader.Find("\r\n");
	
	if(local!=-1){
      strHeader=strHeader.Left(local);
	}

	return atoi(strHeader);
}

//---------------------------------------------------------------------------
BOOL CHttpGet::SocketSend(SOCKET sckDest,CString szHttp)
{
	int iLen=szHttp.GetLength();
	if(send (sckDest,szHttp,iLen,0)==SOCKET_ERROR)
	{
		closesocket(sckDest);
		TRACE("��������ʧ��!\n");
		return FALSE;
	}
	
	return TRUE;
}


CDealSocket dealsocket;

//---------------------------------------------------------------------------
CDealSocket::CDealSocket()
{
	// �׽��ֳ�ʼ��.
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	
	// ��ʼ��WinSock.
	if (WSAStartup(wVersionRequested, &wsaData)!=0)
	{
		TRACE("WSAStartup\n");
		return;
	}

	// ��� WinSock �汾.
	if (wsaData.wVersion != wVersionRequested)
	{
		TRACE("WinSock version not supported\n");
		WSACleanup();
		return;
	}
}

//---------------------------------------------------------------------------
CDealSocket::~CDealSocket()
{
	// �ͷ�WinSock.
	WSACleanup();
}

//---------------------------------------------------------------------------
CString CDealSocket::GetResponse(SOCKET hSock)
{
	char szBufferA[MAX_RECV_LEN];  	// ASCII�ַ���. 
	int	iReturn;					// recv�������ص�ֵ.
	
	CString szError;
	CString strPlus;
	strPlus.Empty();

	while(1)
	{
		// ���׽��ֽ�������.
		iReturn = recv (hSock, szBufferA, MAX_RECV_LEN, 0);
		szBufferA[iReturn]=0;
		strPlus +=szBufferA;

		TRACE(szBufferA);

		if (iReturn == SOCKET_ERROR)
		{
			szError.Format("No data is received, recv failed. Error: %d",
				WSAGetLastError ());
			MessageBox (NULL, szError, TEXT("Client"), MB_OK);
			break;
		}
		else if(iReturn<MAX_RECV_LEN){
			TRACE("Finished receiving data\n");
			break;
		}
	}

	return strPlus;
}

//---------------------------------------------------------------------------
SOCKET CDealSocket::GetConnect(CString host ,int port)
{
    SOCKET hSocket;
	SOCKADDR_IN saServer;          // �������׽��ֵ�ַ.
	PHOSTENT phostent = NULL;	   // ָ��HOSTENT�ṹָ��.

	// ����һ���󶨵���������TCP/IP�׽���.
	if ((hSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		TRACE("Allocating socket failed. Error: %d\n",WSAGetLastError ());
		return INVALID_SOCKET;
	}
	
	// ʹ��TCP/IPЭ��.
	saServer.sin_family = AF_INET;
	
	// ��ȡ��������ص���Ϣ.
	if ((phostent = gethostbyname (host)) == NULL) 
	{
		TRACE("Unable to get the host name. Error: %d\n",WSAGetLastError ());
		closesocket (hSocket);
		return INVALID_SOCKET;
	}

	// ���׽���IP��ַ��ֵ.
	memcpy ((char *)&(saServer.sin_addr), 
		phostent->h_addr, 
		phostent->h_length);
	
	// �趨�׽��ֶ˿ں�.
	saServer.sin_port =htons (port); 

	// ���������������׽�������.
	if (connect (hSocket,(PSOCKADDR) &saServer, 
		sizeof (saServer)) == SOCKET_ERROR) 
	{
		TRACE("Connecting to the server failed. Error: %d\n",WSAGetLastError ());
		closesocket (hSocket);
		return INVALID_SOCKET;
	}

	return hSocket;
}

//---------------------------------------------------------------------------
SOCKET CDealSocket::Listening(int port)
{
	SOCKET ListenSocket = INVALID_SOCKET;	// �����׽���.
	SOCKADDR_IN local_sin;				    // �����׽��ֵ�ַ.
	
	// ����TCP/IP�׽���.
	if ((ListenSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
	{
		TRACE("Allocating socket failed. Error: %d\n",WSAGetLastError ());
		return INVALID_SOCKET;
	}
	
	// ���׽�����Ϣ�ṹ��ֵ.
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons (port); 
	local_sin.sin_addr.s_addr = htonl (INADDR_ANY);
	
	// ���б�����ַ������׽��ְ�.
	if (bind (ListenSocket, 
		(struct sockaddr *) &local_sin, 
		sizeof (local_sin)) == SOCKET_ERROR) 
	{
		TRACE("Binding socket failed. Error: %d\n",WSAGetLastError ());
		closesocket (ListenSocket);
		return INVALID_SOCKET;
	}
	
	// �����׽��ֶ��ⲿ���ӵļ���.
	if (listen (ListenSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
	{
		TRACE("Listening to the client failed. Error: %d\n",	
			WSAGetLastError ());
		closesocket (ListenSocket);
		return INVALID_SOCKET;
	}

	return ListenSocket;
}

CMyFile myfile;

//---------------------------------------------------------------------------
CMyFile::CMyFile()
{
}

//---------------------------------------------------------------------------
CMyFile::~CMyFile()
{
}

//---------------------------------------------------------------------------
BOOL CMyFile::FileExists(LPCTSTR lpszFileName)
{
	DWORD dwAttributes = GetFileAttributes(lpszFileName);
    if (dwAttributes == 0xFFFFFFFF)
        return FALSE;

	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		 ==	FILE_ATTRIBUTE_DIRECTORY)
	{
			return FALSE;
	}
	else{
		return TRUE;
	}
}

//---------------------------------------------------------------------------
FILE* CMyFile::GetFilePointer(LPCTSTR lpszFileName)
{
    FILE *fp;
	if(FileExists(lpszFileName)){
		// �������ļ�����д����.
		fp=fopen(lpszFileName,"r+b");
	}
	else{
		// �������ļ�����д����.
		fp=fopen(lpszFileName,"w+b");
	}

	return fp;
}

//---------------------------------------------------------------------------
DWORD CMyFile::GetFileSizeByName(LPCTSTR lpszFileName)
{
	if(!FileExists(lpszFileName)) return 0;
	struct _stat ST; 
	// ��ȡ�ļ�����.
	_stat(lpszFileName, &ST);
	UINT nFilesize=ST.st_size;
	return nFilesize; 
}

//---------------------------------------------------------------------------
// ��ȫ���ļ�������ȡ���ļ���.
CString CMyFile::GetShortFileName(LPCSTR lpszFullPathName)
{
   CString strFileName=lpszFullPathName;
   CString strShortName;
   strFileName.TrimLeft();
   strFileName.TrimRight();
   int n=strFileName.ReverseFind('/');
   strShortName=strFileName.Right(strFileName.GetLength()-n-1);
   return strShortName;
}