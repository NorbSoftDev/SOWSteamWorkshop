//*****************************************************************************
// SOWSteamWorkshopDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "SOWSteamWorkshop.h"
#include "SOWSteamWorkshopDlg.h"
#include "afxdialogex.h"
#include "uploader.h"

#include "../Steam/public/steam/isteamugc.h"
#include "../Steam/public/steam/steam_api.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

STag gTags[] =
{
	{ 0x0001, L"Campaign", IDC_CAMP },
	{ 0x0002, L"Gameplay", IDC_GAME },
	{ 0x0004, L"Music Sound", IDC_MUSIC },
	{ 0x0008, L"Uniform Sprites", IDC_UNIFORM },
	{ 0x0010, L"Map", IDC_MAP },
	{ 0x0020, L"Toolbar", IDC_TOOLBAR },
	{ 0x0040, L"Order Of Battle", IDC_OOB },
	{ 0x0080, L"Terrain Sprites", IDC_TERRAIN },
	{ 0x0100, L"User Interface", IDC_UI },
	{ 0x0200, L"AI Modules", IDC_AI },
	{ 0x0400, L"Scenarios", IDC_SCEN },
	{ 0x0800, L"Effects", IDC_EFFECTS },
	{ 0, L"", 0 }
};

#define INIFILE "\\steam_sow_mod.ini"

//*****************************************************************************

CSOWSteamWorkshopDlg* theDlg = NULL;

//*****************************************************************************

CSOWSteamWorkshopDlg::CSOWSteamWorkshopDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOWSTEAMWORKSHOP_DIALOG, pParent)
	, m_bValid(0), m_sel(0)
{
	theDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_data.modId = 0;
	m_data.tags = 0;
	m_data.universe = 0;

	m_loader = new WorkshopUploader;
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LANGUAGE, m_lang);
	DDX_Control(pDX, IDC_MSGS, m_log);
}

//*****************************************************************************

BEGIN_MESSAGE_MAP(CSOWSteamWorkshopDlg, CDialogEx)
	ON_BN_CLICKED(IDYES, &CSOWSteamWorkshopDlg::OnValidate)
	ON_BN_CLICKED(IDC_PATHDLG, &CSOWSteamWorkshopDlg::OnClickedPathdlg)
	ON_CBN_SELCHANGE(IDC_LANGUAGE, &CSOWSteamWorkshopDlg::OnCbnSelchangeLanguage)
	ON_BN_CLICKED(IDOK, &CSOWSteamWorkshopDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSOWSteamWorkshopDlg::OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//*****************************************************************************

BOOL CSOWSteamWorkshopDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

#ifdef SOWGB
	SetWindowText(L"Scourge Of War - Gettysburg - Steam Mod Uploader");
#endif

	m_lang.InsertString(0, L"English");
	m_lang.InsertString(1, L"Italian");
	m_lang.InsertString(2, L"German");
	m_lang.InsertString(3, L"Spanish");
	m_lang.InsertString(4, L"French");
	m_lang.InsertString(5, L"Chinese");

	m_sel = 0;

	m_lang.SetCurSel(m_sel);

	SteamAPI_RestartAppIfNecessary(STEAM_APP_ID);

	if (!SteamAPI_Init() || !SteamUGC())
	{
		MessageBox(L"Error initializing Steam Libraries");
#ifdef SOWGB
		MessageBox(L"You may need to rename gb_appid.txt to: steam_appid.txt");
#else
		MessageBox(L"You may need to rename wl_appid.txt to: steam_appid.txt");
#endif
		MessageBox(L"It's safe to copy this folder to another location for use of use.");
		MessageBox(L"It's in the same folder as this exe");
		EndDialog(IDCANCEL);
		return FALSE;
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetTimer(0, 100, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != 0)
		return;

	SteamAPI_RunCallbacks();

	CDialogEx::OnTimer(nIDEvent);
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::log(LPCTSTR msg)
{
	int i = m_log.AddString(msg);

	m_log.SetTopIndex(i);
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnCbnSelchangeLanguage()
{
	int i = m_lang.GetCurSel();

	CString name;
	CString desc;
	CString lang;

	m_lang.GetLBText(m_sel, lang);

	GetDlgItemText(IDC_NAME, name);
	GetDlgItemText(IDC_DESC, desc);

	m_name.SetAt(lang, name);
	m_desc.SetAt(lang, desc);

	m_lang.GetLBText(i, lang);

	name = "";
	desc = "";

	m_name.Lookup(lang, name);
	SetDlgItemText(IDC_NAME, name);

	m_desc.Lookup(lang, desc);
	SetDlgItemText(IDC_DESC, desc);

	m_sel = i;
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::LoadMod()
{
	CString sPath;
	CString iniPath;
	CFileStatus fstat;

	UpdateData();

	GetDlgItemText(IDC_PATH, sPath);

	if (sPath.IsEmpty())
		return;

	iniPath = sPath;
	iniPath += INIFILE;

	if (!CFile::GetStatus(iniPath, fstat))
		return;

	TCHAR buff[8000];
	CString key;
	CString value;

	m_lang.SetCurSel(0);

	m_desc.RemoveAll();
	m_name.RemoveAll();

	int num = m_lang.GetCount();

	for (int i = 0; i < num; ++i)
	{
		m_lang.GetLBText(i, key);

		int num = GetPrivateProfileString(key, L"title", L"", buff, sizeof(buff), iniPath);

		if (num > 5)
			m_name.SetAt(key, buff);

		if ( i == 0 )
			SetDlgItemText(IDC_NAME, buff);

		num = GetPrivateProfileString(key, L"desc", L"", buff, sizeof(buff), iniPath);

		if (num > 5)
			m_desc.SetAt(key, buff);

		if (i == 0)
			SetDlgItemText(IDC_DESC, buff);
	}
	GetPrivateProfileStruct(L"options", L"data", &m_data, sizeof SModData, iniPath);

	UpdateData(FALSE);

	int i = 0;

	while (gTags[i].id > 0)
	{
		CButton* btn = (CButton*)GetDlgItem(gTags[i].id);

		if (m_data.tags & gTags[i].val)
			btn->SetCheck(1);
		else
			btn->SetCheck(0);

		++i;
	}
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::Reset()
{
	m_lang.SetCurSel(0);

	m_desc.RemoveAll();
	m_name.RemoveAll();

	SetDlgItemText(IDC_NAME, L"");
	SetDlgItemText(IDC_DESC, L"");

	UpdateData(FALSE);

	int i = 0;

	while (gTags[i].id > 0)
	{
		CButton* btn = (CButton*)GetDlgItem(gTags[i].id);

		btn->SetCheck(0);

		++i;
	}
	m_data.modId = 0;
	m_data.universe = 0;
	m_data.tags = 0;
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnValidate()
{
	CString sPath;
	CString iniPath;
	CFileStatus fstat;

	UpdateData();

	GetDlgItemText(IDC_PATH, sPath);

	m_bValid = true;

	log(L"");
	log(L"NEW VALIDATE");
	log(L"------------");

	if (sPath.IsEmpty())
	{
		m_bValid = false;
		log(L"You need a Mod Path!");
		return;
	}
	WIN32_FIND_DATAW fdata;

	CString fold(sPath);

	fold += "\\*";

	BOOL bOK = TRUE;
	HANDLE hSearch = ::FindFirstFile(fold, &fdata);

	int count = 0;

	while (hSearch != INVALID_HANDLE_VALUE && bOK)
	{
		if (lstrcmp(fdata.cFileName, L"preview.png") == 0)
			count++;

		bOK = ::FindNextFile(hSearch, &fdata);
	}

	if (hSearch != INVALID_HANDLE_VALUE)
		::FindClose(hSearch);

	if (count < 1)
	{
		m_bValid = false;
		log(L"preview.png missing, remember it's case sensitive.\n");
	}
	iniPath  = sPath;
	iniPath += INIFILE;

	if (!CFile::GetStatus(sPath + "\\EnglishModIntro.txt", fstat))
	{
		m_bValid = false;
		log(L"EnglishModIntro.txt not found!");
	}
	
	if (!CFile::GetStatus(sPath + "\\preview.png", fstat))
	{
		m_bValid = false;
		log(sPath + L"\\preview.png (workshop preview image) not found!");
	}
	else if (fstat.m_size > 1048576)
	{
		log(sPath + L"\\preview.png (workshop preview image) too large (1MB max)!");
		m_bValid = false;
	}

	// this just updates the maps
	OnCbnSelchangeLanguage();

	bool    bFound = false;
	CString key;
	CString value;

	for (POSITION pos = m_name.GetStartPosition(); pos != NULL;)
	{
		m_name.GetNextAssoc(pos, key, value);

		if (value.GetLength() > 128)
		{
			m_bValid = false;
			log(key + L" too long, max 128 for title");
		}
		else if (key == "English" && value.GetLength() > 5)
			bFound = true;

		WritePrivateProfileString(key, L"title", value, iniPath);
	}

	if (!bFound)
	{
		m_bValid = false;
		log(L"must have at least an English Title");
	}
	bFound = false;

	for (POSITION pos = m_desc.GetStartPosition(); pos != NULL;)
	{
		m_desc.GetNextAssoc(pos, key, value);

		if (value.GetLength() > 8000)
		{
			m_bValid = false;
			log(key + L" too long, max 8000 for description");
		}
		else if (key == "English" && value.GetLength() > 5)
			bFound = true;

		WritePrivateProfileString(key, L"desc", value, iniPath);
	}

	if (!bFound)
	{
		m_bValid = false;
		log(L"must have at least an English Description");
	}
	m_data.universe = 0;
	m_data.tags = 0;

	int i = 0;

	while (gTags[i].id > 0)
	{
		if (IsDlgButtonChecked(gTags[i].id))
			m_data.tags |= gTags[i].val;

		++i;
	}

	if (m_data.tags == 0)
	{
		m_bValid = false;
		log(L"no tags selected");
	}
	WritePrivateProfileStruct(L"options", L"data", &m_data, sizeof SModData, iniPath);

	if ( m_bValid )
		log(L"VALIDATION PASSED!  You can upload now.");
}

//*****************************************************************************

uint64_t CSOWSteamWorkshopDlg::Submit(uint64_t modid)
{
	UGCUpdateHandle_t handle = SteamUGC()->StartItemUpdate(STEAM_APP_ID, modid);

	CString str;
	CString sPath;

	GetDlgItemText(IDC_PATH, sPath);

	CT2A sstr(sPath);

	str.Format(L"Submitting mod id: %llu\n", modid);

	log(str);

	if (!SteamUGC()->SetItemContent(handle, sstr.m_psz))
	{
		log(L"error setting mod path.");
		return NULL;
	}
	CString iniPath = sPath;
	iniPath += INIFILE;

	m_data.modId = modid;

	WritePrivateProfileStruct(L"options", L"data", &m_data, sizeof SModData, iniPath);

	for (POSITION pos = m_name.GetStartPosition(); pos != NULL;)
	{
		CString key;
		CString value;

		m_name.GetNextAssoc(pos, key, value);

		CT2A kstr(key);
		CT2A vstr(value);
		
		if (!SteamUGC()->SetItemUpdateLanguage(handle, kstr.m_psz))
			log(L"error setting language.");

		if (!SteamUGC()->SetItemTitle(handle, vstr.m_psz))
			log(L"error setting title.");
	}

	for (POSITION pos = m_desc.GetStartPosition(); pos != NULL;)
	{
		CString key;
		CString value;

		m_desc.GetNextAssoc(pos, key, value);

		CT2A kstr(key);
		CT2A vstr(value);

		if (!SteamUGC()->SetItemUpdateLanguage(handle, kstr.m_psz))
			log(L"error setting language.");

		if (!SteamUGC()->SetItemDescription(handle, vstr.m_psz))
			log(L"error setting title.");
	}
	str  = sPath;
	str += "\\preview.png";

	CT2A istr(str);

	if ( !SteamUGC()->SetItemPreview(handle, istr.m_psz))
		log(L"error setting image preview.png");

	int i = 0;

	CStringArray tags;

	while (gTags[i].id > 0)
	{
		CT2A cstr(gTags[i].name);

		if (m_data.tags & gTags[i].val)
			tags.Add(gTags[i].name);

		++i;
	}

#ifdef SOWGB
	tags.Add(L"Gettysburg");
#else
	tags.Add(L"Waterloo");
#endif

	SteamParamStringArray_t ptags;

	ptags.m_ppStrings = new const char* [tags.GetSize()];
	ptags.m_nNumStrings = (int)tags.GetSize();

	for (i = 0; i < tags.GetSize(); ++i)
	{
		CT2A cstr(tags[i]);

		char *buff = new char[256];

		ptags.m_ppStrings[i] = buff;

		strcpy(buff, cstr.m_psz);
	}

	if (!SteamUGC()->SetItemTags(handle, &ptags))
		log(L"error setting item tags");

	for (i = 0; i < tags.GetSize(); ++i)
		delete ptags.m_ppStrings[i];

	delete ptags.m_ppStrings;

	return handle;
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnClickedPathdlg()
{
	CString sPath;
	CString oldPath;
	CFileStatus stat;

	TCHAR buff[512];

	GetCurrentDirectory(sizeof buff, buff);

	CFolderPickerDialog fdlg(buff);
	
	if (IDOK == fdlg.DoModal())
	{
		sPath = fdlg.GetPathName();

		GetDlgItemText(IDC_PATH, oldPath);
		SetDlgItemText(IDC_PATH, sPath);
		
		log(L"Mod Path Added:");
		log(sPath);

		if (oldPath != sPath)
		{
			Reset();

			if (CFile::GetStatus(sPath + INIFILE, stat))
				LoadMod();
		}
	}
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnBnClickedOk()
{
	OnValidate();

	if (!m_bValid)
	{
		MessageBox(L"You must fix the errors first");
		return;
	}

	if (m_data.modId > 0)
		m_loader->submit(m_data.modId);
	else
		m_loader->create_item(STEAM_APP_ID);
}

//*****************************************************************************

void CSOWSteamWorkshopDlg::OnBnClickedCancel()
{
	KillTimer(0);

	// just to save
	OnValidate();

	delete m_loader;

	CDialogEx::OnCancel();
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
