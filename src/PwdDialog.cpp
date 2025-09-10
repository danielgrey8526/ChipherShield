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

// PwdDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ChipherShield.h"
#include "PwdDialog.h"


// CPwdDialog dialog

IMPLEMENT_DYNAMIC(CPwdDialog, CDialog)
CPwdDialog::CPwdDialog(bool hide,CWnd* pParent /*=NULL*/)
	: CDialog(CPwdDialog::IDD, pParent), mHidePwd(hide)
{
}

CPwdDialog::~CPwdDialog()
{
}

void CPwdDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPwdDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CPwdDialog message handlers

void CPwdDialog::OnBnClickedOk()
{
	CWnd* pEdit = GetDlgItem(IDC_EDIT1);
	pEdit->SendMessage(WM_GETTEXT,256,(LPARAM)mPassword);

	CString str(mPassword);
	str.TrimLeft();
	str.TrimRight();

    if (str.GetLength())
		OnOK();
	else
		OnCancel();
}

void CPwdDialog::OnBnClickedCancel()
{
	OnCancel();
}

const char* CPwdDialog::GetPassword()
{
	return mPassword;
}

BOOL CPwdDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd* hEdit = GetDlgItem(IDC_EDIT1);

	if (mHidePwd)
	{
		hEdit->SendMessage(EM_SETPASSWORDCHAR,(WPARAM)'*',0);
	}

	hEdit->SetFocus();
	return TRUE;
}
