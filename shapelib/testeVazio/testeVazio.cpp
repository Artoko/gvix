// testeVazio.cpp : Defines the entry point for the application.
//

// http://www.codeproject.com/Articles/32035/Rendering-Shapefile-in-OpenGL
// Rendering Shapefile in OpenGL

#include "stdafx.h"
#include "testeVazio.h"

#include "GLES/glutes.h"
//#include <GL/glut.h>

#include "GLES/gl.h"

#include "shapelib_ce2.h"

#include <vector>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// GDI Support for Windows CE
// http://msdn.microsoft.com/en-us/library/ms903434.aspx
/*

Windows CE GDI does not support the following features:

    Transformation functions of coordinate space, such as SetMapMode, GetMapMode, SetViewportExt, and SetWindowExt. 
	Coordinate space is equivalent to device space.
    World Transform API.
    MoveTo and LineTo functions.

*/

// VSMathLib - Very Simple Matrix Library
// http://www.lighthouse3d.com/very-simple-libs/vsml/


//Program to Parse ESRI Shapefile and render it in OpenGL
//Author : Dhulipudi Durga Prasad
//Contact: dgplinux@yahoo.com
//Libraries Used: Shapelib ,OpenGL

//#include "GLES/glutes.h"
////#include <GL/glut.h>
////#include <stdlib.h>
// 
////#include "shapelib\shapefil.h"
//#include "shapefil.h"
//
//#include <vector>

using namespace std;

/// number of settable matrices
#define COUNT_MATRICES 4

/// Enumeration of the matrix types
///* MatrixMode */
//#define GL_MODELVIEW                      0x1700
//#define GL_PROJECTION                     0x1701
//#define GL_TEXTURE                        0x1702
enum MatrixTypes{
        MODEL,
        VIEW,
        PROJECTION,  // GL_PROJECTION
        AUX0,
        AUX1,
        AUX2,
        AUX3
} ;

/// The storage for matrices
float mMatrix[COUNT_MATRICES][16];
//float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

void
multMatrix(MatrixTypes aType, float *aMatrix);

/** Similar to glOrtho and gluOrtho2D (just leave the
  * last two params blank).
  *
  * \param left,right coordinates for the left and right vertical
  * clipping planes
  * \param bottom,top coordinates for the bottom and top horizontal
  * clipping planes
  * \param nearp,farp distance to the near and far planes
*/
void glOrtho(float left, float right, float bottom, float top,
                float nearp=-1.0f, float farp=1.0f);

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void
setIdentityMatrix( float *mat, int size);

// Compute res = M * point
void
multMatrixPoint(MatrixTypes aType, float *point, float *res);
//multMatrixPoint(float *point, float *res);

typedef struct MyPoint2D
{
 double dX;
 double dY;
 
}MyPoint2D;
 
//Holds Coordinates of Point Shapefile
vector<MyPoint2D> vPoints;
 


typedef struct MyLineString2D
{
 vector<MyPoint2D> vPointList;
 
}MyLineString2D;

//Holds Coordinates of Line Shapefile
vector<MyLineString2D> vLines;
  


typedef struct MyPolygon2D
{
 vector<MyPoint2D> vPointList;
 
}MyPolygon2D;
//Holds Coordinates of Polygon Shapefile 
vector<MyPolygon2D> vPolygons;
  



typedef struct SBoundingBox
{
 float fMaxX;
 float fMaxY;
 float fMinX;
 float fMinY;
 
}SBoundingBox;
//Bounding Box of Shapefile 
SBoundingBox sBoundingBox;

GLAPI void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

//Function to Open Shapefile and parse the info 

void OpenShapeFile(char* fileName)
{
           
    SHPHandle hSHP=SHPOpen(fileName, "rb" );


	//Read Bounding Box of Shapefile
    sBoundingBox.fMaxX=(float)hSHP->adBoundsMax[0];
    sBoundingBox.fMaxY=(float)hSHP->adBoundsMax[1];

    sBoundingBox.fMinX=(float)hSHP->adBoundsMin[0];
    sBoundingBox.fMinY=(float)hSHP->adBoundsMin[1];

    if(hSHP == NULL) return;  

	//Point Shapefile
    if(hSHP->nShapeType == SHPT_POINT)
    {
        SHPObject *psShape;
        for(int i=0;i<hSHP->nRecords;i++)
        {
            psShape = SHPReadObject(hSHP, i);

            double fX = psShape->padfX[0];
            double fY = -psShape->padfY[0];

            //Plot these points
            MyPoint2D pt;
            pt.dX=fX;
            pt.dY=-fY;
            vPoints.push_back(pt);
        }
    }


	//Line Shapefile
    else if(hSHP->nShapeType == SHPT_ARC)
    {
        SHPObject *psShape;
	    for(int i=0;i<hSHP->nRecords;i++)
        {
            psShape = SHPReadObject(hSHP, i);
            vector<MyPoint2D> tempPointArray;

            for(int j=0;j<psShape->nVertices;j++)
            {
				double fX = psShape->padfX[j];
				double fY = psShape->padfY[j];
				MyPoint2D pt;
				pt.dX=fX;
				pt.dY=fY;
				tempPointArray.push_back(pt);
            }
           
			MyLineString2D linestring;
            linestring.vPointList=tempPointArray;
            vLines.push_back(linestring);
           
        }
    }

 	//Polygon Shapefile
    if(hSHP->nShapeType == SHPT_POLYGON)
    {
		SHPObject *psShape;
		for(int i=0;i<hSHP->nRecords;i++)
		{
			psShape = SHPReadObject(hSHP, i);
      		vector<MyPoint2D> tempPointArray;
     
			for(int j=0;j<psShape->nVertices;j++)
			{
				double fX = psShape->padfX[j];
				double fY = psShape->padfY[j];
				MyPoint2D pt;
				pt.dX=fX;
				pt.dY=fY;
      			tempPointArray.push_back(pt);
			}
			MyPolygon2D polygon;
			polygon.vPointList=tempPointArray;
			vPolygons.push_back(polygon);
		}

    }
  
}


void initializeGL()
{
	glClearColor (0.0, 0.0, 0.0, 0.0);

	glShadeModel (GL_FLAT);

	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

	//Assign Default Map Bounds to glOrtho
	sBoundingBox.fMaxX=180.0f;
	sBoundingBox.fMaxY=90.0f;
	sBoundingBox.fMinX=-180.0f;
	sBoundingBox.fMinY=-90.0f;
}

void resizeGL(int w, int h)
{
	if(h<=0) h=1 ;

	//  glViewport defines the area of the window you are using into which you want to draw. 
	// Here is how to tell OpenGL that you want to use your entire window to draw in: 
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	//Assign Bounding Box Coordinates of Shapefile to glOrtho()
	// 06-21-2012 Implemented local function glOrtho by Gerson in replacement because of lack in OpenGL ES for WinCE / PPC / WinMob
	// Original implementation is at OpenGL Win32/x86 for Windows at GL.h in C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\gl
	//glOrtho(sBoundingBox.fMinX, sBoundingBox.fMaxX,sBoundingBox.fMinY,sBoundingBox.fMaxY,-1,1);
	glOrthof(sBoundingBox.fMinX, sBoundingBox.fMaxX,sBoundingBox.fMinY,sBoundingBox.fMaxY,-1,1);

	// OpenGL ES 2: Who stole all my functions? 
	// http://igamedev.posterous.com/opengl-es-2-who-stole-all-my-functions
	// But isn't that a lot of extra work?
	// Yup, sure is. Until you've built up some reusable code, it's a ton of extra work. 
	// Hopefully as I write these tutorials you'll get some code that you can use.

	// http://forums.macrumors.com/showthread.php?t=593661
	//So... most of the functions don't work. glOrtho, glBegin, glEnd, glVertex3f (and 2f), all got errors. 
	//What would be the proper OpenGL ES functions ...

	// Here, totally free for everyone, is my current graphics engine 
	// http://www.khronos.org/message_boards/viewtopic.php?f=9&t=3140

	// http://www.khronos.org/opengles/sdk/1.1/docs/man/

	// implementation:
	// http://www.khronos.org/opengles/documentation/opengles1_0/html/glOrtho.html

	// http://stackoverflow.com/questions/7131037/how-do-you-implement-glortho-for-opengles-2-0-with-or-without-tx-ty-tz-values-f
	// implement my own glOtho function

	// The glOrtho function multiplies the current matrix by an orthographic matrix.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd373965%28v=vs.85%29.aspx

	//http://stackoverflow.com/questions/7131037/how-do-you-implement-glortho-for-opengles-2-0-with-or-without-tx-ty-tz-values-f

	glMatrixMode(GL_MODELVIEW);
}

void draw()
{
  
	glClear (GL_COLOR_BUFFER_BIT);

	// 06-21-2012 by Gerson This function of OpenGL is not supported by mobileplatforms
	//	glColor3f (0.0, 0.0, 1.0);
	// The alpha channel is normally used as an opacity channel. 
	//If a pixel has a value of 0% in its alpha channel, 
	//it is fully transparent (and, thus, invisible), 
	//whereas a value of 100% in the alpha channel gives a fully opaque pixel (traditional digital images)
	glColor4x(0, 0, 1, 1); 

	glLoadIdentity ();
	
	//Render Point Shapefile
	//glColor3f (0.0, 0.0, 1.0);
	glColor4x(0, 0, 1, 1); 

	glEnable(GL_POINT_SMOOTH) ;
	glPointSize(5.0);

	//Two of the more significant differences between OpenGL ES and OpenGL are the removal of the glBegin ... 
	//glEnd calling semantics for primitive rendering (in favor of vertex arrays) and the introduction 
	//of fixed-point data types for vertex coordinates and attributes to better support the 
	//computational abilities of embedded processors, which often lack a floating point unit (FPU).

	// The glBegin and glend functions delimit the vertices of a primitive or a group of like primitives.
	//glBegin(GL_POINTS);

	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
	GLfloat vPoints_OpenGLMobile[250];
	int vPoints_count=0;
	// end 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	
	for(int i=0; i < (int)vPoints.size();i++)
	{
		// Specifies a vertex.
		/*
		The glVertex function commands are used within glBegin/glEnd pairs to specify point, line, and polygon vertices. 
		The current color, normal, and texture coordinates are associated with the vertex when glVertex is called. 
		When only x and y are specified, z defaults to 0.0 and w defaults to 1.0. 
		When x, y, and z are specified, w defaults to 1.0. 
		Invoking glVertex outside of a glBegin/glEnd pair results in undefined behavior. 
		*/
		//glVertex2f(vPoints[i].dX,vPoints[i].dY);

		// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		vPoints_OpenGLMobile[vPoints_count++]=(GLfloat)vPoints[i].dX;
		vPoints_OpenGLMobile[vPoints_count++]=(GLfloat)vPoints[i].dY;
		// 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)

	}

	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
	//glEnableClientState (GL_VERTEX_ARRAY);
	//glEnableClientState (GL_COLOR_ARRAY); // enables the color-array.

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vPoints_OpenGLMobile); // set your vertex-coordinates here..
	//glColorPointer (...   // set your color-coordinates here..
	glDrawArrays(GL_TRIANGLE_FAN,0,(int)vPoints.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	// END 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	
//	glEnd();
	
	//Render Line Shapefile
	//glColor3f (0.0, 1.0, 0.0);
	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	glColor4x(0, 0, 1, 0);
	// end 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)

	for( int i=0; i < (int)vLines.size();i++)
	{

		// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
		int vLines_OpenGLMobile[250];
		int vLines_count=0;
		// end 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		
//		glBegin(GL_LINE_STRIP);
		for(int j=0; j < (int)vLines[i].vPointList.size();j++)
		{
		    //glVertex2f(vLines[i].vPointList[j].dX,vLines[i].vPointList[j].dY);

			// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
			vLines_OpenGLMobile[vLines_count++]=(int)vLines[i].vPointList[j].dX;
			vLines_OpenGLMobile[vLines_count++]=(int)vLines[i].vPointList[j].dY;
			// 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	
		}

		// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vLines_OpenGLMobile); // set your vertex-coordinates here..
		//glColorPointer (...   // set your color-coordinates here..
		glDrawArrays(GL_TRIANGLE_FAN,0,vLines_count);
		glDisableClientState(GL_VERTEX_ARRAY);
		// END 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		
//		glEnd();
	}	
	
	//Render Polygon Shapefile
	//glColor3f(1.0,0.0, 0.0);
	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	glColor4x(1, 0, 0, 0);
	// end 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)


	for(int i=0; i < (int)vPolygons.size();i++)
	{

		// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
		int vPolygons_OpenGLMobile[250];
		int vPolygons_count=0;
		// end 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)

//		glBegin(GL_LINE_LOOP);
		for(int j=0; j < (int)vPolygons[i].vPointList.size();j++)
		{
			//glVertex2f(vPolygons[i].vPointList[j].dX,vPolygons[i].vPointList[j].dY);

			// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
			vPolygons_OpenGLMobile[vPolygons_count++]=(int)vPolygons[i].vPointList[j].dY;
			vPolygons_OpenGLMobile[vPolygons_count++]=(int)vPolygons[i].vPointList[j].dX;
			// 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)

		}

		// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vPolygons_OpenGLMobile); // set your vertex-coordinates here..
		//glColorPointer (...   // set your color-coordinates here..
		glDrawArrays(GL_TRIANGLE_FAN,0,vPolygons_count);
		glDisableClientState(GL_VERTEX_ARRAY);
		// END 06-21-2012 by Gerson - END of porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
		
//		glEnd();
	}
 
    glFlush();
}

void paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{

	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip
	//glutInit(&argc, argv);
	//glutInitDisplayMode (GLUT_SINGLE| GLUT_RGB);
	//glutInitWindowSize (600,600);
	//glutCreateWindow ("Render Shapefiles");
	
	//initializeGL();

	// C:\proj\Lifeware\clientes\Vix\implementacao\Arquivos SHP ArcGis\Arquivos SHP ArcGis
	//OpenShapeFile("C:\\proj\\Lifeware\\clientes\\Vix\\implementacao\\Arquivos SHP ArcGis\\Arquivos SHP ArcGis\\cs_quadra_area.shp");//Line Shapefile
		
	//glutReshapeFunc (resizeGL);
	//glutDisplayFunc (paintGL);
	//glutMainLoop();
	// 06-21-2012 by Gerson - porting OpenGL code to OpenGL-ES for MS Mobile platforms (WCE, PPC and Win Mobile)
	//http://stackoverflow.com/questions/835903/opengl-to-opengl-es-changing-color-of-triangles-in-a-strip

	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTEVAZIO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTEVAZIO));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_TESTEVAZIO, szWindowClass, MAX_LOADSTRING);

    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

    if (!MyRegisterClass(hInstance, szWindowClass))
    {
    	return FALSE;
    }

    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    static SHACTIVATEINFO s_sai;
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_HELP_ABOUT:
                    DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
                    break;
                case IDM_OK:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }

            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            CommandBar_Destroy(g_hWndMenuBar);
            PostQuitMessage(0);
            break;

        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

    }
    return (INT_PTR)FALSE;
}
