================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : 3FramePictViewer Project Overview
===============================================================================

The application wizard has created this 3FramePictViewer application for 
you.  This application not only demonstrates the basics of using the Microsoft 
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your 3FramePictViewer application.

3FramePictViewer.vcproj
    This is the main project file for VC++ projects generated using an application wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    application wizard.

3FramePictViewer.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    C3FramePictViewerApp application class.

3FramePictViewer.cpp
    This is the main application source file that contains the application
    class C3FramePictViewerApp.

3FramePictViewer.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\3FramePictViewer.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file 3FramePictViewer.rc.

res\My3FramePictViewer.rc2
    This file contains resources that are not edited by Microsoft 
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    Windows Explorer Style: The project will include a Windows Explorer-like
    interface, with two frames.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

BrowserTreeView.h, BrowserTreeView.cpp
    These files contain the left frame class CBrowserTreeView, which is derived from
    CTreeView.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

3FramePictViewerDoc.h, 3FramePictViewerDoc.cpp - the document
    These files contain your C3FramePictViewerDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via C3FramePictViewerDoc::Serialize).

BrowserListView.h, BrowserListView.cpp - the view of the document
    These files contain your CBrowserListView class.
    CBrowserListView objects are used to view C3FramePictViewerDoc objects.





/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.

/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named 3FramePictViewer.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

3FramePictViewer.manifest
	Application manifest files are used by Windows XP to describe an applications 
	dependency on specific versions of Side-by-Side assemblies. The loader uses this 
	information to load the appropriate assembly from the assembly cache or private 
	from the application. The Application manifest  maybe included for redistribution 
	as an external .manifest file that is installed in the same folder as the application 
	executable or it may be included in the executable in the form of a resource. 
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need 
to redistribute the MFC DLLs. If your application is in a language 
other than the operating system's locale, you will also have to 
redistribute the corresponding localized resources MFC80XXX.DLL. 
For more information on both of these topics, please see the section on 
redistributing Visual C++ applications in MSDN documentation. 

/////////////////////////////////////////////////////////////////////////////
