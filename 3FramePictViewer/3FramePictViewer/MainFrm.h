// MainFrm.h : interface of the CMainFrame class
//


#pragma once

class CBrowserListView;
class CBrowserTreeView;
class CPictView;


////////////////////////////////////
//				Helper classes

///////////////////////////////////////
//CShellMgr
class CShellMgr
{
public:
	int GetIconIndex(LPITEMIDLIST lpi, UINT uFlags);

	void GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTVITEM lptvitem);

	LPITEMIDLIST ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

	BOOL GetName (LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPTSTR lpFriendlyName);
	LPITEMIDLIST Next(LPCITEMIDLIST pidl);
	UINT GetSize(LPCITEMIDLIST pidl);
	LPITEMIDLIST CopyITEMID(LPITEMIDLIST lpi);

	LPITEMIDLIST GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi);

	BOOL DoContextMenu(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpi, POINT point);
};
//CShellMgr
///////////////////////////////////////

class CShellItemIDList
{
public:
	LPITEMIDLIST m_pidl;

	CShellItemIDList(LPITEMIDLIST pidl = NULL) : m_pidl(pidl)
	{ }

	~CShellItemIDList()
	{
		::CoTaskMemFree(m_pidl);
	}

	void Attach(LPITEMIDLIST pidl)
	{
		::CoTaskMemFree(m_pidl);
		m_pidl = pidl;
	}

	LPITEMIDLIST Detach()
	{
		LPITEMIDLIST pidl = m_pidl;
		m_pidl = NULL;
		return pidl;
	}

	bool IsNull() const
	{
		return (m_pidl == NULL);
	}

	CShellItemIDList& operator =(LPITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	LPITEMIDLIST* operator &()
	{
		return &m_pidl;
	}

	operator LPITEMIDLIST()
	{
		return m_pidl;
	}

	operator LPCTSTR()
	{
		return (LPCTSTR)m_pidl;
	}

	operator LPTSTR()
	{
		return (LPTSTR)m_pidl;
	}

	void CreateEmpty(UINT cbSize)
	{
		::CoTaskMemFree(m_pidl);
		m_pidl = (LPITEMIDLIST)::CoTaskMemAlloc(cbSize);
		ATLASSERT(m_pidl != NULL);
		if(m_pidl != NULL)
			memset(m_pidl, 0, cbSize);
	}
};

typedef struct _LVItemData
{
	_LVItemData() : ulAttribs(0)
	{ }
	
	CComPtr<IShellFolder> spParentFolder;
	
	CShellItemIDList lpi;
	ULONG ulAttribs;

} LVITEMDATA, *LPLVITEMDATA;


typedef struct _TVItemData
{
	_TVItemData()
	{ }
	
	CComPtr<IShellFolder> spParentFolder;
	
	CShellItemIDList lpi;
	CShellItemIDList lpifq;

} TVITEMDATA, *LPTVITEMDATA;


//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-2002, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support. - KR
//
//           11/3/99 Fixed most common complaint. Added
//                   background color fill. - KR
//
//           11/3/99 Added support for mapping modes other than
//                   MM_TEXT as suggested by Lee Sang Hun. - KR
//
//           02/11/02 Added support for CScrollView as supplied
//                    by Gary Kirkham. - KR
//
// This class implements a memory Device Context which allows
// flicker free drawing.

class CMemDC : public CDC {
private:	
	CBitmap		m_bitmap;		// Offscreen bitmap
	CBitmap*	m_oldBitmap;	// bitmap originally found in CMemDC
	CDC*		m_pDC;			// Saves CDC passed in constructor
	CRect		m_rect;			// Rectangle of drawing area.
	BOOL		m_bMemDC;		// TRUE if CDC really is a Memory DC.
public:

	CMemDC(CDC* pDC, const CRect* pRect = NULL) : CDC()
	{
		ASSERT(pDC != NULL); 

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();

		// Get the rectangle to draw
		if (pRect == NULL) {
			pDC->GetClipBox(&m_rect);
		} else {
			m_rect = *pRect;
		}

		if (m_bMemDC) {
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			pDC->LPtoDP(&m_rect);

			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);

			SetMapMode(pDC->GetMapMode());

			SetWindowExt(pDC->GetWindowExt());
			SetViewportExt(pDC->GetViewportExt());

			pDC->DPtoLP(&m_rect);
			SetWindowOrg(m_rect.left, m_rect.top);
		} else {
			// Make a copy of the relevent parts of the current DC for printing
			m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}

		// Fill background 
		FillSolidRect(m_rect, pDC->GetBkColor());
	}

	~CMemDC()	
	{		
		if (m_bMemDC) {
			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);			

			//Swap back the original bitmap.
			SelectObject(m_oldBitmap);
		} else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.			
			m_hDC = m_hAttribDC = NULL;
		}	
	}

	// Allow usage as a pointer	
	CMemDC* operator->() 
	{
		return this;
	}	

	// Allow usage as a pointer	
	operator CMemDC*() 
	{
		return this;
	}
};

//				Helper classes
////////////////////////////////////

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndHorzSplitter;
	CBrowserTreeView*	m_pBrowserTreeView;
	CBrowserListView*	m_pBrowserListView;
	CPictView*			m_pPictView;
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	CBrowserTreeView* GetTree() const { return m_pBrowserTreeView; }
	CBrowserListView* GetList() const { return m_pBrowserListView; }
	CPictView* GetPictView() const { return m_pPictView; }

// Implementation
public:
	virtual ~CMainFrame();
	CBrowserListView* GetRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CShellMgr m_ShellMgr;

// Generated message map functions
protected:
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
	afx_msg void OnViewStyle(UINT nCommandID);
	afx_msg void OnTVSelChanged(UINT id, NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnLVDblClick(UINT id, NMHDR* pnmh, LRESULT* pRes);
	afx_msg void OnLVItemChanged(UINT id, NMHDR* pnmh, LRESULT* pRes);
	DECLARE_MESSAGE_MAP()
};


