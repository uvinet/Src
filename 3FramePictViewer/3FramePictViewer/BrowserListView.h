// BrowserListView.h : interface of the CBrowserListView class
//


#pragma once

#include "MainFrm.h"

class CBrowserListView : public CListView
{
	BOOL m_bInitialized;
	void InitList();
	
protected: // create from serialization only
	CBrowserListView();
	DECLARE_DYNCREATE(CBrowserListView)

// Attributes
public:
	C3FramePictViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

	TCHAR m_szListViewBuffer[MAX_PATH];

// Implementation
public:
	virtual ~CBrowserListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL PopulateListView(LPTVITEMDATA lptvid, LPSHELLFOLDER pShellFolder);

	static int CALLBACK ListViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnDeleteItem(NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnGetDispInfo(NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in BrowserListView.cpp
inline C3FramePictViewerDoc* CBrowserListView::GetDocument() const
   { return reinterpret_cast<C3FramePictViewerDoc*>(m_pDocument); }
#endif

