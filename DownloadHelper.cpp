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

//������������Դ����url��ΪΨһ��ʶ��
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
  
	//ÿ��ɾ����һ��
/*	vector<string>::iterator startIterator = downloadListRemoteURLs.begin();  
	downloadListRemoteURLs.erase( startIterator );
*/

//�ж��ַ����Ƿ��Ѿ���vector<string>�г���
bool existInVector(vector<string>& array, string& str){
	for(int k = 0;k<array.size();k++){
		if(array[k].compare(str)==0)
			return true;
	}
	return false;
}

//��ʼ����
bool DownloadHelper::startDownload()
{
	this->start();
	return true;
}

//���߳��ع�����
void * DownloadHelper::run(void *)
{	
	//����ռ䣬���ڸ��١�
	unsigned long temp = 0;
	unsigned long *downloaded = &temp;
	unsigned long totalSize = 1024;
	while(downloadListRemoteURLs.size()>0)
	{
		cout<<downloadListRemoteURLs[0]<<endl;
		//Ĭ�����߳����أ������޸ģ������뱣�ֲ��䣬��Ϊ�ϵ�������Ҫǰ�������߳���һ��
		while(true){
			//����ʽ��ֱ�����سɹ�����������������
			fnMyDownload(downloadListRemoteURLs[0].data(),
				downloadListLocalFolders[0].data(),downloaded,totalSize,"",0,THREAD_COUNT);	
			if(!exist(0)){
				//�ļ������ڣ���ʾ�����ж�
				cout<<"�����жϣ��ȴ�����..."<<endl;
				Sleep(RECONNECT_INTERVAL);	//10�������
			}else{
				//���سɹ���ɾ����һ������
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

//�ж������б���ļ��Ƿ��Ѿ�����
//����index��downloadListRemoteURLs���±�
bool DownloadHelper::exist(int index)
{
	string fileName = downloadListRemoteURLs[index].substr(downloadListRemoteURLs[index].find_last_of("/")+1);
	string file(downloadListLocalFolders[index].data());	//copy
	file.append(fileName);
	return (_access(file.data(), 0) == 0);;
}


	//���뺯��ָ�룬������ɺ����
void DownloadHelper::setOnFinish(void (*func)()){
	onFinish = func;
}