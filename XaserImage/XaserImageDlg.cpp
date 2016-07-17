
// XaserImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XaserImage.h"
#include "XaserImageDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Called at start of GCODE
const TCHAR START_MACRO[] =		_T ( "G21\nG0X0Y0S0F100\t;Reset Position" );
const TCHAR END_MACRO[] =		_T ( "G0X0Y0\t\t;Reset to Home\nG0\nG0\nG0\nG0" );

bool SetReg ( CString Section, CString Name, DWORD dwVal )
{
    HKEY hKey;
    DWORD dwDisp = 0;
    LPDWORD lpdwDisp = &dwDisp;

    LONG iSuccess = RegCreateKeyEx ( HKEY_CURRENT_USER, Section, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, lpdwDisp );

    if ( iSuccess == ERROR_SUCCESS ) {
        RegSetValueEx ( hKey, Name, 0L, REG_DWORD, ( CONST BYTE* ) &dwVal, sizeof ( DWORD ) );

        return true;
    }

    return false;
}

bool SetReg ( CString Section, CString Name, CString &in_Val )
{
    HKEY hKey;
    DWORD dwDisp = 0;
    LPDWORD lpdwDisp = &dwDisp;

    CStringA Val ( in_Val );

    LONG iSuccess = RegCreateKeyEx ( HKEY_CURRENT_USER, Section, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, lpdwDisp );

    if ( iSuccess == ERROR_SUCCESS ) {

        int length;
        char *buffer;

        length = Val.GetLength();

        buffer = Val.GetBuffer ( length );

        ASSERT ( buffer );

        RegSetValueEx ( hKey, Name, 0, REG_SZ, ( LPBYTE ) buffer, length + 1 );

        Val.ReleaseBuffer();

        return true;
    }

    return false;
}

DWORD GetReg ( CString Section, CString Name )
{
    DWORD dwVersion = 0;
    HKEY hKey;
    LONG returnStatus;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof ( DWORD );

    returnStatus = RegOpenKeyEx ( HKEY_CURRENT_USER, Section, 0L, KEY_ALL_ACCESS, &hKey );

    if ( returnStatus == ERROR_SUCCESS ) {
        returnStatus = RegQueryValueEx ( hKey, Name, NULL, &dwType, ( LPBYTE ) &dwVersion, &dwSize );
    }

    RegCloseKey ( hKey );


    return dwVersion;
}

CString GetRegStr ( CString Section, CString Name )
{
    TCHAR buffer[256];
    HKEY hKey;
    LONG returnStatus;
    DWORD dwType = REG_SZ;
    DWORD dwSize = sizeof ( buffer ) - 1;

    memset ( &buffer[0], 0, dwSize * sizeof ( TCHAR ) );

    returnStatus = RegOpenKeyEx ( HKEY_CURRENT_USER, Section, 0L, KEY_ALL_ACCESS, &hKey );

    if ( returnStatus == ERROR_SUCCESS ) {
        returnStatus = RegQueryValueEx ( hKey, Name, NULL, &dwType, ( LPBYTE ) &buffer, &dwSize );

        if ( ERROR_MORE_DATA == returnStatus ) {
            returnStatus = RegQueryValueEx ( hKey, Name, NULL, &dwType, ( LPBYTE ) &buffer, &dwSize );
        }
    }

    RegCloseKey ( hKey );

    return buffer;

}
// scale a value within an upper/lower range
// value = value to scale
// lower/upper = range to scale too
// min/max = range of original data
double ranged_scale ( double value, double lower, double upper, double min, double max )
{
    // scale to 0..1, set to upper range, add lower range
    value = ( ( value - min ) / ( max - min ) * ( upper - lower ) ) + lower;

    return value;
}

void rgb2hsv ( COLORREF rgb, hsv_t* hsv )
{
    ASSERT ( hsv );

    int r = GetRValue ( rgb );
    int g = GetGValue ( rgb );
    int b = GetBValue ( rgb );
    int m = MIN3 ( r, g, b );
    int M = MAX3 ( r, g, b );
    int delta = M - m;

    if ( delta == 0 ) {
        /* Achromatic case (i.e. grey scale) */
        hsv->hue = -1;          /* undefined */
        hsv->saturation = 0;

    } else {
        int h;

        if ( r == M )
        { h = ( ( g - b ) * 60 * HUE_DEGREE ) / delta; }

        else
            if ( g == M )
            { h = ( ( b - r ) * 60 * HUE_DEGREE ) / delta + 120 * HUE_DEGREE; }

            else /*if(b == M)*/
            { h = ( ( r - g ) * 60 * HUE_DEGREE ) / delta + 240 * HUE_DEGREE; }

        if ( h < 0 )
        { h += 360 * HUE_DEGREE; }

        hsv->hue = h;

        /* The constant 8 is tuned to statistically cause as little
        * tolerated mismatches as possible in RGB -> HSV -> RGB conversion.
        * (See the unit test at the bottom of this file.)
        */
        hsv->saturation = ( 256 * delta - 8 ) / M;
    }

    hsv->value = M;
}

CString GetLoadFile ( const TCHAR *ptypes, const TCHAR*caption, const TCHAR *pStartDir )
{
    CString szFile;
    TCHAR tmpFile[MAX_PATH];

    OPENFILENAME ofn;

    szFile.Empty();
    memset ( tmpFile, 0, MAX_PATH * sizeof ( TCHAR ) );

    if ( pStartDir ) {
        _tcscpy_s ( tmpFile, MAX_PATH, pStartDir );
    }

    ZeroMemory ( &ofn, sizeof ( OPENFILENAME ) );
    ofn.lStructSize = sizeof ( OPENFILENAME );

    ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();

    ofn.lpstrFilter = ptypes;
    ofn.lpstrInitialDir = pStartDir;

    ofn.lpstrTitle = caption;

    ofn.lpstrFile = &tmpFile[0];
    ofn.nMaxFile = MAX_PATH;

    if ( IDOK != GetOpenFileName ( &ofn ) ) {

        return _T ( "" );
    }

    szFile = tmpFile;

    return szFile;
}

CString GetSaveFile ( const TCHAR *ptypes, const TCHAR*caption, const TCHAR *pStartDir )
{
    TCHAR szFile[MAX_PATH];

    OPENFILENAME ofn;
    ZeroMemory ( szFile, MAX_PATH * sizeof ( TCHAR ) );

    if ( pStartDir ) {
        _tcscpy_s ( szFile, _tcsclen ( pStartDir ) - 1, pStartDir );
    }

    ZeroMemory ( &ofn, sizeof ( OPENFILENAME ) );
    ofn.lStructSize = sizeof ( OPENFILENAME );

    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();

    ofn.lpstrFilter = ptypes;
    ofn.lpstrInitialDir = pStartDir;

    ofn.lpstrTitle = caption;

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;

    if ( IDOK != GetSaveFileName ( &ofn ) ) {

        return _T ( "" );
    }

    return szFile;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
    public:
        CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
        enum { IDD = IDD_ABOUTBOX };
#endif

    protected:
        virtual void DoDataExchange ( CDataExchange* pDX ); // DDX/DDV support

// Implementation
    protected:
        DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx ( IDD_ABOUTBOX )
{
}

void CAboutDlg::DoDataExchange ( CDataExchange* pDX )
{
    CDialogEx::DoDataExchange ( pDX );
}

BEGIN_MESSAGE_MAP ( CAboutDlg, CDialogEx )
END_MESSAGE_MAP()


// CXImageDlg dialog



CXImageDlg::CXImageDlg ( CWnd* pParent /*=NULL*/ )
    : CDialogEx ( IDD_XASERIMAGE_DIALOG, pParent ),
      y_pixel_size ( 1 ), x_pixel_size ( 1 )
{
    m_hIcon = AfxGetApp()->LoadIcon ( IDR_MAINFRAME );
}

void CXImageDlg::DoDataExchange ( CDataExchange* pDX )
{
    CDialogEx::DoDataExchange ( pDX );
    DDX_Control ( pDX, IDC_IMAGE, m_Image );
    DDX_Control ( pDX, IDC_PROGRESS_BAR, m_Progress );
    DDX_Control ( pDX, IDC_ZORS, m_SZ );
    DDX_Control ( pDX, IDC_INVERT, m_Invert );
    DDX_Control ( pDX, IDC_SCALE, m_Scale );
    DDX_Control ( pDX, IDC_SCALE_VALUE, m_ScaleValue );
    DDX_Control ( pDX, IDC_LOWER_VAL, m_Upper );
    DDX_Control ( pDX, IDC_UPPER_VAL, m_Lower );
    DDX_Control ( pDX, IDC_CONVERT, m_ConvertButton );
    DDX_Control ( pDX, IDC_SAVE, m_SaveButton );
    DDX_Control ( pDX, IDC_PIXEL_X_SIZE, m_Pixel_X_Size );
    DDX_Control ( pDX, IDC_PIXEL_Y_SIZE, m_Pixel_Y_Size );
}

BEGIN_MESSAGE_MAP ( CXImageDlg, CDialogEx )
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED ( IDC_LOAD_IMAGE, &CXImageDlg::OnBnClickedLoadImage )
    ON_BN_CLICKED ( IDC_CONVERT, &CXImageDlg::OnBnClickedConvert )
    ON_BN_CLICKED ( IDC_SAVE, &CXImageDlg::OnBnClickedSave )
    ON_NOTIFY ( NM_RELEASEDCAPTURE, IDC_SCALE, &CXImageDlg::OnNMReleasedcaptureScale )
    ON_NOTIFY ( NM_CUSTOMDRAW, IDC_SCALE, &CXImageDlg::OnNMCustomdrawScale )
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CXImageDlg message handlers

BOOL CXImageDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CString tmp;

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT ( ( IDM_ABOUTBOX & 0xFFF0 ) == IDM_ABOUTBOX );
    ASSERT ( IDM_ABOUTBOX < 0xF000 );

    CMenu* pSysMenu = GetSystemMenu ( FALSE );

    if ( pSysMenu != NULL ) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString ( IDS_ABOUTBOX );
        ASSERT ( bNameValid );

        if ( !strAboutMenu.IsEmpty() ) {
            pSysMenu->AppendMenu ( MF_SEPARATOR );
            pSysMenu->AppendMenu ( MF_STRING, IDM_ABOUTBOX, strAboutMenu );
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon ( m_hIcon, TRUE );			// Set big icon
    SetIcon ( m_hIcon, FALSE );		// Set small icon


    // the idea of this is to be equivalent to the size of one laser dot
    y_pixel_size = ( ( double ) GetReg ( _T ( "XaserImage" ), _T ( "yPixel" ) ) ) / 1000.0;
    x_pixel_size = ( ( double ) GetReg ( _T ( "XaserImage" ), _T ( "xPixel" ) ) ) / 1000.0;

    if ( x_pixel_size <= 0 ) {
        x_pixel_size = 1;

    }

    tmp.Format ( _T ( "%g" ), x_pixel_size );
    m_Pixel_X_Size.SetWindowText ( tmp );

    if ( y_pixel_size <= 0 ) {
        y_pixel_size = 1;

    }

    tmp.Format ( _T ( "%g" ), y_pixel_size );
    m_Pixel_Y_Size.SetWindowText ( tmp );

    // power range 0-100
    m_Scale.SetRange ( 1, 100 );

    // scale image and power
    int value = GetReg ( _T ( "XaserImage" ), _T ( "Scale" ) );

    // clamp it
    value = MAX ( value, 1 );
    value = MIN ( value, 100 );

    m_Scale.SetPos ( value );
    tmp.Format ( _T ( "%d" ), value );

    m_ScaleValue.SetWindowText ( tmp );

    value = GetReg ( _T ( "XaserImage" ), _T ( "Lower" ) );

    // clamp range
    if ( value == 0 ) {
        value = 1;
    }

    tmp.Format ( _T ( "%d" ), value );

    m_Lower.SetWindowText ( tmp );

    value = GetReg ( _T ( "XaserImage" ), _T ( "Upper" ) );

    // clamp upper range
    if ( value == 0 ) {
        value = 100;
    }

    tmp.Format ( _T ( "%d" ), value );
    m_Upper.SetWindowText ( tmp );

    m_SaveButton.ShowWindow ( FALSE );
    m_ConvertButton.ShowWindow ( FALSE );

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXImageDlg::OnSysCommand ( UINT nID, LPARAM lParam )
{
    if ( ( nID & 0xFFF0 ) == IDM_ABOUTBOX ) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();

    } else {
        CDialogEx::OnSysCommand ( nID, lParam );
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXImageDlg::OnPaint()
{
    if ( IsIconic() ) {
        CPaintDC dc ( this ); // device context for painting

        SendMessage ( WM_ICONERASEBKGND, reinterpret_cast<WPARAM> ( dc.GetSafeHdc() ), 0 );

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics ( SM_CXICON );
        int cyIcon = GetSystemMetrics ( SM_CYICON );
        CRect rect;
        GetClientRect ( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;

        // Draw the icon
        dc.DrawIcon ( x, y, m_hIcon );

    } else {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXImageDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR> ( m_hIcon );
}

void CXImageDlg::OnBnClickedLoadImage()
{
    m_ImageFilename = GetLoadFile (
                          _T ( "Supported Files Types(*.png,*.gif,*.bmp,*.jpg)\0*.bmp;*.jpg;*.png;*.gif\0\0" ),
                          _T ( "Choose image to load " ),
                          // start dir
                          _T ( "" )
                      );

    m_CImg.Destroy();
    m_CImg.ReleaseGDIPlus();

    m_CImg.Load ( m_ImageFilename );

    m_Image.SetBitmap ( NULL );

    if ( m_Image.GetBitmap() == NULL ) {
        m_Image.SetBitmap ( HBITMAP ( m_CImg ) );
    }

    m_Image.ShowWindow ( SW_SHOW );
    InvalidateRect ( NULL );

    m_ConvertButton.ShowWindow ( TRUE );
}

/*
 * convert image to grey scale
 */
void CXImageDlg::OnBnClickedConvert()
{
    double width, height, x, y , end_x ;
    COLORREF rgb;
    hsv_t hsv;
    CString output;
    double power, last_power = -1;

    /// no image loaded
    if ( m_Image.GetBitmap() == NULL ) {

        m_ConvertButton.ShowWindow ( FALSE );

        return;
    }

    // empty gcode list
    gcode.clear();

    // starting code
    gcode.push_back ( START_MACRO );

    width = m_CImg.GetWidth();
    height = m_CImg.GetHeight();

    double upper_value = GetValue ( m_Upper );
    double lower_value = GetValue ( m_Lower );

    x_pixel_size =  GetValue ( m_Pixel_X_Size );
    y_pixel_size = GetValue ( m_Pixel_Y_Size );

    if ( x_pixel_size <= 0 ) {
        x_pixel_size = 1;
    }

    if ( y_pixel_size <= 0 ) {
        y_pixel_size = 1;
    }

    // progress meter
    m_Progress.SetRange32 ( 0, ( int ) ( height ) - 1 );

    double scale;
    scale = m_Scale.GetPos();

    if ( scale == 0 ) {
        scale = 1;
    }

    double output_x, output_y;


    output_y = 0;

    for ( y = 0; y < height; y ++ ) {

        // reset vars from x loop to beginning of line
        end_x = 0;
        last_power = -1;

        // laser off and move to Y position

        output.Format ( _T ( "G0Y%g" ), ( output_y / scale ) );
        gcode.push_back ( output );

        // move down by laser kerf size
        output_y += y_pixel_size;

        //reset to beginning of line
        output_x = 0;

        for ( x = 0; x < width; x ++ ) {

            power = 0;

            // get RGB value, x,y is relative to image size
            rgb = m_CImg.GetPixel ( ( int ) x, ( int ) y );

            // convert to HSV
            rgb2hsv ( rgb, &hsv );

            // if we're not inverting the V , default is inverted
            if ( m_Invert.GetState() == FALSE ) {
                hsv.value = ( 255 - hsv.value );
            }

            // linear scale to lower/upper range from 0.255
            hsv.value = ( BYTE ) ranged_scale ( hsv.value, lower_value, upper_value, 0, 255 );

            // not 0 ?
            if ( hsv.value ) {

                // scale to 0..1
                power = ( hsv.value  ) /  100.0;

                // round to 2 decimal places (@todo: move to GUI)
                power = round ( power * 100.0 ) / 100.0;
            }

            // convert to grey RGB
            rgb = RGB ( hsv.value, hsv.value, hsv.value );

            // write it back to loaded image
            m_CImg.SetPixel ( ( int ) x, ( int ) y, rgb );

            // if different, write out, unless its the first in a horizontal line

            // first time
            if ( last_power == -1 ) {

                // move x along.
                end_x = output_x;

                // remember power
                last_power = power;

            } else

                // power level changed ?
                if ( power != last_power ) {

                    // output X and power.
                    // move X to end_x with S level power

                    // decide to write as Z or S command, Z lets you see it in all the GCODE simulators
                    if ( m_SZ.GetState() == FALSE ) {
                        output.Format ( _T ( "G1X%gS%0.03g" ),  ( output_x / scale ), last_power );

                    } else {
                        output.Format ( _T ( "G1X%gZ%0.03g" ), ( output_x / scale ), last_power );
                    }

                    // save it to the gcode output list
                    gcode.push_back ( output );

                    // save the power level
                    last_power = power;

                } else {

                    // not using at the moment
                    end_x = output_x;
                }

            // if power = last power, don't write out a start point.
            // feed rate
            // min/max power
            // burn size.
            /*

            	G1S0F(FEED)
            		repeat for height
            			for width
            				step = ( count number of equal ) * size of pixel
            				output G1X(STEP)S(VALUE)


            */

            // move gcode along
            output_x += x_pixel_size;
        }


        // have we written out to the end of the line?, if not add it.
        if ( power ) {
            output.Format ( _T ( "G1X%gS%0.03g" ), ( double ) ( output_x / scale ), power  );
            gcode.push_back ( output );
        }

        // move laser head back to start
        output.Format ( _T ( "G0X0F5500" ) );
        gcode.push_back ( output );

        // update progress
        if ( ( ( int ) y % 10 ) == 0 ) {
            m_Progress.SetPos ( ( int ) y );
        }
    }

    // move to X0Y0
    output.Format ( END_MACRO );
    gcode.push_back ( output );

    // redraw image since we changed it
    InvalidateRect ( NULL );

    m_ConvertButton.ShowWindow ( FALSE );
    m_SaveButton.ShowWindow ( TRUE );
}

void CXImageDlg::OnBnClickedSave()
{
    // no bitmap loaded
    if ( m_Image.GetBitmap() == NULL ) {
        m_SaveButton.ShowWindow ( FALSE );
        m_ConvertButton.ShowWindow ( FALSE );
        return;
    }

    // nothing converted
    if ( gcode.empty() == true ) {
        m_ConvertButton.ShowWindow ( TRUE );
        m_SaveButton.ShowWindow ( FALSE );
        return;
    }

    // reset progress bar
    m_Progress.SetRange32 ( 0, ( int ) gcode.size() - 1 );
    m_Progress.SetPos ( 0 );

    CString filename;
    // get the output filename
    filename = GetSaveFile ( _T ( "Supported Files Types ( *.gcode )\0*.gcode\0\0" ), _T ( "Choose file to save as" ), _T ( "\0" ) );

    if ( filename.IsEmpty() == true ) {
        return;
    }

    std::wofstream ofs;

    ofs.open ( filename );

    if ( !ofs.is_open() ) {
        AfxMessageBox ( _T ( "File failed to save" ), MB_OK );
        return;
    }

    for ( auto &it : gcode ) {

        // write out GCODE
        ofs << ( LPCTSTR ) ( it ) << "\n";

        m_Progress.SetPos ( m_Progress.GetPos() + 1 );
    }

    ofs.close();

    m_SaveButton.ShowWindow ( FALSE );
}


void CXImageDlg::OnNMReleasedcaptureScale ( NMHDR *pNMHDR, LRESULT *pResult )
{
    CString str;
    str.Format ( _T ( "%d" ), m_Scale.GetPos() );
    m_ScaleValue.SetWindowText ( str );

    *pResult = 0;
}

void CXImageDlg::OnNMCustomdrawScale ( NMHDR *pNMHDR, LRESULT *pResult )
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW> ( pNMHDR );
    *pResult = 0;

    CString str;
    str.Format ( _T ( "%d" ), m_Scale.GetPos() );
    m_ScaleValue.SetWindowText ( str );
}

void CXImageDlg::OnDestroy()
{
    // send values to registry

    SetReg ( _T ( "XaserImage" ), _T ( "Scale" ), m_Scale.GetPos() );
    SetReg ( _T ( "XaserImage" ), _T ( "Upper" ), ( DWORD ) GetValue ( m_Upper ) );
    SetReg ( _T ( "XaserImage" ), _T ( "Lower" ), ( DWORD ) GetValue ( m_Lower ) );

    // set these as text instead.
    SetReg ( _T ( "XaserImage" ), _T ( "xPixel" ), ( DWORD ) ( GetValue ( m_Pixel_X_Size ) * 1000.0 ) );
    SetReg ( _T ( "XaserImage" ), _T ( "yPixel" ), ( DWORD ) ( GetValue ( m_Pixel_Y_Size ) * 1000.0 ) );

    CDialogEx::OnDestroy();

}
