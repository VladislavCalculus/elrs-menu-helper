#ifndef __lua_elrs_menu__
#define __lua_elrs_menu__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "driver/uart.h"
#include "src/user_dfn.h"

typedef struct {
    uint8_t address;
    crsf_ping_responce_t *ping_responce;
    param_entry_t *prameters_list;
} lua_elrs_menu_config_t;

//return true if ELRS menu was asked, and false if it is a regular contoll data
//buffer[] - array of CRSF before unpacking
//cfg - lua_elrs_menu_config_t, used to get variables for other fucntions
bool handle_exchange(uint8_t buffer[], lua_elrs_menu_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif // __lua_elrs_menu__