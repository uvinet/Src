// 3FramePictViewerDoc.h : interface of the C3FramePictViewerDoc class
//


#pragma once


class C3FramePictViewerDoc : public CDocument
{
protected: // create from serialization only
	C3FramePictViewerDoc();
	DECLARE_DYNCREATE(C3FramePictViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~C3FramePictViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


