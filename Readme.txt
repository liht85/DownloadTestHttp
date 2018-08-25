作者：Kenko 欢迎交流：kenkofox@qq.com
博客：http://www.cnblogs.com/kenkofox/

###原创声明：
Mydownload是一个匿名作者写的。非常完美的代码。
其他部分是我（Kenko）写的。另外，Mydownload里边有点小问题(无法断点续传)，经过修改，已经修复。

###使用方法：
在自己的工程中引入所有文件，在stdafx.h中对照本下载工具项目的stdafx.h，应该是要加入afxsock.h。
参考main.cpp。主要是调用DownloadHelper。先添加任务，然后设置onFinish（下载完成后的动作），最后start。
如果目录中已有该文件，程序会自动覆盖，不会出错。
默认下载端口是80，不支持其他端口。如果要这个功能，请修改Mydownload中的fnMyDownload,HttpDownLoadNonProxy加入nHostPort。

###fnMyDownload说明：
          bool fnMyDownload(  CString strUrl,
				CString strWriteFileName,
				unsigned long *& downloaded,
			        unsigned long & totalSize,
                                CString strProxy,
			        int nProxyPort,
				int nThread
				)
strUrl:               需要下载的文件地址，  如：“http://10.20.1.6/musiconline/091029/daisy.mp3”
strWriteFileName：    下载后文件的存放地址，如：“d:\\”
downloaded：          指针引用，调用函数后会不断变化，实时反映已下载多少字节的数据
totalSize：           传递引用，表示所要下载文件的总长度，与downloaded配合可以分析各种下载信息
strProxy，nProxyPort  这两个为代理选项，有的话填上，如"192.168.1.89",8080  没有的话麻烦点填上"",0 。（呵呵，暂时没有去研究具体怎么使用）
nThread：             用于下载开启的线程数，一般3，4个最佳。。太多反而降低速度，而且有些服务器不支持多连接。
例:fnMyDownload("http://10.20.1.6/musiconline/091029/daisy.mp3","d:\\",downloaded,totalSize,"",0,3)

###代码说明
Mydownload.cpp底端的fnMyDownload函数是下载器的关键函数。
其中在原作者的基础上，加了几句编码转换的代码。把网络的文件名和路径转化为UTF8，但这里可能会有问题，例如中文域名。有需要，请再作修改。
DownloadHelper是外围封装的类，用于列表管理，自动断点重连。
ChineseCode用于编码转换。
Thread是一个封装好的线程类，被DownloadHelper继承。类似于Java的Thread类。

###Mydownload代码笔记：
从fnMyDownload开始，程序首先解析输入的url，拆分为地址，路径，文件名等。然后获取文件头，得到文件大小，然后再下载。重点函数是ThreadDownLoad。下载完之后用FileCombine合并文件。

###开发测试环境：
windows xp + Visual C++ 6.0
!!!!!!需要使用MFC环境!!!!!!!!!!!!