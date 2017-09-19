// PictView.cpp : implementation file
//

#include "stdafx.h"
#include "3FramePictViewer.h"
#include "PictView.h"
#include "MainFrm.h"

// CPictView

IMPLEMENT_DYNCREATE(CPictView, CHtmlView)

CPictView::CPictView()
{

}

CPictView::~CPictView()
{
}

void CPictView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPictView, CHtmlView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CPictView diagnostics

#ifdef _DEBUG
void CPictView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CPictView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

void CPictView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

	Navigate2(_T("about:blank"), 0, NULL);

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

void CPictView::OnPaint()
{
	CPaintDC dc(this);

	dc.SetBkColor(GetSysColor(COLOR_WINDOW));
	CMemDC memDC(CDC::FromHandle(dc.m_hDC), &CRect(&dc.m_ps.rcPaint));
	DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

BOOL CPictView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


// CPictView message handlers
