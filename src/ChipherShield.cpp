/*
-------------------------------------------------------------------------------
 This source file is part of ChipherShield (a file encryption software).

 Copyright (C) 2002-2003, Arijit De <arijit1985@yahoo.co.in>

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 http://www.gnu.org/copyleft/gpl.html.
-------------------------------------------------------------------------------
*/

// ChipherShield.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ChipherShield.h"
#include "ChipherShieldDlg.h"
#include "cryptlib.h"
#include <io.h>
#include <list>
#include <string>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT NCM_AddFiles;
LPVOID gShared;

// CChipherShieldApp

BEGIN_MESSAGE_MAP(CChipherShieldApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CChipherShieldApp construction
extern "C" BOOL WINAPI DllMain_Cryptlib( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );

CChipherShieldApp::CChipherShieldApp()
{
	// Initialize COM
	CoInitialize(0);

	// added by Jackson 20240331
	DllMain_Cryptlib(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, 0);

	cryptInit();
}


// The one and only CChipherShieldApp object

CChipherShieldApp theApp;


// CChipherShieldApp initialization

BOOL CChipherShieldApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// create shared memory for transfering filenames
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE,0,10*1024,_T("chiphershield_shared"));
	if (!hMapFile)
	{
		MessageBox(0,_T("Could not allocate shared memory! Please increase your virtual memory.\n\
ChipherShield will now exit."),_T("Error"),MB_ICONERROR);
		return 1;
	}

	gShared = MapViewOfFile(hMapFile,FILE_MAP_ALL_ACCESS,0,0,0);
	if (!gShared)
	{
		MessageBox(0,_T("Could not map shared memory! Please increase your virtual memory.\n\
ChipherShield will now exit."),_T("Error"),MB_ICONERROR);
		return 1;
	}

	HKEY hKey;
	RegCreateKey(HKEY_LOCAL_MACHINE,_T("Software\\ChipherShield\\1.1"),&hKey);
	RegCloseKey(hKey);

	NCM_AddFiles = RegisterWindowMessage(_T("NCM_AddFiles"));

	CChipherShieldDlg dlg(m_lpCmdLine);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	UnmapViewOfFile(gShared);
	CloseHandle(hMapFile);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

CChipherShieldApp::~CChipherShieldApp()
{
	cryptEnd();
}