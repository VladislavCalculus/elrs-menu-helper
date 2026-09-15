#include "elrs_exhange.h"
#include "../elrs_menu.h"

param_entry_t ARR_TERMINATOR {
    .name = "",
    .value = NULL,
    .entry_root = PARENT_ENTRY_ROUTE_GLOBAL,
    .param_type = PARAM_ARRAY_TERMINATOR,
    .w_func = NULL
};

param_entry_t STRING_HANDLE {
    .name = ".",
    .value = NULL,
    .entry_root = PARENT_ENTRY_ROUTE_GLOBAL,
    .param_type = CRSF_PARAM_TYPE_STRING,
    .w_func = NULL
};

uint8_t requester = 0;
uint8_t param_count = 0;
uint8_t c_buffer[MAX_PACKET_SIZE];
bool handle_exchange(uint8_t buffer[], lua_elrs_menu_config_t *cfg) {
    if(buffer[2] == CRSF_TYPE_DEVICE_PING || (buffer[2] == CRSF_FRAMETYPE_PARAMETER_WRITE && buffer[5] == 0x00)) {
        crsf_device_ping_response(buffer[4], cfg->ping_responce, cfg->prameters_list);
        requester = buffer[4];
        return true;
    } else if(buffer[3] == cfg->address) {
        if (requester != 0x0 && buffer[2] == CRSF_FRAMETYPE_PARAMETER_READ) {
            if(cfg->prameters_list[param_count].param_type == PARAM_ARRAY_TERMINATOR) {
                requester = 0;
                param_count = 0;
                return true;
            }
            crsf_send_param_entry_reply(param_count+1, cfg->address, cfg->prameters_list[param_count]);
            param_count++;
            if(cfg->ping_responce->param_count == param_count) {
                requester = 0;
                param_count = 0;
            }
        }
        if(buffer[2] == CRSF_FRAMETYPE_PARAMETER_WRITE) {
            crsf_device_write(buffer, buffer[5] != 0xFF ? &cfg->prameters_list[buffer[5]-1] : &STRING_HANDLE);
        }
        return true;
    }
    return false;
}
