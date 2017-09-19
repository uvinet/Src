// BrowserTreeView.cpp : implementation of the CBrowserTreeView class
//

#include "stdafx.h"
#include "3FramePictViewer.h"

#include "3FramePictViewerDoc.h"
#include "BrowserTreeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBrowserTreeView

IMPLEMENT_DYNCREATE(CBrowserTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CBrowserTreeView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CBrowserTreeView construction/destruction

CBrowserTreeView::CBrowserTreeView() : m_bInitialized(FALSE)
{
	// TODO: add construction code here
}

CBrowserTreeView::~CBrowserTreeView()
{
}

BOOL CBrowserTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

void CBrowserTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	InitTree();

	// TODO: You may populate your TreeView with items by directly accessing
	//  its tree control through a call to GetTreeCtrl().
}

void CBrowserTreeView::InitTree()
{
	if(m_bInitialized)
		return;

	m_bInitialized = TRUE;

	GetTreeCtrl().ModifyStyle(0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_TRACKSELECT);

	// Get system image lists
	CShellItemIDList spidl;
	HRESULT hRet = ::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &spidl);
	SHFILEINFO sfi = { 0 };
	HIMAGELIST hImageListSmall = (HIMAGELIST)::SHGetFileInfo(spidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ASSERT(hImageListSmall);
	GetTreeCtrl().SendMessage(TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageListSmall);


	CComPtr<IShellFolder> spFolder;
	HRESULT hr = ::SHGetDesktopFolder(&spFolder);
	if(SUCCEEDED(hr))
	{
		CWaitCursor wait;
		PopulateTreeView(spFolder, NULL, TVI_ROOT);
		GetTreeCtrl().Expand(GetTreeCtrl().GetRootItem(), TVE_EXPAND);
		GetTreeCtrl().SelectItem(GetTreeCtrl().GetRootItem());
	}
}

void CBrowserTreeView::OnPaint()
{
	CPaintDC dc(this);

	dc.SetBkColor(GetSysColor(COLOR_WINDOW));
	CMemDC memDC(CDC::FromHandle(dc.m_hDC), &CRect(&dc.m_ps.rcPaint));
	DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

BOOL CBrowserTreeView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


// CBrowserTreeView diagnostics

#ifdef _DEBUG
void CBrowserTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CBrowserTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

C3FramePictViewerDoc* CBrowserTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(C3FramePictViewerDoc)));
	return (C3FramePictViewerDoc*)m_pDocument;
}
#endif //_DEBUG

void CBrowserTreeView::OnDeleteItem(NMHDR* pnmh, LRESULT* pRes)
{
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;
	LPTVITEMDATA lptvid = (LPTVITEMDATA)pnmtv->itemOld.lParam;
	delete lptvid;

	*pRes = 0;
}

void CBrowserTreeView::OnItemExpanding(NMHDR* pnmh, LRESULT* pRes)
{
	*pRes = 0;

	CWaitCursor wait;
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;
	if ((pnmtv->itemNew.state & TVIS_EXPANDEDONCE))
		 return;

	LPTVITEMDATA lptvid=(LPTVITEMDATA)pnmtv->itemNew.lParam;
	
	CComPtr<IShellFolder> spFolder;
	HRESULT hr=lptvid->spParentFolder->BindToObject(lptvid->lpi, 0, IID_IShellFolder, (LPVOID *)&spFolder);
	if(FAILED(hr)) 
		return;

	PopulateTreeView(spFolder, lptvid->lpifq, pnmtv->itemNew.hItem);

	TVSORTCB tvscb;
	tvscb.hParent = pnmtv->itemNew.hItem;
	tvscb.lpfnCompare = TreeViewCompareProc;
	tvscb.lParam = 0;

	TreeView_SortChildrenCB(GetTreeCtrl().m_hWnd, &tvscb, 0);
}

int CALLBACK CBrowserTreeView::TreeViewCompareProc(LPARAM lparam1, LPARAM lparam2, LPARAM /*lparamSort*/)
{
	LPTVITEMDATA lptvid1 = (LPTVITEMDATA)lparam1;
	LPTVITEMDATA lptvid2 = (LPTVITEMDATA)lparam2;

	HRESULT hr = lptvid1->spParentFolder->CompareIDs(0, lptvid1->lpi, lptvid2->lpi);

	return (int)(short)HRESULT_CODE(hr);
}

BOOL CBrowserTreeView::PopulateTreeView(IShellFolder* pShellFolder, LPITEMIDLIST lpifq, HTREEITEM hParent)
{
	if(!m_bInitialized)
		InitTree();

	CMainFrame* pmf = STATIC_DOWNCAST(CMainFrame,AfxGetMainWnd());
	if(pmf==NULL)
		return FALSE;

	if(pShellFolder == NULL)
		return FALSE;

	CComPtr<IEnumIDList> spIDList;
	HRESULT hr = pShellFolder->EnumObjects(m_hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &spIDList);
	if (FAILED(hr))
		return FALSE;

	CShellItemIDList lpi;
	CShellItemIDList lpifqThisItem;
	LPTVITEMDATA lptvid = NULL;
	ULONG ulFetched	= 0;

	TCHAR szBuff[256] = { 0 };

	TVITEM tvi = { 0 };             // TreeView Item
	TVINSERTSTRUCT tvins = { 0 };   // TreeView Insert Struct
	HTREEITEM hPrev = NULL;         // Previous Item Added

	// Hourglass on
	CWaitCursor wait;

	int iCnt = 0;
	while (spIDList->Next(1, &lpi, &ulFetched) == S_OK)
	{
		ULONG ulAttrs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER;
		pShellFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&lpi, &ulAttrs);
		if ((ulAttrs & (SFGAO_HASSUBFOLDER | SFGAO_FOLDER)) != 0)
		{
			// We need this next if statement so that we don't add things like
			// the MSN to our tree. MSN is not a folder, but according to the
			// shell is has subfolders....
			if (ulAttrs & SFGAO_FOLDER)
			{
				tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

				if (ulAttrs & SFGAO_HASSUBFOLDER)
				{
					// This item has sub-folders, so let's put the + in the TreeView.
					// The first time the user clicks on the item, we'll populate the
					// sub-folders then.
					tvi.cChildren = 1;
					tvi.mask |= TVIF_CHILDREN;
				}

				// OK, let's get some memory for our ITEMDATA struct
				lptvid = new TVITEMDATA;
				if (lptvid == NULL)
					return FALSE;

				// Now get the friendly name that we'll put in the treeview...
				if (!pmf->m_ShellMgr.GetName(pShellFolder, lpi, SHGDN_NORMAL, szBuff))
					return FALSE;

				tvi.pszText = szBuff;
				tvi.cchTextMax = MAX_PATH;

				lpifqThisItem = pmf->m_ShellMgr.ConcatPidls(lpifq, lpi);

				// Now, make a copy of the ITEMIDLIST
				lptvid->lpi=pmf->m_ShellMgr.CopyITEMID(lpi);

				pmf->m_ShellMgr.GetNormalAndSelectedIcons(lpifqThisItem, &tvi);

				lptvid->spParentFolder.p = pShellFolder;    // Store the parent folders SF
				pShellFolder->AddRef();

				lptvid->lpifq = pmf->m_ShellMgr.ConcatPidls(lpifq, lpi);

				tvi.lParam = (LPARAM)lptvid;

				tvins.item = tvi;
				tvins.hInsertAfter = hPrev;
				tvins.hParent = hParent;

				hPrev = TreeView_InsertItem(GetTreeCtrl().m_hWnd, &tvins);

				int nIndent = 0;
				while(NULL != (hPrev = (HTREEITEM)GetTreeCtrl().SendMessage(TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hPrev)))
				{
					nIndent++;
				}
			}

			lpifqThisItem = NULL;
		}

		lpi = NULL;   // Finally, free the pidl that the shell gave us...

	}

	return TRUE;
}

// CBrowserTreeView message handlers
