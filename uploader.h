
#pragma once

#include "../Steam/public/steam/isteamugc.h"
#include "../Steam/public/steam/steam_api.h"

class WorkshopUploader
{
public:
    WorkshopUploader();

    void create_item(AppId_t game);

    void submit(uint64_t mod);

    void create_callback(CreateItemResult_t *result, bool _bool);

    void submit_callback(SubmitItemUpdateResult_t *result, bool _bool);

    CCallResult<WorkshopUploader, CreateItemResult_t> call_result;

    CCallResult<WorkshopUploader, SubmitItemUpdateResult_t> submit_call;

    bool callback_called;
};
