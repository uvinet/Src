// BrowserListView.cpp : implementation of the CBrowserListView class
//

#include "stdafx.h"
#include "3FramePictViewer.h"

#include "3FramePictViewerDoc.h"
#include "BrowserListView.h"
#include "BrowserTreeView.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBrowserListView

IMPLEMENT_DYNCREATE(CBrowserListView, CListView)

BEGIN_MESSAGE_MAP(CBrowserListView, CListView)
	ON_WM_STYLECHANGED()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CBrowserListView construction/destruction

CBrowserListView::CBrowserListView() : m_bInitialized(FALSE)
{
	// TODO: add construction code here

}

CBrowserListView::~CBrowserListView()
{
}

BOOL CBrowserListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

void CBrowserListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	InitList();

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

void CBrowserListView::OnPaint()
{
	CPaintDC dc(this);

	dc.SetBkColor(GetSysColor(COLOR_WINDOW));
	CMemDC memDC(CDC::FromHandle(dc.m_hDC), &CRect(&dc.m_ps.rcPaint));
	DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

BOOL CBrowserListView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

// CBrowserListView diagnostics

#ifdef _DEBUG
void CBrowserListView::AssertValid() const
{
	CListView::AssertValid();
}

void CBrowserListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

C3FramePictViewerDoc* CBrowserListView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(C3FramePictViewerDoc)));
	return (C3FramePictViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CBrowserListView message handlers
void CBrowserListView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}

void CBrowserListView::InitList()
{
	if(m_bInitialized)
		return;

	m_bInitialized = TRUE;

	GetListCtrl().ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_LIST | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_NOCOLUMNHEADER);
	GetListCtrl().SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT);

	// Get system image lists
	CShellItemIDList spidl;
	HRESULT hRet = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &spidl);
	hRet;	// avoid level 4 warning
	ATLASSERT(SUCCEEDED(hRet));

	SHFILEINFO sfi = { 0 };
	HIMAGELIST hImageList = (HIMAGELIST)::SHGetFileInfo(spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON);
	ASSERT(hImageList != NULL);

	memset(&sfi, 0, sizeof(SHFILEINFO));
	HIMAGELIST hImageListSmall = (HIMAGELIST)::SHGetFileInfo(spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ASSERT(hImageListSmall != NULL);

	GetListCtrl().SendMessage(LVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);
	GetListCtrl().SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImageListSmall);
}

void CBrowserListView::OnDeleteItem(NMHDR* pnmh, LRESULT* pRes)
{
	*pRes = 0;

	LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)pnmh;

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = pnmlv->iItem;
	lvi.iSubItem = 0;

	if (!GetListCtrl().GetItem(&lvi))
		return;

	LPLVITEMDATA lplvid = (LPLVITEMDATA)lvi.lParam;
	delete lplvid;
}

void CBrowserListView::OnGetDispInfo(NMHDR* pnmh, LRESULT* pRes)
{
	*pRes = 0;

	NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;
	if(plvdi == NULL)
		return;

	CMainFrame* pmf = STATIC_DOWNCAST(CMainFrame,AfxGetMainWnd());
	if(pmf==NULL && pmf->GetTree())
		return;

	LPLVITEMDATA lplvid = (LPLVITEMDATA)plvdi->item.lParam;

	HTREEITEM hti = pmf->GetTree()->GetTreeCtrl().GetSelectedItem();
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hti;

	pmf->GetTree()->GetTreeCtrl().GetItem(&tvi);
	if(tvi.lParam <= 0)
		return;

	LPTVITEMDATA lptvid = (LPTVITEMDATA)tvi.lParam;
	if (lptvid == NULL)
		return;
	
	CShellItemIDList pidlTemp = pmf->m_ShellMgr.ConcatPidls(lptvid->lpifq, lplvid->lpi);
	plvdi->item.iImage = pmf->m_ShellMgr.GetIconIndex(pidlTemp, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	if (plvdi->item.iSubItem == 0 && (plvdi->item.mask & LVIF_TEXT) )   // File Name
	{
		pmf->m_ShellMgr.GetName(lplvid->spParentFolder, lplvid->lpi, SHGDN_NORMAL, plvdi->item.pszText);
	}	
	else
	{
		CComPtr<IShellFolder2> spFolder2;
		HRESULT hr = lptvid->spParentFolder->QueryInterface(IID_IShellFolder2, (void**)&spFolder2);
		if(FAILED(hr))
			return;

		SHELLDETAILS sd = { 0 };
		sd.fmt = LVCFMT_CENTER;
		sd.cxChar = 15;
		
		hr = spFolder2->GetDetailsOf(lplvid->lpi, plvdi->item.iSubItem, &sd);
		if(FAILED(hr))
			return;

		if(sd.str.uType == STRRET_WSTR)
		{
			StrRetToBuf(&sd.str, lplvid->lpi.m_pidl, m_szListViewBuffer, MAX_PATH);
			plvdi->item.pszText=m_szListViewBuffer;
		}
		else if(sd.str.uType == STRRET_OFFSET)
		{
			plvdi->item.pszText = (LPTSTR)lptvid->lpi + sd.str.uOffset;
		}
		else if(sd.str.uType == STRRET_CSTR)
		{
			USES_CONVERSION;
			plvdi->item.pszText = A2T(sd.str.cStr);
		}
	}
	
	plvdi->item.mask |= LVIF_DI_SETITEM;
}

BOOL CBrowserListView::PopulateListView(LPTVITEMDATA lptvid, LPSHELLFOLDER pShellFolder)
{
	if(!m_bInitialized)
		InitList();

	ATLASSERT(pShellFolder != NULL);

	CMainFrame* pmf = STATIC_DOWNCAST(CMainFrame,AfxGetMainWnd());
	if(pmf==NULL)
		return FALSE;

	CComPtr<IEnumIDList> spEnumIDList;
	HRESULT hr = pShellFolder->EnumObjects(::GetParent(GetListCtrl().m_hWnd), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &spEnumIDList);
	if (FAILED(hr))
		return FALSE;

	CShellItemIDList lpifqThisItem;
	CShellItemIDList lpi;
	ULONG ulFetched = 0;
	UINT uFlags = 0;
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	int iCtr = 0;

	while (spEnumIDList->Next(1, &lpi, &ulFetched) == S_OK)
	{
		// Get some memory for the ITEMDATA structure.
		LPLVITEMDATA lplvid = new LVITEMDATA;
		if (lplvid == NULL) 
			return FALSE;

		// Since you are interested in the display attributes as well as other attributes, 
		// you need to set ulAttrs to SFGAO_DISPLAYATTRMASK before calling GetAttributesOf()
		ULONG ulAttrs = SFGAO_DISPLAYATTRMASK;
		hr = pShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&lpi, &ulAttrs);
		if(FAILED(hr)) 
			return FALSE;

		lplvid->ulAttribs = ulAttrs;

		lpifqThisItem = pmf->m_ShellMgr.ConcatPidls(lptvid->lpifq, lpi);

		lvi.iItem = iCtr;
		lvi.iSubItem = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.cchTextMax = MAX_PATH;
		uFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;
		lvi.iImage = I_IMAGECALLBACK;

		lplvid->spParentFolder.p = pShellFolder;
		pShellFolder->AddRef();

		// Now make a copy of the ITEMIDLIST
		lplvid->lpi= pmf->m_ShellMgr.CopyITEMID(lpi);

		lvi.lParam = (LPARAM)lplvid;

		// Add the item to the list view control
		int n = GetListCtrl().InsertItem(&lvi);

		/*
		GetListCtrl().SetItem(n, 1, LVIF_TEXT, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK, 0, 0, 0);
		GetListCtrl().SetItem(n, 2, LVIF_TEXT, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK, 0, 0, 0);
		GetListCtrl().SetItem(n, 3, LVIF_TEXT, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK, 0, 0, 0);
		GetListCtrl().SetItem(n, 4, LVIF_TEXT, LPSTR_TEXTCALLBACK, I_IMAGECALLBACK, 0, 0, 0);
		*/

		iCtr++;
		lpifqThisItem = NULL;
		lpi = NULL;   // free PIDL the shell gave you
	}

	GetListCtrl().SortItems(ListViewCompareProc, NULL);

	return TRUE;
}

int CALLBACK CBrowserListView::ListViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM lParamSort)
{
	LPLVITEMDATA lplvid1 = (LPLVITEMDATA)lparam1;
	LPLVITEMDATA lplvid2 = (LPLVITEMDATA)lparam2;

	HRESULT hr = 0;
	hr = lplvid1->spParentFolder->CompareIDs(0, lplvid1->lpi, lplvid2->lpi);

	return (int)(short)HRESULT_CODE(hr);
}