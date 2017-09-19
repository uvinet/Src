// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "3FramePictViewer.h"

#include "MainFrm.h"
#include "BrowserTreeView.h"
#include "BrowserListView.h"
#include "PictView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CMainFrame::OnUpdateViewStyles)
	ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CMainFrame::OnViewStyle)
	
	ON_NOTIFY_RANGE(TVN_SELCHANGED, 0, 0xffff, OnTVSelChanged)
	ON_NOTIFY_RANGE(NM_DBLCLK, 0, 0xffff, OnLVDblClick)
	ON_NOTIFY_RANGE(LVN_ITEMCHANGED, 0, 0xffff, OnLVItemChanged)

END_MESSAGE_MAP()


// CMainFrame construction/destruction

CMainFrame::CMainFrame() : 	m_pBrowserTreeView(NULL), m_pBrowserListView(NULL), m_pPictView(NULL)
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	if( !m_wndSplitter.CreateStatic(this,1,2, WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE) ) 
		return FALSE;

	if(!m_wndHorzSplitter.CreateStatic(&m_wndSplitter, 2, 1, WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 1)))
		return FALSE;

	if(!m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CBrowserTreeView),CSize(400,100),pContext))
		return FALSE;
	
	if(!m_wndHorzSplitter.CreateView(0,0,RUNTIME_CLASS(CBrowserListView),   CSize(100,200) ,pContext))
		return FALSE;

	if( !m_wndHorzSplitter.CreateView(1,0,RUNTIME_CLASS(CPictView),CSize(100,200),pContext) )
		return FALSE;

	m_pBrowserTreeView = DYNAMIC_DOWNCAST(CBrowserTreeView, m_wndSplitter.GetPane(0,0));
	m_pBrowserListView = DYNAMIC_DOWNCAST(CBrowserListView, m_wndHorzSplitter.GetPane(0,0));
	m_pPictView = DYNAMIC_DOWNCAST(CPictView, m_wndHorzSplitter.GetPane(1,0));

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// remove "Untitled"
	cs.style &= ~FWS_ADDTOTITLE;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


CBrowserListView* CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CBrowserListView* pView = DYNAMIC_DOWNCAST(CBrowserListView, pWnd);
	return pView;
}

void CMainFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	if (!pCmdUI)
		return;

	// TODO: customize or extend this code to handle choices on the View menu

	CBrowserListView* pView = GetRightPane(); 

	// if the right-hand pane hasn't been created or isn't a view,
	// disable commands in our range

	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		// if the command is ID_VIEW_LINEUP, only enable command
		// when we're in LVS_ICON or LVS_SMALLICON mode

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// otherwise, use dots to reflect the style of the view
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}

			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}


void CMainFrame::OnViewStyle(UINT nCommandID)
{
	// TODO: customize or extend this code to handle choices on the View menu
	CBrowserListView* pView = GetRightPane();

	// if the right-hand pane has been created and is a CBrowserListView,
	// process the menu commands...
	if (pView != NULL)
	{
		DWORD dwStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				// ask the list control to snap to grid
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		// other commands change the style on the list control
		case ID_VIEW_DETAILS:
			dwStyle = LVS_REPORT;
			break;

		case ID_VIEW_SMALLICON:
			dwStyle = LVS_SMALLICON;
			break;

		case ID_VIEW_LARGEICON:
			dwStyle = LVS_ICON;
			break;

		case ID_VIEW_LIST:
			dwStyle = LVS_LIST;
			break;
		}

		// change the style; window will repaint automatically
		if (dwStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
	}
}

void CMainFrame::OnTVSelChanged(UINT id, NMHDR* pnmh, LRESULT* pRes)
{
	*pRes = 0;

	if(m_pBrowserListView==NULL || m_pBrowserTreeView==NULL)
		return;

	if(m_pBrowserTreeView->GetTreeCtrl().m_hWnd != pnmh->hwndFrom)
		return;

	LPNMTREEVIEW lpTV = (LPNMTREEVIEW)pnmh;
	LPTVITEMDATA lptvid = (LPTVITEMDATA)lpTV->itemNew.lParam;
	if (lptvid != NULL)
	{
		CComPtr<IShellFolder> spFolder;
		HRESULT hr=lptvid->spParentFolder->BindToObject(lptvid->lpi, 0, IID_IShellFolder, (LPVOID *)&spFolder);
		if(FAILED(hr))
			return;
		
		if(m_pBrowserListView->GetListCtrl().GetItemCount() > 0)
			m_pBrowserListView->GetListCtrl().DeleteAllItems();
		m_pBrowserListView->PopulateListView(lptvid, spFolder);
	}
}

void CMainFrame::OnLVDblClick(UINT id, NMHDR* pnmh, LRESULT* pRes)
{
	if(m_pBrowserListView==NULL || m_pBrowserListView->GetListCtrl().m_hWnd != pnmh->hwndFrom)
		return;

	// пока не работает :(
	ASSERT(NULL);
	*pRes = 0;
}

int IsPict(LPCTSTR pstrFileName)
{
	CString fileName(pstrFileName);
	CString ext3=fileName.Right(3);
	CString ext4=fileName.Right(4);
	
	if(ext3.CompareNoCase(_T("bmp"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("gif"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("jpg"))==0 || ext4.CompareNoCase(_T("jpeg"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("png"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("mng"))==0 || ext3.CompareNoCase(_T("jng"))==0 ||ext3.CompareNoCase(_T("png"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("ico"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("tif"))==0 || ext4.CompareNoCase(_T("tiff"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("tga"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("pcx"))==0)
		return TRUE;

	if(ext4.CompareNoCase(_T("wbmp"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("wmf"))==0 || ext3.CompareNoCase(_T("emf"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("j2k"))==0 || ext3.CompareNoCase(_T("jp2"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("jbg"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("j2k"))==0 || ext3.CompareNoCase(_T("jp2"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("j2c"))==0 || ext3.CompareNoCase(_T("jpc"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("pgx"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("pnm"))==0 || ext3.CompareNoCase(_T("pgm"))==0 || ext3.CompareNoCase(_T("ppm"))==0)
		return TRUE;

	if(ext3.CompareNoCase(_T("ras"))==0)
		return TRUE;

	return FALSE;
}

void CMainFrame::OnLVItemChanged(UINT id, NMHDR* pnmh, LRESULT* pRes)
{
	*pRes = 0;

	if(m_pPictView==NULL || m_pBrowserListView==NULL || pnmh->hwndFrom!=m_pBrowserListView->GetListCtrl().m_hWnd)
		return;

	m_pPictView->Navigate2(_T("about:blank"), 0, NULL);

	NMLISTVIEW* pnmlv = (NMLISTVIEW*)pnmh;
	
	if(pnmlv->uNewState & LVIS_SELECTED)
	{
		LPLVITEMDATA lplvid=(LPLVITEMDATA)pnmlv->lParam;

		if(lplvid == NULL)
			return ;

		if ((lplvid->ulAttribs & SFGAO_FOLDER)==0)
		{
			TCHAR szBuff[MAX_PATH] = { 0 };
			if(m_ShellMgr.GetName(lplvid->spParentFolder, lplvid->lpi, SHGDN_FORPARSING, szBuff) && IsPict(szBuff))
			{
				m_pPictView->Navigate2(szBuff, 0, NULL);
			}
		}
	}
}



///////////////////////////////////////
//CShellMgr

int CShellMgr::GetIconIndex(LPITEMIDLIST lpi, UINT uFlags)
{
	SHFILEINFO sfi = { 0 };
	DWORD_PTR dwRet = ::SHGetFileInfo((LPCTSTR)lpi, 0, &sfi, sizeof(SHFILEINFO), uFlags);
	return (dwRet != 0) ? sfi.iIcon : -1;
}

void CShellMgr::GetNormalAndSelectedIcons(LPITEMIDLIST lpifq, LPTVITEM lptvitem)
{
	int nRet = lptvitem->iImage = GetIconIndex(lpifq, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ATLASSERT(nRet >= 0);
	nRet = lptvitem->iSelectedImage = GetIconIndex(lpifq, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON);
	ATLASSERT(nRet >= 0);
}

LPITEMIDLIST CShellMgr::ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	UINT cb1 = 0;
	if (pidl1 != NULL)   // May be NULL
		cb1 = GetSize(pidl1) - sizeof(pidl1->mkid.cb);

	UINT cb2 = GetSize(pidl2);

	LPITEMIDLIST pidlNew = (LPITEMIDLIST)::CoTaskMemAlloc(cb1 + cb2);
	if (pidlNew != NULL)
	{
		if (pidl1 != NULL)
			memcpy(pidlNew, pidl1, cb1);

		memcpy(((LPSTR)pidlNew) + cb1, pidl2, cb2);
	}

	return pidlNew;
}

BOOL CShellMgr::GetName(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi, DWORD dwFlags, LPTSTR lpFriendlyName)
{
	BOOL bSuccess = TRUE;
	STRRET str = { STRRET_CSTR };

	if (lpsf->GetDisplayNameOf(lpi, dwFlags, &str) == NOERROR)
	{
		USES_CONVERSION;

		switch (str.uType)
		{
		case STRRET_WSTR:
			lstrcpy(lpFriendlyName, W2CT(str.pOleStr));
			::CoTaskMemFree(str.pOleStr);
			break;
		case STRRET_OFFSET:
			lstrcpy(lpFriendlyName, (LPTSTR)lpi + str.uOffset);
			break;
		case STRRET_CSTR:
			lstrcpy(lpFriendlyName, A2CT(str.cStr));
			break;
		default:
			bSuccess = FALSE;
			break;
		}
	}
	else
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}

LPITEMIDLIST CShellMgr::Next(LPCITEMIDLIST pidl)
{
	LPSTR lpMem = (LPSTR)pidl;
	lpMem += pidl->mkid.cb;
	return (LPITEMIDLIST)lpMem;
}

UINT CShellMgr::GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl != NULL)
	{
		cbTotal += sizeof(pidl->mkid.cb);   // Null terminator
		while (pidl->mkid.cb != NULL)
		{
			cbTotal += pidl->mkid.cb;
			pidl = Next(pidl);
		}
	}

	return cbTotal;
}

LPITEMIDLIST CShellMgr::CopyITEMID(LPITEMIDLIST lpi)
{
	LPITEMIDLIST lpiTemp = (LPITEMIDLIST)::CoTaskMemAlloc(lpi->mkid.cb + sizeof(lpi->mkid.cb));
	::CopyMemory((PVOID)lpiTemp, (CONST VOID*)lpi, lpi->mkid.cb + sizeof(lpi->mkid.cb));
	return lpiTemp;
}

LPITEMIDLIST CShellMgr::GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi)
{
	TCHAR szBuff[MAX_PATH] = { 0 };

	if (!GetName(lpsf, lpi, SHGDN_FORPARSING, szBuff))
		return NULL;

	CComPtr<IShellFolder> spDeskTop;
	HRESULT hr = ::SHGetDesktopFolder(&spDeskTop);
	if (FAILED(hr))
		return NULL;

	ULONG ulEaten = 0;
	LPITEMIDLIST lpifq = NULL;
	ULONG ulAttribs = 0;
	USES_CONVERSION;
	hr = spDeskTop->ParseDisplayName(NULL, NULL, T2W(szBuff), &ulEaten, &lpifq, &ulAttribs);

	if (FAILED(hr))
		return NULL;

	return lpifq;
}

BOOL CShellMgr::DoContextMenu(HWND hWnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST lpi, POINT point)
{
	CComPtr<IContextMenu> spContextMenu;
	HRESULT hr = lpsfParent->GetUIObjectOf(hWnd, 1, (const struct _ITEMIDLIST**)&lpi, IID_IContextMenu, 0, (LPVOID*)&spContextMenu);
	if(FAILED(hr))
		return FALSE;

	HMENU hMenu = ::CreatePopupMenu();
	if(hMenu == NULL)
		return FALSE;

	// Get the context menu for the item.
	hr = spContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_EXPLORE);
	if(FAILED(hr))
		return FALSE;

	int idCmd = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);

	if (idCmd != 0)
	{
		USES_CONVERSION;

		// Execute the command that was selected.
		CMINVOKECOMMANDINFO cmi = { 0 };
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.fMask = 0;
		cmi.hwnd = hWnd;
		cmi.lpVerb = T2CA(MAKEINTRESOURCE(idCmd - 1));
		cmi.lpParameters = NULL;
		cmi.lpDirectory = NULL;
		cmi.nShow = SW_SHOWNORMAL;
		cmi.dwHotKey = 0;
		cmi.hIcon = NULL;
		hr = spContextMenu->InvokeCommand(&cmi);
	}

	::DestroyMenu(hMenu);

	return TRUE;
}

//CShellMgr
///////////////////////////////////////