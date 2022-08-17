#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <ocidl.h>          // IPicture
#include <olectl.h>         // Support for iPicture
#include "winbgim.h"
#include "winbgitypes.h"        // Internal structures and routines
#include "dibapi.h"         // DIB functions from Microsoft
int imageheight(IMAGE *image)
{ return ((BITMAP *)image)->bmHeight;
}
int imagewidth(IMAGE *image)
{ return ((BITMAP *)image)->bmWidth;
}
void freeimage(IMAGE *image)
{ free(image);
}
IMAGE *createimage(int w, int h)
{ BITMAP *image;
  image=(BITMAP *)calloc(1,sizeof(BITMAP)+h*(w*32+7)/8);
  if(image==NULL) return NULL;
  image->bmWidth=w;
  image->bmHeight=h;
  image->bmWidthBytes=(w*32+7)/8;
  image->bmPlanes=1;
  image->bmBitsPixel=32;
  image->bmBits=((BYTE*)image)+sizeof(BITMAP);
  return image;
}
void imageputpixel(IMAGE *image,int x, int y, int color)
{ COLORREF rgb;
  if(IS_RGB_COLOR(color))
  {
    rgb=color&0x0FFFFFF;
  }
  else
  {
    rgb=BGI__Colors[color];
  }
  rgb = ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb & 0xFF0000) >> 16);
  BITMAP *im = (BITMAP *)image;
  if(x>=0 && x<im->bmWidth && y>=0 && y<im->bmHeight)
  {
    memcpy(((BYTE*)im->bmBits)+x*4+y*im->bmWidthBytes,&rgb,sizeof(COLORREF));
  }
}
int imagegetpixel(IMAGE *image, int x, int y)
{ COLORREF rgb;
  BITMAP *im=(BITMAP *)image;
  if(x>=0 && x<im->bmWidth && y>=0 && y<im->bmHeight)
  {
    memcpy(&rgb,((BYTE*)im->bmBits)+x*4+y*im->bmWidthBytes,sizeof(COLORREF));
	rgb = ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb & 0xFF0000) >> 16);
/*    for (int color = 0; color <=WHITE; color++)
    {
        if (BGI__Colors[color]==rgb)
	    return color;
    }*/
    return 0x03000000 | rgb;
  }
  return CLR_INVALID;
}

// MGM modified imagesize so that it returns zero in the case of failure.
unsigned int imagesize(int left, int top, int right, int bottom)
{
    long width, height;   // Width and height of the image in pixels
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    BITMAP b;             // The actual bitmap object for hBitmap
    long answer;          // Bytes needed to save this image
    int tries;

    // Preliminary computations
    width = 1 + abs(right - left);
    height = 1 + abs(bottom - top);
    pWndData = BGI__GetWindowDataPtr( );
    hDC = BGI__GetWinbgiDC( );

    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

    // Copy the requested region into hBitmap which is selected into hMemoryDC,
    // then get a copy of this actual bitmap so we can compute its size.
    BitBlt(hMemoryDC, 0, 0, width, height, hDC, left, top, SRCCOPY);
    GetObject(hBitmap, sizeof(BITMAP), &b);
    answer = sizeof(BITMAP) + b.bmHeight*b.bmWidthBytes;
    if (answer > UINT_MAX) answer = 0;

    // Delete resources
    BGI__ReleaseWinbgiDC( );
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp

    return (unsigned int) answer;
}


void getimage(int left, int top, int right, int bottom, IMAGE *bitmap)
{
    long width, height;   // Width and height of the image in pixels
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    BITMAP* pUser;        // A pointer into the user's buffer, used as a BITMAP
    long answer;          // Bytes needed to save this image

    // Preliminary computations
    pWndData = BGI__GetWindowDataPtr( );
    hDC = BGI__GetWinbgiDC( );
    width = 1 + abs(right - left);
    height = 1 + abs(bottom - top);

    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

    // Grab the bitmap data from hDC and put it in hMemoryDC
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hDC, left, top, SRCCOPY);
    
    // Copy the device-dependent bitmap into the user's allocated space
    pUser = (BITMAP*) bitmap;
    GetObject(hBitmap, sizeof(BITMAP), pUser);
    pUser->bmBits = (BYTE*) bitmap + sizeof(BITMAP);
    GetBitmapBits(hBitmap, pUser->bmHeight*pUser->bmWidthBytes, pUser->bmBits);

    // Delete resources
    BGI__ReleaseWinbgiDC( );
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
}

void putimage( int left, int top, IMAGE *bitmap, int op , int dwidth, int dheight)
{
    long width, height;   // Width and height of the image in pixels
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    BITMAP* pUser;        // A pointer into the user's buffer, used as a BITMAP
    int win_op;
    int trcolor;
    // Preliminary computations
    pUser = (BITMAP*) bitmap;
    width = pUser->bmWidth;
    height = pUser->bmHeight;
    pWndData = BGI__GetWindowDataPtr( );
    hDC = BGI__GetWinbgiDC( );
    if(dwidth==0 && dheight==0)
    { dwidth=width;
      dheight=height;
    }
    else if(dwidth!=0 && dheight==0)
    { dheight=double(dwidth)*height/width;
      if(dheight==0) dheight=1;
    }
    else if(dwidth==0 && dheight!=0)
    { dwidth=double(dheight)*width/height;
      if(dwidth==0) dwidth=1;
    }
    
    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, pUser->bmWidth, pUser->bmHeight);
    hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

    // Grab the bitmap data from the user's bitmap and put it in hMemoryDC
    SetBitmapBits(hBitmap, pUser->bmHeight*pUser->bmWidthBytes, pUser->bmBits);

    // Copy the bitmap from hMemoryDC to the active hDC:
    switch (op)
    {
    case COPY_PUT:
        win_op=SRCCOPY;
	break;
    case XOR_PUT:
	win_op=SRCINVERT;
	break;
    case OR_PUT:
	win_op=SRCPAINT;
	break;
    case AND_PUT:
	win_op=SRCAND;
	break;
    case NOT_PUT:
	win_op=NOTSRCCOPY;
	break;
    case TRANSPARENT_PUT:
       win_op=-1;
       trcolor=imagegetpixel(bitmap,0,0);
       break;
    case TRANSPARENTBK_PUT:
       win_op=-1;
       trcolor=converttorgb(pWndData->bgColor);
       break;
    default:
      win_op=-2; 
    }
    if(width==dwidth && height==dheight && win_op>=0)
      BitBlt(hDC, left, top, width, height, hMemoryDC, 0, 0, win_op);
    else if(win_op>=0)
    {
      if(dwidth<width && dheight<height && win_op==SRCCOPY)
      { POINT pt;
        GetBrushOrgEx(hDC, &pt);
        SetStretchBltMode(hDC, HALFTONE);
        SetBrushOrgEx(hDC, pt.x, pt.y, NULL);
      }
      else
        SetStretchBltMode(hDC, COLORONCOLOR);
      StretchBlt(hDC, left, top, dwidth, dheight, hMemoryDC, 0, 0, width, height, win_op);
    }
    else if(win_op==-1)
    { TransparentBlt(hDC, left, top, dwidth, dheight, hMemoryDC, 0, 0, width, height, trcolor&0xFFFFFF);
    }
    RefreshWindow( NULL );
    
    // Delete resources
    BGI__ReleaseWinbgiDC( );
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
}

static LPPICTURE readipicture(const char* filename)
{
    // The only way that I have found to use OleLoadImage is to first put all
    // the picture information into a stream. Based on Serguei's implementation
    // and information from http://www.codeproject.com/bitmap/render.asp?print=true
    HANDLE hFile;        // Handle to the picture file
    DWORD dwSize;        // Size of that file
    HGLOBAL hGlobal;     // Handle for memory block
    LPVOID pvData;       // Pointer to first byte of that memory block
    BOOL bRead;          // Was file read okay?
    DWORD dwBytesRead;   // Number of bytes read from the file
    LPSTREAM pStr;       // Pointer to an IStream
    HRESULT hOK;         // Result of various OLE operations
    LPPICTURE pPicture;  // Picture read by OleLoadPicture

    // Open the file. Page 943 Win32 Superbible
    hFile = CreateFile(
        filename,        // Name of the jpg, gif, or bmp
        GENERIC_READ,    // Open for reading
        FILE_SHARE_READ, // Allow others to read, too
        NULL,            // Security attributes
        OPEN_EXISTING,   // The file must previously exist
        0,               // Attributes if creating new file
        NULL             // Attribute templates if creating new file
        );
    if (hFile == INVALID_HANDLE_VALUE) return NULL;

    // Get the file size and check that it is not empty.
    dwSize = GetFileSize(hFile, NULL);
    if (dwSize == (DWORD)-1)
    {
        CloseHandle(hFile);
        // AfxMessageBox("Photo file was empty."); -- needs MFC;
        return NULL;
    }

    // Allocate memory based on file size and lock it so it can't be moved.
    hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (hGlobal == NULL)
    {
        CloseHandle(hFile);
	showerrorbox("Insufficient memory to read image");
        return NULL;
    }

    // Lock memory so it can't be moved, then read the file into that memory.
    pvData = GlobalLock(hGlobal);
    if (pvData != NULL)
    {
        dwBytesRead = 0; // To force whole file to be read
        bRead = ReadFile(hFile, pvData, dwSize, &dwBytesRead, NULL);
    }
    GlobalUnlock(hGlobal);
    CloseHandle(hFile);
    if ((pvData == NULL) || !bRead || (dwBytesRead != dwSize))
    {
        GlobalFree(hGlobal);
        // AfxMessage("Could not read photo file."; -- needs MFC
        return NULL;
    }

    // At this point, the file is in the hGlobal memory block.
    // We will now connect an IStream* to this global memory.
    pStr = NULL; // In case CreateStreamOnHGlobal doesn't set it.
    hOK = CreateStreamOnHGlobal(hGlobal, TRUE, &pStr);
    if (pStr == NULL)
    {
        GlobalFree(hGlobal);
        // AfxMessage("Could not create IStream."; -- needs MFC
        return NULL;
    }
    if (FAILED(hOK))
    {
        GlobalFree(hGlobal);
        pStr->Release( );
        // AfxMessage("Could not create IStream."; -- needs MFC
        return NULL;
    }
    
    // Finally: Load the picture
    hOK = OleLoadPicture(pStr, dwSize, FALSE, IID_IPicture, (LPVOID *)&pPicture);
    pStr->Release( );
    if (!SUCCEEDED(hOK) || (pPicture == NULL))
    {
        GlobalFree(hGlobal);
        // AfxMessage("Could not create IStream."; -- needs MFC
        return NULL;
    }

    // pPicture is now a pointer to our picture.
    GlobalFree(hGlobal);
    return pPicture;
}

void readimagefile(
    const char* filename,
    int left, int top, int right, int bottom
    )
{
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    LPPICTURE pPicture = NULL;                     // Picture object for this image
    long full_width, full_height;                  // Dimensions of full image
    long width, height;                            // Dimensions of drawn image
    OPENFILENAME ofn;     // Struct for opening a file
    TCHAR fn[MAX_PATH+1]; // Space for storing the open file name
    // Get the filename, if needed
    if (filename == NULL)
    {
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ZeroMemory(&fn, MAX_PATH+1);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = _T("Image files (*.bmp, *.gif, *.jpg, *.ico, *.emf, *.wmf)\0*.BMP;*.GIF;*.JPG;*.ICO;*.EMF;*.WMF\0\0");
	ofn.lpstrFile = fn;
	ofn.nMaxFile = MAX_PATH+1;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST ;
	if (!GetOpenFileName(&ofn)) return;
    }

    if (filename == NULL)
	pPicture = readipicture(fn);
    else
	pPicture = readipicture(filename);
    if (pPicture)
    {
	pWndData = BGI__GetWindowDataPtr( );
	hDC = BGI__GetWinbgiDC( );
	width = 1 + abs(right - left);
	height = 1 + abs(bottom - top);
        pPicture->get_Width(&full_width);
        pPicture->get_Height(&full_height);
	pPicture->Render( hDC, left, top, width, height, 0, full_height, full_width, -full_height, NULL);
	BGI__ReleaseWinbgiDC( );
        pPicture->Release( );
	RefreshWindow( NULL );
    }
}    

void writeimagefile(
    const char* filename,
    int left, int top, int right, int bottom,
    bool active, HWND hwnd
    )
{
    long width, height;   // Width and height of the image in pixels
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    HDIB hDIB;            // Handle to equivalent device independent bitmap
    OPENFILENAME ofn;     // Struct for opening a file
    TCHAR fn[MAX_PATH+1]; // Space for storing the open file name
    // Get the filename, if needed
    if (filename == NULL)
    {
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ZeroMemory(&fn, MAX_PATH+1);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = _T("Bitmap files (*.bmp)\0*.BMP\0\0");
	ofn.lpstrFile = fn;
	ofn.nMaxFile = MAX_PATH+1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
	if (!GetSaveFileName(&ofn)) return;
	if (strlen(fn) < 4 || (fn[strlen(fn)-4] != '.' && strlen(fn) < MAX_PATH-4))
	    strcat(fn, ".BMP");
    }

    // Preliminary computations
    pWndData = BGI__GetWindowDataPtr(hwnd);
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    if (active)
	hDC = pWndData->hDC[pWndData->ActivePage];
    else
	hDC = pWndData->hDC[pWndData->VisualPage];
    if (left < 0) left = 0;
    else if (left >= pWndData->width) left = pWndData->width - 1;
    if (right < 0) right = 0;
    else if (right >= pWndData->width) right = pWndData->width;
    if (bottom < 0) bottom = 0;
    else if (bottom >= pWndData->height) bottom = pWndData->height;
    if (top < 0) top = 0;
    else if (top >= pWndData->height) top = pWndData->height;
    width = 1 + abs(right - left);
    height = 1 + abs(bottom - top);

    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);

    hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

    // Grab the bitmap data from hDC and put it in hMemoryDC
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hDC, left, top, SRCCOPY);

    // Get the equivalent DIB and write it to the file
    hDIB = BitmapToDIB(hBitmap, NULL);
    if (filename == NULL)
	SaveDIB(hDIB, fn);
    else
	SaveDIB(hDIB, filename);
    
    // Delete resources
    ReleaseMutex(pWndData->hDCMutex);
    DestroyDIB(hDIB);
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
}

void printimage(
    const char* title,
    double width_inches, double border_left_inches, double border_top_inches,
    int left, int top, int right, int bottom, bool active, HWND hwnd
    )
{
    static PRINTDLG pd_Printer; 
    WindowData* pWndData; // Our own window data struct for visual window
    long width, height;   // Width and height of the image in pixels
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HDC hDC;              // Device context for the visual window to print
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    int titlelen;         // Length of the title
    int pixels_per_inch_x, pixels_per_inch_y;
    double factor_x, factor_y;
    DOCINFO di;

    // Set pd_Printer.hDC to a handle to the printer dc using a print dialog,
    // as shown on page 950-957 of Win32 Programming.  Note that the setup
    // is done the first time this function is called (since pd_Printer will
    // be all zeros) or if both hDevNames and hDevMode are later NULL, as
    // shown in Listing 13.3 on page 957.
    if (pd_Printer.hDevNames == NULL && pd_Printer.hDevMode == NULL)
    {
        memset(&pd_Printer, 0, sizeof(PRINTDLG));
        pd_Printer.lStructSize = sizeof(PRINTDLG);
        pd_Printer.Flags = PD_RETURNDEFAULT;
        // Get the default printer:
        if (!PrintDlg(&pd_Printer))
            return; // Failure
        // Set things up so next call to PrintDlg won't go back to default.
        pd_Printer.Flags &= ~PD_RETURNDEFAULT;
    }
    // Cause PrintDlg to return a DC in hDC; could set other flags here, too.
    pd_Printer.Flags |= PD_RETURNDC;
    if (!PrintDlg(&pd_Printer))
        return; // Failure or canceled

    // Get the window's hDC, width and height
    pWndData = BGI__GetWindowDataPtr(hwnd);
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    if (active)
	hDC = pWndData->hDC[pWndData->ActivePage];
    else
	hDC = pWndData->hDC[pWndData->VisualPage];
    if (left < 0) left = 0;
    else if (left >= pWndData->width) left = pWndData->width - 1;
    if (right < 0) right = 0;
    else if (right >= pWndData->width) right = pWndData->width;
    if (bottom < 0) bottom = 0;
    else if (bottom >= pWndData->height) bottom = pWndData->height;
    if (top < 0) top = 0;
    else if (top >= pWndData->height) top = pWndData->height;
    width = 1 + abs(right - left);
    height = 1 + abs(bottom - top);

    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);
    
    // Copy the bitmap data from hDC and put it in hMemoryDC for printing
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hDC, left, top, SRCCOPY);

    // Determine the size factors for blowing up the photo.
    pixels_per_inch_x = GetDeviceCaps(pd_Printer.hDC, LOGPIXELSX);
    pixels_per_inch_y = GetDeviceCaps(pd_Printer.hDC, LOGPIXELSY);
    factor_x = pixels_per_inch_x * width_inches / width;
    factor_y = factor_x * pixels_per_inch_y / pixels_per_inch_x;

    // Set up a DOCINFO structure.
    memset(&di, 0, sizeof(DOCINFO));
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = "Windows BGI";

    // StartDoc, print stuff, EndDoc
    if (StartDoc(pd_Printer.hDC, &di) != SP_ERROR)
    {   
        StartPage(pd_Printer.hDC);
	if (title == NULL) title = pWndData->title.c_str( );
	titlelen = strlen(title);
	if (titlelen > 0)
	{
	    TextOut(pd_Printer.hDC, int(pixels_per_inch_x*border_left_inches), int(pixels_per_inch_y*border_top_inches), title, titlelen);
	    border_top_inches += 0.25;
	}
        if (GetDeviceCaps(pd_Printer.hDC, RASTERCAPS) & RC_BITBLT)
        {
            StretchBlt(
                pd_Printer.hDC, int(pixels_per_inch_x*border_left_inches), int(pixels_per_inch_y*border_top_inches),
                int(width*factor_x), 
                int(height*factor_y), 
                hMemoryDC, 0, 0, width, height,
                SRCCOPY
                );
        }
        EndPage(pd_Printer.hDC);

        EndDoc(pd_Printer.hDC);
    }

    // Delete the resources
    ReleaseMutex(pWndData->hDCMutex);
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
}
#define HIMETRIC_INCH   2540    // HIMETRIC units per inch

IMAGE *loadBMP(const char *filename)
{
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    LPPICTURE pPicture = NULL;                     // Picture object for this image
	long width, height;                  // Dimensions of full image
	long full_width, full_height;                  // Dimensions of full image
	pPicture = readipicture(filename);
    if (pPicture)
    {
	pWndData = BGI__GetWindowDataPtr( );
	hDC = BGI__GetWinbgiDC( );
        pPicture->get_Width(&full_width);
        pPicture->get_Height(&full_height);
		int nPixX = GetDeviceCaps(hDC,LOGPIXELSX);
		int nPixY = GetDeviceCaps(hDC,LOGPIXELSY);

		width = MulDiv(full_width, nPixX, HIMETRIC_INCH);
		height = MulDiv(full_height, nPixY, HIMETRIC_INCH);

        BITMAP *bitmap=(BITMAP*)createimage(width,height);
        hMemoryDC = CreateCompatibleDC(hDC);
        hBitmap = CreateCompatibleBitmap(hDC, width, height);
        hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

        // Grab the bitmap data from hDC and put it in hMemoryDC
        SelectObject(hMemoryDC, hBitmap);
        
		pPicture->Render(hMemoryDC, 0,0, width, height, 0, full_height, full_width, -full_height, NULL);
        BITMAP* pUser;        // A pointer into the user's buffer, used as a BITMAP
        pUser = (BITMAP*) bitmap;
        GetObject(hBitmap, sizeof(BITMAP), pUser);
        pUser->bmBits = (BYTE*) bitmap + sizeof(BITMAP);
        GetBitmapBits(hBitmap, pUser->bmHeight*pUser->bmWidthBytes, pUser->bmBits);

        // Delete resources
        SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
        DeleteObject(hBitmap);               // Delete the bitmap we used
        DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
	BGI__ReleaseWinbgiDC( );
        pPicture->Release( );
		return bitmap;
    }
    else
      return NULL;
}
void saveBMP(const char *filename, IMAGE *bitmap)
{
    long width, height;   // Width and height of the image in pixels
    WindowData* pWndData; // Our own window data struct for active window
    HDC hDC;              // Device context for the active window
    HDC hMemoryDC;        // Memory device context for a copy of the image
    HBITMAP hOldBitmap;   // Handle to original bitmap of hMemDC
    HBITMAP hBitmap;      // Handle to bitmap that will be selected into hMemDC
    HDIB hDIB;            // Handle to equivalent device independent bitmap

    // Preliminary computations
    pWndData = BGI__GetWindowDataPtr();
    WaitForSingleObject(pWndData->hDCMutex, 5000);
    hDC = BGI__GetWinbgiDC( );
    BITMAP* pUser;        // A pointer into the user's buffer, used as a BITMAP

    // Preliminary computations
    pUser = (BITMAP*) bitmap;
    width = pUser->bmWidth;
    height = pUser->bmHeight;

    // Create the memory DC and select a new larger bitmap for it, saving the
    // original bitmap to restore later (before deleting).
    hMemoryDC = CreateCompatibleDC(hDC);
    hBitmap = CreateCompatibleBitmap(hDC, width, height);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    // Grab the bitmap data from the user's bitmap and put it in hMemoryDC
    SetBitmapBits(hBitmap, pUser->bmHeight*pUser->bmWidthBytes, pUser->bmBits);

    // Get the equivalent DIB and write it to the file
    hDIB = BitmapToDIB(hBitmap, NULL);
    SaveDIB(hDIB, filename);

    
    // Delete resources
    ReleaseMutex(pWndData->hDCMutex);
    DestroyDIB(hDIB);
    SelectObject(hMemoryDC, hOldBitmap); // Restore original bmp so it's deleted
    DeleteObject(hBitmap);               // Delete the bitmap we used
    DeleteDC(hMemoryDC);                 // Delete the memory dc and it's bmp
    BGI__ReleaseWinbgiDC( );
}
