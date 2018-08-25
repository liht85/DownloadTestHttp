// DownloadHelper.cpp: implementation of the DownloadHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DownloadHelper.h"
#include "Mydownload.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DownloadHelper::DownloadHelper()
{
	onFinish = NULL;
}

DownloadHelper::~DownloadHelper()
{

}

//添加下载任务，以传入的url作为唯一标识符
bool DownloadHelper::addDownloadTask(const char* remoteUrl, const char* localFolder)
{
	string remoteUrlString(remoteUrl);
	string localFolderString(localFolder);

	if(!existInVector(downloadListRemoteURLs,remoteUrlString)){
		downloadListRemoteURLs.push_back(remoteUrlString);
		downloadListLocalFolders.push_back(localFolderString);
		return true;
	}else{
		return false;
	}
}
  
	//每次删除第一个
/*	vector<string>::iterator startIterator = downloadListRemoteURLs.begin();  
	downloadListRemoteURLs.erase( startIterator );
*/

//判断字符串是否已经在vector<string>中出现
bool existInVector(vector<string>& array, string& str){
	for(int k = 0;k<array.size();k++){
		if(array[k].compare(str)==0)
			return true;
	}
	return false;
}

//开始下载
bool DownloadHelper::startDownload()
{
	this->start();
	return true;
}

//多线程重构函数
void * DownloadHelper::run(void *)
{	
	//分配空间，用于跟踪。
	unsigned long temp = 0;
	unsigned long *downloaded = &temp;
	unsigned long totalSize = 1024;
	while(downloadListRemoteURLs.size()>0)
	{
		cout<<downloadListRemoteURLs[0]<<endl;
		//默认三线程下载，可以修改，但必须保持不变，因为断点续传需要前后两次线程数一致
		while(true){
			//阻塞式，直到下载成功或者网络出错才跳出
			fnMyDownload(downloadListRemoteURLs[0].data(),
				downloadListLocalFolders[0].data(),downloaded,totalSize,"",0,THREAD_COUNT);	
			if(!exist(0)){
				//文件不存在，表示下载中断
				cout<<"网络中断，等待重连..."<<endl;
				Sleep(RECONNECT_INTERVAL);	//10秒后重连
			}else{
				//下载成功，删除第一个任务
				vector<string>::iterator startIterator = downloadListRemoteURLs.begin();  
				downloadListRemoteURLs.erase( startIterator );
				startIterator = downloadListLocalFolders.begin();  
				downloadListLocalFolders.erase( startIterator );
				break;	
			}
		}
    }
	if(onFinish!=NULL){
		onFinish();
	}
	return NULL;
}

//判断下载列表的文件是否已经存在
//传入index是downloadListRemoteURLs的下标
bool DownloadHelper::exist(int index)
{
	string fileName = downloadListRemoteURLs[index].substr(downloadListRemoteURLs[index].find_last_of("/")+1);
	string file(downloadListLocalFolders[index].data());	//copy
	file.append(fileName);
	return (_access(file.data(), 0) == 0);;
}


	//传入函数指针，下载完成后调用
void DownloadHelper::setOnFinish(void (*func)()){
	onFinish = func;
}