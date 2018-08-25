// ChineseCode.h: interface for the CChineseCode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHINESECODE_H__E9206178_CA96_4B8A_AB2A_AD94C9CD12B1__INCLUDED_)
#define AFX_CHINESECODE_H__E9206178_CA96_4B8A_AB2A_AD94C9CD12B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
using namespace std;

class ChineseCode
{
   public:
       static void UTF_8ToUnicode(wchar_t* pOut,char *pText);  // 把UTF-8转换成Unicode
       static void UnicodeToUTF_8(char* pOut,wchar_t* pText);  //Unicode 转换成UTF-8
       static void UnicodeToGB2312(char* pOut,wchar_t uData);  // 把Unicode 转换成 GB2312 
       static void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer);// GB2312 转换成　Unicode
       static void GB2312ToUTF_8(string& pOut,char *pText, int pLen);//GB2312 转为 UTF-8
       static void UTF_8ToGB2312(string &pOut, char *pText, int pLen);//UTF-8 转为 GB2312
};

#endif // !defined(AFX_CHINESECODE_H__E9206178_CA96_4B8A_AB2A_AD94C9CD12B1__INCLUDED_)
