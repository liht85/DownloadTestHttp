#ifndef _THREAD_SPECIFICAL_H__
#define _THREAD_SPECIFICAL_H__

#define    WIN32_LEAN_AND_MEAN   //��ֹwindows.h����winsock.h��winsock2.h��ͻ
#include   <windows.h>   

static unsigned int __stdcall threadFunction(void *);

class Thread {
   friend unsigned int __stdcall threadFunction(void *);
public:
   Thread();
   virtual ~Thread();
   int start(void * = NULL);//�߳����������������������������ָ�롣
   void stop();
   void* join();//�ȴ���ǰ�߳̽���
   void detach();//���ȴ���ǰ�߳�
   static void sleep(unsigned int);//�õ�ǰ�߳����߸���ʱ�䣬��λΪ����

protected:
   virtual void * run(void *) = 0;//����ʵ���߳�����̺߳�������

private:
   HANDLE threadHandle;
   bool started;
   bool detached;
   void * param;
   unsigned int threadID;
};

#endif