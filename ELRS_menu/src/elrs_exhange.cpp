#include "elrs_exhange.h"
#include <esp_log.h>
#include "interface/interface.h"
#include "../elrs_menu.h"

static inline int crc8_data(uint8_t data[], int data_len) {
    uint8_t crc = 0;
    for(int i = 0; i < data_len; i++) {
        crc = crc ^ data[i];
        for(int j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0xD5;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

void crsf_send_param_entry_reply(uint8_t entry_id, uint8_t req_extsrc, param_entry_t e_cfg)
{
    param_entry_data* data_ptr = e_cfg.value;
    const size_t total_bytes = data_ptr != NULL ? data_ptr->get_size() : 0;   // includes '\0' + 5 tail bytes
    size_t data_sent = 0;

    for (;;) {
        uint8_t payload[MAX_PAYLOAD_SIZE];
        size_t p = 0;

        if (data_sent == 0) {
            payload[p++] = e_cfg.entry_root;
            payload[p++] = e_cfg.param_type;
            const size_t name_len = strlen(e_cfg.name) + 1;
            const size_t fit = MAX_PAYLOAD_SIZE - p;
            const size_t copy = (name_len > fit) ? fit : name_len;
            memcpy(&payload[p], e_cfg.name, copy);
            p += copy;
        }

        const size_t data_cap = MAX_PAYLOAD_SIZE - p;
        size_t wrote = 0;
        bool done = false;
        if(data_ptr != NULL) {
            if (data_cap > 0) {
                const esp_err_t fp = data_ptr->form_packet(payload + p, data_cap);
                if (fp == ESP_ERR_NOT_FINISHED) {
                    wrote = data_cap;
                } else if (fp == ESP_OK) {
                    const size_t remaining = total_bytes - data_sent;
                    wrote = remaining;
                    done = true;
                } else {
                    return;
                }
            }

            p += wrote;
            data_sent += wrote;
        } else {
            done = true;
        }

        const size_t remaining = (total_bytes > data_sent) ? (total_bytes - data_sent) : 0;
        const uint8_t packets_left =
            (remaining == 0) ? 0 : (uint8_t)((remaining + (MAX_PAYLOAD_SIZE - 1)) / MAX_PAYLOAD_SIZE);

        const uint8_t len_field = (uint8_t)(1 + 2 + 2 + p + 1);
        if (len_field > CRSF_LEN_MAX) return;

        uint8_t buf[2 + 1 + 2 + 2 + MAX_PAYLOAD_SIZE + 1];
        size_t i = 0;
        buf[i++] = CRSF_ADDR_TX;
        buf[i++] = len_field;
        buf[i++] = CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY;
        buf[i++] = CRSF_ADDRESS_ELRS_LUA;
        buf[i++] = req_extsrc;
        buf[i++] = entry_id;
        buf[i++] = packets_left;
        memcpy(&buf[i], payload, p); i += p;
        buf[i++] = crc8_data(&buf[2], (size_t)len_field - 1);

        e_cfg.w_func(buf, i);

        if (remaining == 0) break;
        if (done) break;
    }
}

const uint8_t CRSF_WRITE_STRING_SUCCESS[] = {
  0xC8, 0x0D, 0x2D, 0xEE, 0xEF, 0xFF,
  'S','u','c','c','e','s','s', 0x00,
  0xE7
};

const uint8_t CRSF_WRITE_STRING_FAIL[] = {
  0xC8, 0x0A, 0x2D, 0xEE, 0xEF, 0xFF,
  'F','a','i','l', 0x00,
  0xCD
};

param_entry_t *command_entry_cfg;
static TaskHandle_t e_cmd_task = NULL;

bool confirmed = false;
bool canceled = false;
void handle_command_execution_task(void *pvParameters){
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        command_obj_t *cmd_obj = (command_obj_t*)command_entry_cfg->value;

        confirmed = false;
        canceled = false;
        cmd_obj->type = CDM_RESP_EXECUTING;

        cmd_obj->execute(&confirmed, &canceled);

        cmd_obj->type = CDM_RESP_IDLE;
    }
}

static inline void ensure_cmd_task_started()
{
    if (e_cmd_task == NULL) {
        xTaskCreatePinnedToCore(
            handle_command_execution_task,
            "handle command execution task",
            1024*4,
            NULL,
            10,
            &e_cmd_task,
            0
        );
    }
}

const uint8_t PAYLOAD_OFFSET = 6;
const uint8_t TYPE_IDX = 6;
void crsf_device_write(uint8_t packet[], param_entry_t *e_cfg) {
    if(e_cfg->param_type >= CRSF_PARAM_TYPE_UINT8 && e_cfg->param_type <= CRSF_PARAM_TYPE_INT16) {
        int_obj_t *num_obj = (int_obj_t*) e_cfg->value;
        num_obj->apply_write(&packet[PAYLOAD_OFFSET], num_obj->value_width());
    } else if(e_cfg->param_type == CRSF_PARAM_TYPE_TEXT_SELECTION) {
        *(e_cfg->value->out) = packet[PAYLOAD_OFFSET];
    } else if(e_cfg->param_type == CRSF_PARAM_TYPE_COMMAND) {
        switch (packet[TYPE_IDX]) {
            case CMD_CLICK: {
                ensure_cmd_task_started();
                command_entry_cfg = e_cfg;
                xTaskNotifyGive(e_cmd_task);
                break;
            }

            case CMD_CONFIRMED:
                confirmed = true;
                break;

            case CMD_CANCEL:
                canceled = true;
                break;
                
            case CMD_QUERY: {
                crsf_send_param_entry_reply(packet[5], 0xEE, *e_cfg);
                break;
            }

            default:
                break;
        }
    }
    //this is purelly for CUSTOM config from remote handleing, it isn't usiversal. Change for your needs.
    else if(e_cfg->param_type == CRSF_PARAM_TYPE_STRING) {
        char c_buffer[MAX_BUFF_SIZE];
        char *buf[MAX_TOKENS+1];
        uint8_t n = 0;

        if(parse_string((char*)&packet[PAYLOAD_OFFSET], buf, &n) != ESP_OK) {
            uart_write_bytes(UART_NUM_2, CRSF_WRITE_STRING_FAIL, sizeof(CRSF_WRITE_STRING_FAIL));
        } else {
            return;
        }

        char **arg = (n > 1) ? &buf[1] : NULL;
        if(execute_command(buf[0], !custom_commands ? COMMANDS : CUSTOM_COMMANDS, !custom_commands ? NUM_OF_COMMANDS : NUM_OF_CUSTOM_COMMANDS, arg, c_buffer) == ESP_OK) {
            uart_write_bytes(UART_NUM_2, CRSF_WRITE_STRING_SUCCESS, sizeof(CRSF_WRITE_STRING_SUCCESS));
        } else {
            uart_write_bytes(UART_NUM_2, CRSF_WRITE_STRING_FAIL, sizeof(CRSF_WRITE_STRING_FAIL));
        }
    }
}

static void responce_cfg_count_entry(crsf_ping_responce_t *r_cfg, param_entry_t e_cfg[]) {
    if(e_cfg == NULL) {
        return;
    }

    for(uint8_t i = 0; i < UINT8_MAX; i++) {
        if(e_cfg[i].param_type == PARAM_ARRAY_TERMINATOR) {
            r_cfg->param_count = i;
            return;
        }
    }
}

void crsf_device_ping_response(uint8_t ping_extsrc, crsf_ping_responce_t *r_cfg, param_entry_t e_cfg[])
{
    responce_cfg_count_entry(r_cfg, e_cfg);
    size_t name_len    = strlen(r_cfg->name) + 1; // include NUL
    size_t devinfo_len = name_len + 4 + 4 + 4 + 1 + 1;
    uint8_t len_field  = (uint8_t)(1 /*type*/ + 2 /*ext*/ + devinfo_len + 1 /*crc*/);

    if (len_field > CRSF_LEN_MAX) {
        size_t min_fixed = 4+4+4+1+1;
        size_t max_name  = (CRSF_LEN_MAX - (1+2+1)) - min_fixed;
        if ((int)max_name <= 0) return;
        if (name_len > max_name) name_len = max_name;
        devinfo_len = name_len + min_fixed;
        len_field   = (uint8_t)(1 + 2 + devinfo_len + 1);
    }

    uint8_t buf[2 + 1 + 2 + 58 + 1];
    size_t i = 0;

    buf[i++] = CRSF_ADDR_TX;
    buf[i++] = len_field;
    buf[i++] = CRSF_TYPE_DEVICE_INFO;
    buf[i++] = ping_extsrc;
    buf[i++] = OUT_EXT_ADDR;

    memcpy(&buf[i], r_cfg->name,   name_len); i += name_len;
    memcpy(&buf[i], &r_cfg->serial, 4);       i += 4;
    memcpy(&buf[i], &r_cfg->hwver,  4);       i += 4;
    memcpy(&buf[i], &r_cfg->swver,  4);       i += 4;
    buf[i++] = r_cfg->param_count;
    buf[i++] = r_cfg->param_proto;

    buf[i++] = crc8_data(&buf[2], (size_t)len_field - 1);
    r_cfg->w_func(buf, i);
}