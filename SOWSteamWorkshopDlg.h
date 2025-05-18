//*****************************************************************************
// SOWSteamWorkshopDlg.h : header file

#pragma once

//*****************************************************************************

struct STag
{
	UINT val;
	CString name;
	UINT id;
};
extern STag gTags[];

//*****************************************************************************

struct SModData
{
	uint64_t modId;
	int universe;
	UINT tags;
};

class WorkshopUploader;

//*****************************************************************************

class CSOWSteamWorkshopDlg : public CDialogEx
{
// Construction
public:
	CSOWSteamWorkshopDlg(CWnd* pParent = nullptr);	// standard constructor

	void log(LPCTSTR msg);

	uint64_t Submit(uint64_t modid);

	SModData m_data;

	WorkshopUploader* m_loader;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOWSTEAMWORKSHOP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	int m_sel;

	CMapStringToString m_desc;
	CMapStringToString m_name;

	void Reset();
	void LoadMod();

// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bValid;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnValidate();
	afx_msg void OnClickedPathdlg();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeLanguage();
	CComboBox m_lang;
	CListBox m_log;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
extern CSOWSteamWorkshopDlg* theDlg;

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
