// DownloadTest.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include <afx.h>	//Ҫ�ŵ�һ
#include <afxwin.h>   
#include "Mydownload.h"

#include <iostream>
using namespace std;

#include "DownloadHelper.h"


UINT ThreadCount(void* pParam);
 
//����ռ䣬���ڸ��١�
unsigned long temp = 0;
unsigned long *downloaded = &temp;
unsigned long totalSize = 1024;

//������ɺ�����������
void testFunc(){
	cout<<"Finished!!!!"<<endl;
}

int _tmain(int argc, _TCHAR* argv[])
{	
	////////////////////////////////////////////////////////////////////////////
	//DownloadHelper�÷���ʾ��
	void (*func)();
	func = testFunc;
	DownloadHelper downloadHelper;
	//downloadHelper.setOnFinish(testFunc);
	//downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/IMG_0612_699f056f-004d-4d7b-b566-18a4c28ad0c0.JPG","f:\\");
//	downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/3_3a43066c-474f-4cc1-b225-1770a12e0f74.jpg","f:\\");
//	downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/4_e7a48f3e-2cc0-4f9f-994a-ccba382369ce.jpg","f:\\");
	downloadHelper.addDownloadTask("http://www.dcpsc.com/manage/mediaFiles/ͼƬ2_34ebc1fc-76af-47bc-b895-61c4d45d9574.jpg","f:\\");
	downloadHelper.startDownload();
	downloadHelper.join();

	////////////////////////////////////////////////////////////////////////////
	//ֱ��ʹ��fnMydownload������ʾ��

	//�����ٶ�ͳ��
	//AfxBeginThread(ThreadCount,0);
	//bool success = fnMyDownload("http://www.dcpsc.com/manage/mediaFiles/video_405e85a9-7f10-4294-a997-c4f81970a893.mpg",
	//		"f:\\",downloaded,totalSize,"",0,4);
	//fnMyDownload("http://www.dcpsc.com/mediaFiles/IMG_0612_699f056f-004d-4d7b-b566-18a4c28ad0c0.JPG",
	//		"f:\\",downloaded,totalSize,"",0,3);
	

	return 0;
}

UINT ThreadCount(void* pParam)
{
	unsigned long temp = 0;
	while(1){
	cout<<"����ɣ�"<<(*downloaded*100)/(totalSize+1)<<"%  �����ٶ�"<<(*downloaded-temp)/1024<<"KB/S"<<endl;
	temp = *downloaded;
	if(*downloaded >= totalSize-1)
	{
		cout<<"������ɣ�"<<endl;
        return 0;
	}
	Sleep(2000);
	}
	return 0;
}
