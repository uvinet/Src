// 3FramePictViewerDoc.cpp : implementation of the C3FramePictViewerDoc class
//

#include "stdafx.h"
#include "3FramePictViewer.h"

#include "3FramePictViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C3FramePictViewerDoc

IMPLEMENT_DYNCREATE(C3FramePictViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(C3FramePictViewerDoc, CDocument)
END_MESSAGE_MAP()


// C3FramePictViewerDoc construction/destruction

C3FramePictViewerDoc::C3FramePictViewerDoc()
{
	// TODO: add one-time construction code here

}

C3FramePictViewerDoc::~C3FramePictViewerDoc()
{
}

BOOL C3FramePictViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// C3FramePictViewerDoc serialization

void C3FramePictViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// C3FramePictViewerDoc diagnostics

#ifdef _DEBUG
void C3FramePictViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void C3FramePictViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// C3FramePictViewerDoc commands
