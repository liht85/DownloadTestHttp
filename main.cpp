// DownloadTest.cpp : 定义控制台应用程序的入口点。
//
#include <afx.h>	//要放第一
#include <afxwin.h>   
#include "Mydownload.h"

#include <iostream>
using namespace std;

#include "DownloadHelper.h"


UINT ThreadCount(void* pParam);
 
//分配空间，用于跟踪。
unsigned long temp = 0;
unsigned long *downloaded = &temp;
unsigned long totalSize = 1024;

//下载完成后调用这个函数
void testFunc(){
	cout<<"Finished!!!!"<<endl;
}

int _tmain(int argc, _TCHAR* argv[])
{	
	////////////////////////////////////////////////////////////////////////////
	//DownloadHelper用法演示：
	void (*func)();
	func = testFunc;
	DownloadHelper downloadHelper;
	//downloadHelper.setOnFinish(testFunc);
	//downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/IMG_0612_699f056f-004d-4d7b-b566-18a4c28ad0c0.JPG","f:\\");
//	downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/3_3a43066c-474f-4cc1-b225-1770a12e0f74.jpg","f:\\");
//	downloadHelper.addDownloadTask("http://www.dcpsc.com/mediaFiles/4_e7a48f3e-2cc0-4f9f-994a-ccba382369ce.jpg","f:\\");
	downloadHelper.addDownloadTask("http://www.dcpsc.com/manage/mediaFiles/图片2_34ebc1fc-76af-47bc-b895-61c4d45d9574.jpg","f:\\");
	downloadHelper.startDownload();
	downloadHelper.join();

	////////////////////////////////////////////////////////////////////////////
	//直接使用fnMydownload方法演示：

	//用于速度统计
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
	cout<<"已完成："<<(*downloaded*100)/(totalSize+1)<<"%  下载速度"<<(*downloaded-temp)/1024<<"KB/S"<<endl;
	temp = *downloaded;
	if(*downloaded >= totalSize-1)
	{
		cout<<"下载完成！"<<endl;
        return 0;
	}
	Sleep(2000);
	}
	return 0;
}
