// SqrlLauncher.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "SqrlLauncher_i.h"
#include "xdlldata.h"


using namespace ATL;


class CSqrlLauncherModule : public ATL::CAtlExeModuleT< CSqrlLauncherModule >
{
public :
	DECLARE_LIBID(LIBID_SqrlLauncherLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SQRLLAUNCHER, "{C5EDD644-8351-4F2F-A0EE-BB035E6944E0}")
	};

CSqrlLauncherModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	return _AtlModule.WinMain(nShowCmd);
}

