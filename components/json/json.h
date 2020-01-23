#include "cJSON.h"
#define COMMAND_UNLOCK "UNLOCK"
#define COMMAND_RFID_ADD "ADD"
#define COMMAND_RFID_REMOVE "REMOVE"
#define COMMAND_RFID_UPDATE "UPDATE"
#define COMMAND_RFID_NEW "NEW"
#define COMMAND_DATA "DATA"
#define MAX_DATA_LENGTH 15

enum command{
    UNLOCK = 0,
    ADD = 1,
    REMOVE = 2,
    UPDATE = 3,
    NEW = 4,
    DATA = 5,
    FILE = 6,
};

typedef struct{
    enum command type;
    char data[2][MAX_DATA_LENGTH];
} message_template;

int cToJSON(message_template message, char **char_json);