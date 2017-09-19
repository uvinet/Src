// BrowserTreeView.h : interface of the CBrowserTreeView class
//


#pragma once

class C3FramePictViewerDoc;

#include "MainFrm.h"

class CBrowserTreeView : public CTreeView
{
	BOOL m_bInitialized;
	void InitTree();

protected: // create from serialization only
	CBrowserTreeView();
	DECLARE_DYNCREATE(CBrowserTreeView)

// Attributes
public:
	C3FramePictViewerDoc* GetDocument();

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CBrowserTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	BOOL PopulateTreeView(IShellFolder* pShellFolder, LPITEMIDLIST lpifq, HTREEITEM hParent);

	static int CALLBACK TreeViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnDeleteItem(NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnItemExpanding(NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in BrowserTreeView.cpp
inline C3FramePictViewerDoc* CBrowserTreeView::GetDocument()
   { return reinterpret_cast<C3FramePictViewerDoc*>(m_pDocument); }
#endif

