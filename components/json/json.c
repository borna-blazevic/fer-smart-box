#include "json.h"

int cToJSON(message_template message, char **char_json)
{
    cJSON *json = cJSON_CreateObject();

    switch (message.type)
    {
    case NEW:
        if (cJSON_AddStringToObject(json, "type", COMMAND_RFID_NEW) == NULL)
        {
            goto end;
        }
        if (cJSON_AddStringToObject(json, "value", message.data[0]) == NULL)
        {
            goto end;
        }

        break;
    
    case DATA:
        if (cJSON_AddStringToObject(json, "type", COMMAND_DATA) == NULL)
        {
            goto end;
        }
        if (cJSON_AddNumberToObject(json, "data", message.number_data) == NULL)
        {
            goto end;
        }
        if (cJSON_AddStringToObject(json, "user", message.data[0]) == NULL)
        {
            goto end;
        }
        if (cJSON_AddStringToObject(json, "cell_id", message.data[1]) == NULL)
        {
            goto end;
        }

        break;

    default:
        goto end;
        break;
    }
    *char_json = cJSON_Print(json);
    if (*char_json == NULL)
    {
        return -1;
    }
end:
    cJSON_Delete(json);
    return 1;
}