
// XaserImageDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "afxcmn.h"

// CXImageDlg dialog
class CXImageDlg : public CDialogEx
{
// Construction
    public:
        CXImageDlg ( CWnd* pParent = NULL );	// standard constructor
        double GetValue ( CEdit &edit ) {
            CString tmp;
            edit.GetWindowText ( tmp );
            return  _wtoi ( tmp );
        }

// Dialog Data
#ifdef AFX_DESIGN_TIME
        enum { IDD = IDD_XASERIMAGE_DIALOG };
#endif

    protected:
        virtual void DoDataExchange ( CDataExchange* pDX );	// DDX/DDV support


// Implementation
    protected:
        HICON m_hIcon;
        CImage m_CImg;
        unsigned int y_pixel_size, x_pixel_size;
        std::vector<CString>gcode;
        CString m_ImageFilename;
        // Generated message map functions
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand ( UINT nID, LPARAM lParam );
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        DECLARE_MESSAGE_MAP()
    public:
        afx_msg void OnBnClickedLoadImage();
        CStatic m_Image;
        afx_msg void OnBnClickedConvert();
        afx_msg void OnBnClickedSave();
        CProgressCtrl m_Progress;
        CButton m_SZ;
        CButton m_Invert;
        CSliderCtrl m_Scale;
        afx_msg void OnNMReleasedcaptureScale ( NMHDR *pNMHDR, LRESULT *pResult );
        CEdit m_ScaleValue;

        CEdit m_Upper;
        CEdit m_Lower;
        afx_msg void OnNMCustomdrawScale ( NMHDR *pNMHDR, LRESULT *pResult );
        CButton m_ConvertButton;
        CButton m_SaveButton;
        afx_msg void OnDestroy();
};
