// 3FramePictViewer.h : main header file for the 3FramePictViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// C3FramePictViewerApp:
// See 3FramePictViewer.cpp for the implementation of this class
//

class C3FramePictViewerApp : public CWinApp
{
public:
	C3FramePictViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void DisableUI(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

extern C3FramePictViewerApp theApp;