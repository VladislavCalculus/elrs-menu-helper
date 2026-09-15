#ifndef __uart_dfn__
#define __uart_dfn__

#ifdef __cplusplus
extern "C" {
#endif

#include "entry_classes/entr_cls.h"

#define CRSF_ADDRESS_FLIGHT_CONTROLLER            0xC8
#define CRSF_ADDRESS_RADIO_TRANSMITTER            0xEA
#define CRSF_ADDRESS_CRSF_RECEIVER                0xEC
#define CRSF_ADDRESS_CRSF_TRANSMITTER             0xEE
#define CRSF_ADDRESS_ELRS_LUA                     0xEF

#define PARENT_ENTRY_ROUTE_GLOBAL                 0x0 //if entry as folder created, it is writen there

typedef enum {
    CRSF_PARAM_TYPE_UINT8          = 0,  // 0..255 (step/min/max may apply)
    CRSF_PARAM_TYPE_INT8           = 1,  // -128..127
    CRSF_PARAM_TYPE_UINT16         = 2,  // 0..65535
    CRSF_PARAM_TYPE_INT16          = 3,  // -32768..32767
    // CRSF_PARAM_TYPE_UINT32         = 4,  // 0..4294967295
    // CRSF_PARAM_TYPE_INT32          = 5,  // -2147483648..2147483647
    //CRSF_PARAM_TYPE_UINT64         = 6,
    //CRSF_PARAM_TYPE_INT64          = 7,
    //CRSF_PARAM_TYPE_FLOAT          = 8,  // IEEE-754 float
    CRSF_PARAM_TYPE_TEXT_SELECTION = 9,  // choose from indexed list of strings
    CRSF_PARAM_TYPE_STRING         = 10,  // editable string (variable length)
    CRSF_PARAM_TYPE_FOLDER         = 11, // container node (no value)
    CRSF_PARAM_TYPE_COMMAND        = 13, // action button (write-only trigger)
    CRSF_PARAM_TYPE_INFO           = 12,  // read-only string

    PARAM_ARRAY_TERMINATOR         = 0xff
} crsf_param_type_e;

typedef struct {
    const char *name;
    uint32_t serial;
    uint32_t hwver;
    uint32_t swver;
    uint8_t param_count;
    uint8_t param_proto;
    void (*w_func)(uint8_t *buffer, size_t size);
} crsf_ping_responce_t;

typedef struct {
    const char *name;
    param_entry_data *value;
    uint8_t entry_root;
    crsf_param_type_e param_type;
    void (*w_func)(uint8_t* buffer, size_t size);
} param_entry_t;

extern param_entry_t ARR_TERMINATOR;

#ifdef __cplusplus
}
#endif

#endif // __uart_dfn__