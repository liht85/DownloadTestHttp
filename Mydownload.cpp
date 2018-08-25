// Mydownload.cpp : 定义 DLL 应用程序的入口点。

#include "MyDownload.h"
#include <sys/stat.h>   // 状态显示头文件.
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

// 这是导出函数的一个示例。
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

	//用于跟踪实时下载数据
	downloaded = &rdownloaded;

	ParseURL(strUrl,strHostAddr,strHttpAddr,strHttpFilename);
	strWriteFileName += strHttpFilename;	//本地文件名还是保留GB2312
	
	//http默认是UTF8编码，所以把中文编码为UTF8
	ChineseCode chineseCode;
	string remoteFileName;		//文件名
	chineseCode.GB2312ToUTF_8(remoteFileName, (char*)strHttpFilename.GetBuffer(strHttpFilename.GetLength()), strHttpFilename.GetLength());
	strHttpFilename = remoteFileName.c_str();
	string remoteFilePath;		//服务器的文件路径  "http://www.123.com/123/"
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

	// 发送文件头，计算文件大小.
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
	// 发送文件头，计算文件大小.
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

	m_nCount=0;                                    // 把计数器清零.
	sectinfo=new CHttpSect[nSectNum];              // 给信息结构申请内存.
	DWORD nSize= m_nFileLength/nSectNum;           // 计算分割段的大小.

	int i;
	// 创建线程.不能超过50个。
	CWinThread* pthread[50];
	for(i=0;i<nSectNum;i++)
	{
	    sectinfo[i].szProxyAddr=strProxyAddr;      // 代理服务器地址.
	    sectinfo[i].nProxyPort =nProxyPort;		   // Host地址.
	    sectinfo[i].szHostAddr =strHostAddr;       // Http文件地址.
	    sectinfo[i].nHostPort  =nHostPort;		   // Http文件名.
	    sectinfo[i].szHttpAddr =strHttpAddr;       // 代理服务端口号.
	    sectinfo[i].szHttpFilename=strHttpFilename;// Host端口号.
		sectinfo[i].bProxyMode=bProxy;		       // 下载模态. 


		// 计算临时文件名.
		CString strTempFileName;
		strTempFileName.Format("%s_%d",strWriteFileName, i);
        sectinfo[i].szDesFilename=strTempFileName; // 下载后的文件名.
	
		if(i<nSectNum-1){
			sectinfo[i].nStart=i*nSize;            // 分割的起始位置.
			sectinfo[i].nEnd=(i+1)*nSize;          // 分割的终止位置.
		}
		else{
			sectinfo[i].nStart=i*nSize;            // 分割的起始位置.
			//sectinfo[i].nStart=1744896;            // 分割的起始位置.
			sectinfo[i].nEnd=m_nFileLength;        // 分割的终止位置.
		}

		// AfxBeginThread(ThreadDownLoad,&sectinfo[i],THREAD_PRIORITY_HIGHEST);
		pthread[i] = AfxBeginThread(ThreadDownLoad,&sectinfo[i]);

	}

	HANDLE hThread[50];
	for(int ii = 0 ; ii < nSectNum ; ii++)
		hThread[ii] = pthread[ii]->m_hThread;

	// 等待所有线程结束.
	WaitForMultipleObjects(nSectNum,hThread,TRUE,INFINITE);

	//有某个文件块未下载完，返回错误，应由最上层调用者重新下载。
	if(m_nCount != nSectNum)
		return FALSE;
	
	FILE *fpwrite;

 	// 打开写文件.
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
	
	// 打开文件.
	if((fpread=fopen(pInfo->szDesFilename,"rb"))==NULL)
		return FALSE;

	DWORD nPos=pInfo->nStart;
	
	// 设置文件写指针起始位置.
	fseek(fpwrite,nPos,SEEK_SET);
	
	int c;
	// 把文件数据写入到子文件.		
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


	// 计算临时文件大小，为了断点续传
	DWORD nFileSize=myfile.GetFileSizeByName(pInfo->szDesFilename);
    DWORD nSectSize=(pInfo->nEnd)-(pInfo->nStart);

	// 此段已下载完毕.
	if(nFileSize==nSectSize){
		//mj
		printf("文件下载成功！下载结束！\n");                //这里可以设置写信息
		//mj

		TRACE("文件已下载完毕!\n");                                     
		CHttpGet::m_nCount++;  // 计数.
		return 0;
	}

    FILE *fpwrite=myfile.GetFilePointer(pInfo->szDesFilename);
	if(!fpwrite) return 1;

    // 设置下载范围.
	SendHttpHeader(hSocket,pInfo->szHostAddr,pInfo->szHttpAddr,
		      pInfo->szHttpFilename,pInfo->nStart+nFileSize);
	
	// 设置文件写指针起始位置，断点续传
	fseek(fpwrite,nFileSize,SEEK_SET);

	DWORD nLen; 
	DWORD nSumLen=0; 
	char szBuffer[1024];

	while(1)
	{
		if(nSumLen>=nSectSize-nFileSize) break;
		nLen=recv(hSocket,szBuffer,sizeof(szBuffer),0);
		
		//原子操作，不用同步。
		rdownloaded += nLen;
		
		if (nLen == SOCKET_ERROR){
			TRACE("Read error!\n");
			fclose(fpwrite);
			return 1;
		}

  		if(nLen==0) break;
		nSumLen +=nLen;
		TRACE("%d\n",nLen);

		// 把数据写入文件.		
		fwrite(szBuffer,nLen,1,fpwrite);
	}

    
	fclose(fpwrite);      // 关闭写文件.
	closesocket(hSocket); // 关闭套接字.
	CHttpGet::m_nCount++; // 计数.
	return 0;
}

//---------------------------------------------------------------------------
SOCKET CHttpGet::ConnectHttpProxy(CString strProxyAddr,int nPort)
{
	TRACE("正在建立连接\n");
	
  	CString sTemp;
	char cTmpBuffer[1024];
	SOCKET hSocket=dealsocket.GetConnect(strProxyAddr,nPort);

    if(hSocket == INVALID_SOCKET)
    {
		TRACE("连接http服务器失败！\n");
		return INVALID_SOCKET;
    }

	// 发送CONNCET请求令到代理服务器，用于和代理建立连接代理服务器的
	// 地址和端口放在strProxyAddr,nPort 里面.
	sTemp.Format("CONNECT %s:%d HTTP/1.1\r\nUser-Agent:\
		           MyApp/0.1\r\n\r\n",strProxyAddr,nPort);
	
	if(!SocketSend(hSocket,sTemp))
	{
		TRACE("连接代理失败\n");
		return INVALID_SOCKET;
	}

	// 取得代理响应，如果连接代理成功，代理服务器将
	// 返回"200 Connection established".
	int nLen=GetHttpHeader(hSocket,cTmpBuffer);
	sTemp=cTmpBuffer;
	if(sTemp.Find("HTTP/1.0 200 Connection established",0)==-1)
	{
		TRACE("连接代理失败\n");
		return INVALID_SOCKET;
	}

	TRACE(sTemp);
	TRACE("代理连接完成\n");
	return hSocket; 
}

//---------------------------------------------------------------------------
SOCKET CHttpGet::ConnectHttpNonProxy(CString strHostAddr,int nPort)
{
	TRACE("正在建立连接\n");
    SOCKET hSocket=dealsocket.GetConnect(strHostAddr,nPort);
	if(hSocket == INVALID_SOCKET)
		return INVALID_SOCKET;
    
	return hSocket;
}

//---------------------------------------------------------------------------
// 例如: strHostAddr="www.aitenshi.com",
// strHttpAddr="http://www.aitenshi.com/bbs/images/",
// strHttpFilename="pics.jpg".
BOOL CHttpGet::SendHttpHeader(SOCKET hSocket,CString strHostAddr,
				CString strHttpAddr,CString strHttpFilename,DWORD nPos)
{
	// 进行下载. 
	static CString sTemp;
	char cTmpBuffer[1024];

	// Line1: 请求的路径,版本.
	sTemp.Format("GET %s%s HTTP/1.1\r\n",strHttpAddr,strHttpFilename);
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// Line2:主机.
	sTemp.Format("Host: %s\r\n",strHostAddr);
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// Line3:接收的数据类型.
	sTemp.Format("Accept: \r\n");
	if(!SocketSend(hSocket,sTemp)) return FALSE;
	
	// Line4:参考地址.
    sTemp.Format("Referer: %s\r\n",strHttpAddr); 
	if(!SocketSend(hSocket,sTemp)) return FALSE;
		
	// Line5:浏览器类型.
	sTemp.Format("User-Agent: Mozilla/4.0 \
		(compatible; MSIE 5.0; Windows NT; DigExt; DTS Agent;)\r\n");

	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// 续传. Range 是要下载的数据范围，对续传很重要.
	sTemp.Format("Range: bytes=%d-\r\n",nPos);
	if(!SocketSend(hSocket,sTemp)) return FALSE;
	
	// LastLine: 空行.
	sTemp.Format("\r\n");
	if(!SocketSend(hSocket,sTemp)) return FALSE;

	// 取得http头.
	int i=GetHttpHeader(hSocket,cTmpBuffer);
	if(!i)
	{
		TRACE("获取HTTP头出错!\n");
		return 0;
	}
	
	// 如果取得的http头含有404等字样，则表示连接出问题.
	sTemp=cTmpBuffer;
	if(sTemp.Find("404")!=-1) return FALSE;

	// 得到待下载文件的大小.
	m_nFileLength=GetFileLength(cTmpBuffer);

	// 因为TRACE()函数最大的字符串长度为255.
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
		TRACE("发送请求失败!\n");
		return FALSE;
	}
	
	return TRUE;
}


CDealSocket dealsocket;

//---------------------------------------------------------------------------
CDealSocket::CDealSocket()
{
	// 套接字初始化.
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	
	// 初始化WinSock.
	if (WSAStartup(wVersionRequested, &wsaData)!=0)
	{
		TRACE("WSAStartup\n");
		return;
	}

	// 检查 WinSock 版本.
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
	// 释放WinSock.
	WSACleanup();
}

//---------------------------------------------------------------------------
CString CDealSocket::GetResponse(SOCKET hSock)
{
	char szBufferA[MAX_RECV_LEN];  	// ASCII字符串. 
	int	iReturn;					// recv函数返回的值.
	
	CString szError;
	CString strPlus;
	strPlus.Empty();

	while(1)
	{
		// 从套接字接收资料.
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
	SOCKADDR_IN saServer;          // 服务器套接字地址.
	PHOSTENT phostent = NULL;	   // 指向HOSTENT结构指针.

	// 创建一个绑定到服务器的TCP/IP套接字.
	if ((hSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		TRACE("Allocating socket failed. Error: %d\n",WSAGetLastError ());
		return INVALID_SOCKET;
	}
	
	// 使用TCP/IP协议.
	saServer.sin_family = AF_INET;
	
	// 获取与主机相关的信息.
	if ((phostent = gethostbyname (host)) == NULL) 
	{
		TRACE("Unable to get the host name. Error: %d\n",WSAGetLastError ());
		closesocket (hSocket);
		return INVALID_SOCKET;
	}

	// 给套接字IP地址赋值.
	memcpy ((char *)&(saServer.sin_addr), 
		phostent->h_addr, 
		phostent->h_length);
	
	// 设定套接字端口号.
	saServer.sin_port =htons (port); 

	// 建立到服务器的套接字连接.
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
	SOCKET ListenSocket = INVALID_SOCKET;	// 监听套接字.
	SOCKADDR_IN local_sin;				    // 本地套接字地址.
	
	// 创建TCP/IP套接字.
	if ((ListenSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
	{
		TRACE("Allocating socket failed. Error: %d\n",WSAGetLastError ());
		return INVALID_SOCKET;
	}
	
	// 给套接字信息结构赋值.
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons (port); 
	local_sin.sin_addr.s_addr = htonl (INADDR_ANY);
	
	// 进行本机地址与监听套接字绑定.
	if (bind (ListenSocket, 
		(struct sockaddr *) &local_sin, 
		sizeof (local_sin)) == SOCKET_ERROR) 
	{
		TRACE("Binding socket failed. Error: %d\n",WSAGetLastError ());
		closesocket (ListenSocket);
		return INVALID_SOCKET;
	}
	
	// 建立套接字对外部连接的监听.
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
		// 打开已有文件进行写数据.
		fp=fopen(lpszFileName,"r+b");
	}
	else{
		// 创建新文件进行写数据.
		fp=fopen(lpszFileName,"w+b");
	}

	return fp;
}

//---------------------------------------------------------------------------
DWORD CMyFile::GetFileSizeByName(LPCTSTR lpszFileName)
{
	if(!FileExists(lpszFileName)) return 0;
	struct _stat ST; 
	// 获取文件长度.
	_stat(lpszFileName, &ST);
	UINT nFilesize=ST.st_size;
	return nFilesize; 
}

//---------------------------------------------------------------------------
// 从全程文件名中提取短文件名.
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