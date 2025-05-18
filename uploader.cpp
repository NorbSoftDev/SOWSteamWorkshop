//*****************************************************************************

#include "pch.h"
#include "framework.h"
#include "SOWSteamWorkshop.h"
#include "SOWSteamWorkshopDlg.h"
#include "uploader.h"

#include "../Steam/public/steam/isteamugc.h"
#include "../Steam/public/steam/steam_api.h"

#define LEGALURL L"http://steamcommunity.com/sharedfiles/workshoplegalagreement" 

WorkshopUploader::WorkshopUploader() : callback_called(false) {}

//*****************************************************************************

void WorkshopUploader::create_callback(CreateItemResult_t *result, bool _bool)
{
  wchar_t buff[256];

  if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement) 
  {
    ::ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), _T("open"), LEGALURL, NULL, NULL, SW_SHOW);

    CString str = L"By submitting this item, you agree to the workshop\n";

    str += L"terms of service < http://steamcommunity.com/sharedfiles/workshoplegalagreement>\n\n";
    str += L"Press Yes to Accept.\n";

    if (theDlg->MessageBox(str, L"Steam Workshop Terms of Service", MB_YESNO) != IDYES)
      exit(1);
  }

  switch (result->m_eResult) 
  {
    case k_EResultAccessDenied:
      theDlg->MessageBox(L"Access Denied, this can happen launching WL and uploading a GB mod or the reverse.\n");
      break;

  case k_EResultInsufficientPrivilege:
      theDlg->MessageBox(L"You have insufficient privilege to make a workshop item\n");
      exit(1);
      break;

    case k_EResultTimeout:
      theDlg->MessageBox(L"The operation took longer than expected; please try again\n");
      break;

    case k_EResultNotLoggedOn:
      theDlg->MessageBox(L"Need to be logged into Steam\n");
      exit(1);
      break;

    case k_EResultOK:
      theDlg->MessageBox(L"Success!!\n");
      break;

    default:
      wsprintf(buff, L"Unhandled result from Steam id# %d", result->m_eResult);
      theDlg->MessageBox(buff);
      return;
  }
  wsprintf( buff, L"Your new item is id# %llu", result->m_nPublishedFileId );

  CString str(buff);

  theDlg->log(str.GetString());

  submit(result->m_nPublishedFileId);
}

//*****************************************************************************

void WorkshopUploader::submit(uint64_t mod)
{
  UGCUpdateHandle_t handle = theDlg->Submit(mod);

  if (handle == NULL)
    return;

  SteamAPICall_t call = SteamUGC()->SubmitItemUpdate(handle, "SOW Mod Uploader");

  submit_call.Set(call, this, &WorkshopUploader::submit_callback);

  bool bDone = false;

  int last = -1;

  while (!bDone)
  {
    uint64 dbeg = 0;
    uint64 dtot = 0;

    CString str;
    CString msg;

    EItemUpdateStatus stat = SteamUGC()->GetItemUpdateProgress(handle, &dbeg, &dtot);

    switch ( stat )
    {
    case k_EItemUpdateStatusInvalid: str = L"The item update handle was invalid, job might be finished, listen too SubmitItemUpdateResult_t"; bDone = true; break;
    case k_EItemUpdateStatusPreparingConfig: str = L"The item update is processing configuration data"; break;
    case k_EItemUpdateStatusPreparingContent: str = L"The item update is reading and processing content files"; break;
    case k_EItemUpdateStatusUploadingContent: str = L"The item update is uploading content changes to Steam"; break;
    case k_EItemUpdateStatusUploadingPreviewFile: str = L"The item update is uploading new preview file image"; break;
    case k_EItemUpdateStatusCommittingChanges: str = L"The item update is committing all changes"; bDone = true; break;
    }
    msg.Format(L"%llu of %llu: %s", dbeg, dtot, str.GetString());

    if ( last != (int)stat )
      theDlg->log(msg.GetString());

    last = (int)stat;
  }
}

//*****************************************************************************

void WorkshopUploader::submit_callback(SubmitItemUpdateResult_t *result, bool _bool)
{
  wchar_t buff[256];

  this->callback_called = true;

  switch (result->m_eResult) 
  {
    case k_EResultInsufficientPrivilege:
      theDlg->log(L"You have insufficient privilege to upload the mod.\n");
      break;
    case k_EResultTimeout:
      theDlg->log(L"Steam has timed out.\n");
      break;
    case k_EResultNotLoggedOn:
      theDlg->log(L"Please log onto Steam.\n");
      break;
    case k_EResultOK:
      theDlg->log(L"Mod successfully uploaded!\n");
      break;
    default:
      wsprintf(buff, L"Unhandled result from Steam id# %d\n", result->m_eResult);
      theDlg->log(buff);
      theDlg->MessageBox(buff);
      return;
  }
}

//*****************************************************************************

void WorkshopUploader::create_item(AppId_t game)
{
  theDlg->log(L"Creating item for ");
//  << game << "\n");
  SteamAPICall_t result = SteamUGC()->CreateItem(game, k_EWorkshopFileTypeCommunity);
  
  theDlg->log(L"Created the item and setting a call result\n");
  
  call_result.Set(result, this, &WorkshopUploader::create_callback);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
