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

// ChipherShieldDlg.cpp : implementation file
//

#include <list>
#include <string>
#include "stdafx.h"
#include "ChipherShield.h"
#include "ChipherShieldDlg.h"
#include "shlobj.h"
#include "PwdDialog.h"
#include "Algorithm.h"
#include "Encryptor.h"
#include "Decryptor.h"
#include "ProgressListener.h"

#ifdef _DEBUG
#pragma comment(lib, "libeay32MTd.lib")
#pragma comment(lib, "ssleay32MTd.lib")
#pragma comment(lib, "cl32d.lib")
#else
#pragma comment(lib, "libeay32MT.lib")
#pragma comment(lib, "ssleay32MT.lib")
#pragma comment(lib, "cl32.lib")
#endif

//#pragma comment(linker, "/NODEFAULTLIB:libeay32MT.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CProgUpdater class

class CProgUpdater : public ProgressListener
{
public:
	CProgUpdater(CWnd* progressWin,CWnd* textWin);
	void increment(int step);
	void setRange(int max);
	void complete();
	void reset();
private:
	int mMax;
	float mTotal;
	CWnd* pWin;
	CWnd* pText;
};

CProgUpdater::CProgUpdater(CWnd* progressWin,CWnd* textWin) : pWin(progressWin),pText(textWin),mMax(100),mTotal(0)
{
	pWin->SendMessage(PBM_SETRANGE32,0,(LPARAM)100);
	pText->SendMessage(WM_SETTEXT,0,(LPARAM)"0 %");
}

void CProgUpdater::setRange(int max)
{
	mMax = max;
	reset();	
}

void CProgUpdater::reset()
{
	mTotal = 0;
	pWin->SendMessage(PBM_SETPOS,(WPARAM)0,0);
	pText->SendMessage(WM_SETTEXT,0,(LPARAM)"0 %");
}

void CProgUpdater::increment(int step)
{
	float percent = (float)step*100/mMax;
	mTotal += percent;

	pWin->SendMessage(PBM_SETPOS,(WPARAM)(int)mTotal,0);

    CString str;
	str.Format(_T("%d %%"),(int)mTotal);
	pText->SendMessage(WM_SETTEXT,0,(LPARAM)(LPCTSTR)str);
}

void CProgUpdater::complete()
{
	mTotal = 100;

	pWin->SendMessage(PBM_SETPOS,(WPARAM)100,0);
	pText->SendMessage(WM_SETTEXT,0,(LPARAM)"100 %");
}

///////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// Array of Modes
struct Mode
{
	char* name;
	Algorithm::Type alg;
	char* desc;
};

Mode gModes[10] = {
{"AES",Algorithm::AES,"AES is a 128-bit block cipher with a 128-bit key."},
{"BlowFish",Algorithm::BLOWFISH,"Blowfish is a 64-bit block cipher with a 448-bit key."},
{"CAST-128",Algorithm::CAST_128,"CAST-128 is a 64-bit block cipher with a 128-bit key."},
{"DES",Algorithm::DES,"DES is a 64-bit block cipher with a 56-bit key."},
{"IDEA",Algorithm::IDEA,"IDEA is a 64-bit block cipher with a 128-bit key."},
{"RC2",Algorithm::RC2,"RC2 is a 64-bit block cipher with a 1024-bit key."},
{"RC4",Algorithm::RC4,"RC4 is an 8-bit stream cipher with a key of up to 1024 bits."},
{"RC5",Algorithm::RC5,"RC5 is a 64-bit block cipher with an 832-bit key."},
{"SkipJack",Algorithm::SKIPJACK,"Skipjack is a 64-bit block cipher with an 80-bit key."},
{"Triple-DES (Default)",Algorithm::TRIPLE_DES,"Triple DES is a 64-bit block cipher with a 112/168-bit key."}
};

///////////////////////////////////////////////////////////////////////////////
// CChipherShieldDlg dialog

CChipherShieldDlg::CChipherShieldDlg(LPCTSTR buf)
	: CDialog(CChipherShieldDlg::IDD, 0), p_enc(0), p_dec(0), p_list(0), p_progTotal(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_ImageList.Create(16,16,ILC_COLOR8,2,2);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON1));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON2));

	ParseNames(buf,m_InitFiles);
}

CChipherShieldDlg::~CChipherShieldDlg()
{
	delete p_list;
	delete p_progTotal;
	delete p_enc;
	delete p_dec;
}

void CChipherShieldDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChipherShieldDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(NCM_AddFiles, OnExternalAddFiles)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_HELP_ABOUTChipherShield, OnHelpAbout)
	ON_COMMAND(ID_OPTIONS_HIDEPWD, OnOptionsHidePwd)
	ON_BN_CLICKED(IDC_BUT_ADDDIR, OnBnClickedButAdddir)
	ON_BN_CLICKED(IDC_BUT_ADDFILE, OnBnClickedButAddfile)
	ON_BN_CLICKED(IDC_BUT_CLEAR, OnBnClickedButClear)
	ON_BN_CLICKED(IDC_BUT_REMOVE, OnBnClickedButRemove)
	ON_BN_CLICKED(IDC_PWD_ADD, OnBnClickedPwdAdd)
	ON_BN_CLICKED(IDC_PWD_SUB, OnBnClickedPwdSub)
	ON_CBN_SELCHANGE(IDC_COMBO_MODE, OnCbnSelchangeComboMode)
	ON_BN_CLICKED(IDC_BUT_GO, OnBnClickedButGo)
	ON_BN_CLICKED(IDC_RADIO_DECRYPT, OnBnClickedRadioDecrypt)
	ON_BN_CLICKED(IDC_RADIO_ENCRYPT, OnBnClickedRadioEncrypt)
END_MESSAGE_MAP()


// CChipherShieldDlg message handlers

BOOL CChipherShieldDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// set image list for ListView
	HWND hlv = GetDlgItem(IDC_LV_FILE)->m_hWnd;
	ListView_SetImageList(hlv,m_ImageList.GetSafeHandle(),LVSIL_SMALL);

	// add ListView columns
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = 250;
	lvc.pszText = _T("Name");
	lvc.iSubItem = 0;
	ListView_InsertColumn(hlv,0,&lvc);
	lvc.cx = 100;
	lvc.pszText = _T("Type");
	lvc.iSubItem = 1;
	ListView_InsertColumn(hlv,1,&lvc);

	// add init files
	if (m_InitFiles.size())
	{
		std::list<std::string>::iterator it = m_InitFiles.begin();
		std::list<std::string>::iterator end = m_InitFiles.end();

		for (; it != end; ++it)
		{
			DWORD attrib = GetFileAttributesA((*it).c_str());
			if (attrib != -1)
			{
				if (attrib & FILE_ATTRIBUTE_DIRECTORY)
					AddToFileList((*it).c_str(),true);
				else
					AddToFileList((*it).c_str(),false);
			}	
		}
	}

	// check "Encrypt" radio button
	CheckDlgButton(IDC_RADIO_ENCRYPT,BST_CHECKED);

	// add modes to dropdown-list
	CWnd* pCombo = GetDlgItem(IDC_COMBO_MODE);
	for (int i = 0; i < 10; ++i)
	{
		pCombo->SendMessage(CB_ADDSTRING,0,(LPARAM)gModes[i].name);

		if (gModes[i].alg == Algorithm::TRIPLE_DES)
		{
			pCombo->SendMessage(CB_SETCURSEL,i,0);
			SendDlgItemMessage(IDC_DESC,WM_SETTEXT,0,(LPARAM)gModes[i].desc);
		}
	}

	// create progress updater
	p_list = new CProgUpdater(GetDlgItem(IDC_PROG_FILE),GetDlgItem(IDC_PERCENT_FILE));
	p_progTotal = new CProgUpdater(GetDlgItem(IDC_PROG_TOTAL),GetDlgItem(IDC_PERCENT_TOTAL));

	// create encryptor,decryptor
	p_enc = new Encryptor(p_list);
	p_dec = new Decryptor(p_list);


	BOOL hide;
	DWORD valType;
	DWORD valSize = 4;

	// get registry setting
	HKEY hKey;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\ChipherShield\\1.1",0,KEY_QUERY_VALUE,&hKey);
	if (RegQueryValueExA(hKey,"HidePasswords",0,&valType,(BYTE*)&hide,&valSize) != ERROR_SUCCESS)
		hide = 1;
	RegCloseKey(hKey);

	// hiding passwords
	if (hide)
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_HIDEPWD,MF_CHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(TRUE);
	}
	else
	{
		GetMenu()->CheckMenuItem(ID_OPTIONS_HIDEPWD,MF_UNCHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(FALSE);
	}

	SetStaticPwdMsg(0);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChipherShieldDlg::OnDestroy()
{
	DWORD data = 0;

	if (GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD,MF_BYCOMMAND) & MF_CHECKED)
		data = 1;

	// set registry setting
	HKEY hKey;
	RegCreateKeyExA(HKEY_LOCAL_MACHINE,"Software\\ChipherShield\\1.1",0,0,0,KEY_SET_VALUE,0,&hKey,0);
	RegSetValueExA(hKey,"HidePasswords",0,REG_DWORD,(BYTE*)&data,4);
	RegCloseKey(hKey);
}

void CChipherShieldDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CChipherShieldDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CChipherShieldDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChipherShieldDlg::OnFileExit()
{
	DestroyWindow();
}

void CChipherShieldDlg::OnHelpAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CChipherShieldDlg::OnOptionsHidePwd()
{
	CMenu* menu = GetMenu();

	if (menu->GetMenuState(ID_OPTIONS_HIDEPWD,MF_BYCOMMAND) & MF_CHECKED)
	{
		menu->CheckMenuItem(ID_OPTIONS_HIDEPWD,MF_UNCHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(FALSE);
	}
	else
	{
		menu->CheckMenuItem(ID_OPTIONS_HIDEPWD,MF_CHECKED);
		GetDlgItem(IDC_LIST_PWD)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_PWDMSG)->ShowWindow(TRUE);
	}
}

void CChipherShieldDlg::ParseNames(const char* buf,std::list<std::string>& files)
{
	CString arg(buf);
	if (!arg.IsEmpty())
	{
		arg.TrimLeft();
		arg.TrimRight();
		arg.Insert(arg.GetLength(),' ');
		int i;

		while ( (i = arg.FindOneOf(_T(" \""))) != -1 )
		{
			CString file;

			if (arg[i] == _T('\"'))
			{
				if (i != 0)
				{
					file = arg.Left(i);
					file.TrimLeft();
					file.TrimRight();
					arg.Delete(0,i+1);
				}
				else
				{
					arg.Delete(0,1);

					int j = arg.FindOneOf(_T("\""));
					if (j == -1)
						break;

					file = arg.Left(j);
					file.TrimLeft();
					file.TrimRight();
					arg.Delete(0,j+1);
				}
			}
			else
			{
				file = arg.Left(i);
				arg.Delete(0,i+1);
			}
			arg.TrimLeft();

			if (file.IsEmpty())
				continue;
			else if (GetFileAttributes(file) == -1)
			{
				std::stringstream s;
				s << "Cannot access file \"" << file.operator LPCTSTR() << "\". This file will be ignored.";
				
				MessageBox(s.str().c_str(),"ChipherShield Warning",MB_ICONWARNING);
			}
			else
			{
				files.push_front(std::string(file.operator LPCTSTR()));
			}
		}
	}
}

LRESULT CChipherShieldDlg::OnExternalAddFiles(WPARAM wParam,LPARAM lParam)
{
	if (wParam != 0 && lParam != 0)
		return FALSE;	

	if (strcmp((char*)gShared,"OK") != 0 && GetDlgItem(IDC_BUT_GO)->IsWindowEnabled())
	{
		std::list<std::string> files;
		ParseNames((const char*)gShared,files);

		if (files.size() == 0)
			return TRUE;

		std::list<std::string>::iterator it = files.begin();
		std::list<std::string>::iterator end = files.end();

		for (; it != end; ++it)
		{
			DWORD attrib = GetFileAttributesA((*it).c_str());
			if (attrib != -1)
			{
				if (attrib & FILE_ATTRIBUTE_DIRECTORY)
					AddToFileList((*it).c_str(),true);
				else
					AddToFileList((*it).c_str(),false);
			}
		}
		
		strcpy((char*)gShared,"OK");
		memcpy(((char*)gShared)+3,(char*)&m_hWnd,4);
	}
	return FALSE;
}

void CChipherShieldDlg::OnBnClickedButAdddir()
{
	IMalloc* pim;
	SHGetMalloc(&pim);

	ITEMIDLIST* pidl;
	SHGetSpecialFolderLocation(m_hWnd,CSIDL_DRIVES,&pidl);

	char buf[MAX_PATH];
	BROWSEINFOA bi;
	bi.pidlRoot = pidl;
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = buf;
	bi.lpszTitle = "Select Folder";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = 0;
	bi.iImage = 0;

	ITEMIDLIST* folder = SHBrowseForFolderA(&bi);
	if (!folder)
		return;

	SHGetPathFromIDListA(folder,buf);

	pim->Free(pidl);
	AddToFileList(buf,true);
}

void CChipherShieldDlg::AddToFileList(const char* name, bool dir)
{
	HWND hlv;
	GetDlgItem(IDC_LV_FILE,&hlv);

	// check if the file/dir is already in the list
	LVFINDINFOA lvfi;
	lvfi.flags = LVFI_STRING;
	lvfi.psz = name;
	if (::SendMessage(hlv,LVM_FINDITEM,(WPARAM)(int)-1,(LPARAM)&lvfi) != -1)
		return;

	// add name to list
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = const_cast<LPSTR>(name);
	lvi.iImage = (dir ? 0 : 1);
	ListView_InsertItem(hlv,&lvi);

	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	lvi.iSubItem = 1;
	if (dir)
		lvi.pszText = "Directory";
	else
		lvi.pszText = "File";
	ListView_SetItem(hlv,&lvi);
}

void CChipherShieldDlg::OnBnClickedButAddfile()
{
	char* filter = "All files (*.*)|*.*||";
	CFileDialog cfd(TRUE,0,0,OFN_FILEMUSTEXIST | OFN_ENABLESIZING | OFN_EXPLORER |
		/*OFN_FORCESHOWHIDDEN*/ 0x10000000 | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_ALLOWMULTISELECT,
		filter,this);

	char* buf = new char[10*1024];
	buf[0] = 0;

	cfd.m_ofn.lpstrFile = buf;
	cfd.m_ofn.nMaxFile = 10*1024;

	if (cfd.DoModal() == IDOK)
	{
		POSITION pos = cfd.GetStartPosition();
		CString path;

		do
		{
			path = cfd.GetNextPathName(pos);
			AddToFileList(path,false);
		} while (pos);
	}

	delete [] buf;
}

void CChipherShieldDlg::OnBnClickedButClear()
{
	ListView_DeleteAllItems(GetDlgItem(IDC_LV_FILE)->m_hWnd);
}

void CChipherShieldDlg::OnBnClickedButRemove()
{
	HWND hlv;
	GetDlgItem(IDC_LV_FILE,&hlv);

	int selCount = ListView_GetSelectedCount(hlv);

	if (selCount == 0)
		return;

	while (selCount > 0)
	{
		int sel = ListView_GetNextItem(hlv,-1,LVNI_SELECTED);
		ListView_DeleteItem(hlv,sel);
		--selCount;
	}
}

void CChipherShieldDlg::OnBnClickedPwdAdd()
{
	bool hide;
	if (GetMenu()->GetMenuState(ID_OPTIONS_HIDEPWD,MF_BYCOMMAND) & MF_CHECKED)
		hide = true;
	else
		hide = false;

	CPwdDialog pd(hide,this);
	CWnd* pwdList = GetDlgItem(IDC_LIST_PWD);

	if (pd.DoModal() == IDOK)
	{
		int result = (int)pwdList->SendMessage(LB_INSERTSTRING,0,(LPARAM)pd.GetPassword());
		if (result < 0)
			MessageBox("Error occured while adding password!","Runtime Error",MB_ICONERROR);
		else
		{
			result = (int)pwdList->SendMessage(LB_GETCOUNT,0,0);

			SetStaticPwdMsg(pwdList->SendMessage(LB_GETCOUNT));
			pwdList->SendMessage(LB_SETSEL,FALSE,-1);
			pwdList->SendMessage(LB_SETSEL,TRUE,0);
		}
	}
}

void CChipherShieldDlg::OnBnClickedPwdSub()
{
	CWnd* pList = GetDlgItem(IDC_LIST_PWD);

	int count = (int)pList->SendMessage(LB_GETCOUNT,0,0);
    int* items = new int[count];
	
	count = (int)pList->SendMessage(LB_GETSELITEMS,count,(LPARAM)items);

	if (count)
	{
		--count;
		while (count >= 0)
		{
			pList->SendMessage(LB_DELETESTRING,items[count],0);
			--count;
		}
	}
	else
		pList->SendMessage(LB_RESETCONTENT,0,0);

	delete [] items;

	CWnd* pwdList = GetDlgItem(IDC_LIST_PWD);

	SetStaticPwdMsg(pwdList->SendMessage(LB_GETCOUNT));
	pwdList->SendMessage(LB_SETSEL,FALSE,-1);
	pwdList->SendMessage(LB_SETSEL,TRUE,0);
}

void CChipherShieldDlg::SetStaticPwdMsg(int num)
{
	char buf[128];
	sprintf(buf,"%d password(s) are hidden. (Options -> Hide Passwords)",num);
	GetDlgItem(IDC_STATIC_PWDMSG)->SendMessage(WM_SETTEXT,0,(LPARAM)buf);
}

void CChipherShieldDlg::OnCbnSelchangeComboMode()
{
	int sel = (int)SendDlgItemMessage(IDC_COMBO_MODE,CB_GETCURSEL,0,0);

	if (sel == CB_ERR)
		return;

	SendDlgItemMessage(IDC_DESC,WM_SETTEXT,0,(LPARAM)gModes[sel].desc);
}

void CChipherShieldDlg::OnBnClickedButGo()
{
	GetDlgItem(IDC_BUT_GO)->EnableWindow(FALSE);

	DWORD threadId;
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)&CChipherShieldDlg::Go,this,0,&threadId);
}

DWORD WINAPI CChipherShieldDlg::Go(CChipherShieldDlg* me)
{
	bool encrypt;

	if (me->IsDlgButtonChecked(IDC_RADIO_ENCRYPT) == BST_CHECKED)
	{
		encrypt = true;
	}
	else if (me->IsDlgButtonChecked(IDC_RADIO_DECRYPT) == BST_CHECKED)
	{
		encrypt = false;
	}
	else
	{
		me->MessageBox("None of the Action radio buttons are checked!","Logic Error",MB_ICONERROR);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	// get the password list
	std::list<std::string> pwd;
	CWnd* pList = me->GetDlgItem(IDC_LIST_PWD);
	int count = pList->SendMessage(LB_GETCOUNT,0,0);
	if (!count)
	{
		me->MessageBox("No password(s) given! Please add one or more passwords first.","ChipherShield",MB_ICONWARNING);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	int i;
	for (i = 0; i < count; ++i)
	{
		char pass[512];
		pList->SendMessage(LB_GETTEXT,(WPARAM)i,(LPARAM)pass);

		pwd.push_front(pass);
	}

	// get algorithm type
	Algorithm::Type algType = gModes[(int)me->SendDlgItemMessage(IDC_COMBO_MODE,CB_GETCURSEL,0,0)].alg;

	// enumerate file list items
	CWnd* pLv = me->GetDlgItem(IDC_LV_FILE);
	count = pLv->SendMessage(LVM_GETITEMCOUNT,0,0);
	
	if (count == 0)
	{
		me->MessageBox("No files selected! Please add the files/directories you want to process.","ChipherShield",MB_ICONWARNING);
		
		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;

	char buf[300];
	std::list<std::string> mFiles;

	// generate file list
	for (lvi.iItem = 0; lvi.iItem < count; ++lvi.iItem)
	{
		// name
		lvi.iSubItem = 0;
		lvi.pszText = buf;
		lvi.cchTextMax = 256;
		if (!pLv->SendMessage(LVM_GETITEM,0,(LPARAM)&lvi))
			continue;

		if (lvi.iImage == 0)
		{
			// item is a directory
			me->ProcessDir(buf,mFiles);
		}
		else
		{
			// item is a file
			mFiles.push_front(std::string(buf));
		}
	}

	if (mFiles.size() == 0)
	{
		me->MessageBox("The folder(s) selected do not contain any files!","ChipherShield",MB_ICONWARNING);

		me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
		return 1;
	}

	me->p_progTotal->setRange(mFiles.size());

	// process each file in list
	std::list<std::string>::iterator it;
	std::list<std::string>::iterator end = mFiles.end();
	bool status = true;

	for (it = mFiles.begin(); it != end; ++it)
	{
		me->GetDlgItem(IDC_STATIC_FILE)->SendMessage(WM_SETTEXT,0,(LPARAM)it->c_str());

		if (!me->ProcessFile(it->c_str(),encrypt,pwd,algType))
			status = false;

		me->p_progTotal->increment(1);
	}

	pLv->SendMessage(LVM_DELETEALLITEMS,0,0);
	me->p_progTotal->complete();

	sprintf(buf,"%d file(s) processed successfully!",mFiles.size());

	if (status)
		me->MessageBox(buf,"ChipherShield",MB_OK | MB_ICONINFORMATION);
	else
		me->MessageBox("One or more errors occured while processing files!","ChipherShield",MB_OK | MB_ICONWARNING);

	me->p_list->reset();
	me->p_progTotal->reset();
	me->GetDlgItem(IDC_BUT_GO)->EnableWindow(TRUE);
	me->GetDlgItem(IDC_STATIC_FILE)->SendMessage(WM_SETTEXT,0,(LPARAM)"Ready.");

	return 0;
}

void CChipherShieldDlg::ProcessDir(const char* name,std::list<std::string>& fileList)
{
	// We will recurse through the directory and its sub-directories
	// and add all files to our list
	WIN32_FIND_DATA wfd;
	char mask[MAX_PATH];

	sprintf(mask,"%s\\*",name);

	HANDLE hFind = FindFirstFile(mask,&wfd);
	bool found;

	if (hFind == INVALID_HANDLE_VALUE)
		found = false;
	else
		found = true;

	while (found)
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(wfd.cFileName,".") && strcmp(wfd.cFileName,".."))
			{
				char dirpath[MAX_PATH];
				sprintf(dirpath,"%s\\%s",name,wfd.cFileName);

				ProcessDir(dirpath,fileList);
			}
		}
		else
		{
			char filepath[MAX_PATH];
			sprintf(filepath,"%s\\%s",name,wfd.cFileName);

			fileList.push_front(std::string(filepath));
		}

		if (FindNextFile(hFind,&wfd))
			found = true;
		else
			found = false;
	}
}

bool CChipherShieldDlg::ProcessFile(const char* name,bool encrypt,std::list<std::string>& pwd,Algorithm::Type alg)
{
	try
	{
		if (encrypt)
			p_enc->encrypt(name,pwd,alg);
		else
			p_dec->decrypt(name,pwd);
	}
	catch (std::logic_error& e)
	{
		MessageBox(e.what(),"Logic Error",MB_ICONERROR);
		return false;
	}
	catch (std::runtime_error& e)
	{
		MessageBox(e.what(),"Runtime Error",MB_ICONWARNING);
		return false;
	}

	return true;
}

void CChipherShieldDlg::OnBnClickedRadioDecrypt()
{
	if (IsDlgButtonChecked(IDC_RADIO_DECRYPT))
	{
		GetDlgItem(IDC_STATIC_ALGO)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_STATIC_ALGO)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(TRUE);
	}
}

void CChipherShieldDlg::OnBnClickedRadioEncrypt()
{
	if (IsDlgButtonChecked(IDC_RADIO_DECRYPT))
	{
		GetDlgItem(IDC_STATIC_ALGO)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_STATIC_ALGO)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_MODE)->EnableWindow(TRUE);
	}
}
