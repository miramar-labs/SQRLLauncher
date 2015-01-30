// SqrlLauncher.cpp : Implementation of WinMain
#include "stdafx.h"
#include "resource.h"
#include "SqrlLauncher_i.h"
#include "xdlldata.h"
#include "SqrlPoster.h"

class CSqrlLauncherModule : public ATL::CAtlExeModuleT< CSqrlLauncherModule >
{
public :
	DECLARE_LIBID(LIBID_SqrlLauncherLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SQRLLAUNCHER, "{C5EDD644-8351-4F2F-A0EE-BB035E6944E0}")
	};

CSqrlLauncherModule _AtlModule;

extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
	LPTSTR lpCmdLine, int nShowCmd)
{
	//std::wstring sFilePath(U("C:\\Users\\Aaron Cody\\Desktop\\sqrl\\test.pdf"));

	//Set cmdline args in debug settings to: C:\Users\Aaron Cody\Desktop\sqrl\test.pdf
	std::wstring sFilePath(lpCmdLine);

	SqrlPoster poster(sFilePath);

	bool err = FALSE;

	utility::string_t url = poster.doPOST(&err);

	if (!err){
		/* bring up browser here.....
		IWebBrowser2*	 m_spWebBrowser = NULL;//TODO
		BSTR url = SysAllocString(url.c_str());
		m_spWebBrowser->Navigate(url, NULL, NULL, NULL, NULL);
		SysFreeString(url);*/
	}

	//delete generated PDF ....
	//DeleteFileW(sFilePath.c_str());		//will fail if in use....

	return _AtlModule.WinMain(nShowCmd);
}




