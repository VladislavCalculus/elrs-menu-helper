#include "../entr_cls.h"

command_obj_t::command_obj_t(const char *text, size_t t_size, uint8_t _timeout, ExecFn execution_func) : param_entry_data(NULL) {
    description = (uint8_t*)text;
    desc_size = t_size;
    timeout = _timeout;
    executable = execution_func;
}

size_t command_obj_t::get_size() {
    return desc_size+2;
}

esp_err_t command_obj_t::form_packet(uint8_t* buffer, size_t b_size) {
    if(b_size < 3) return ESP_ERR_INVALID_ARG;
    
    if(written == 0) {
        buffer[0] = type;
    }
    if(written == 0 || written == 1) {
        buffer[1] = timeout;
    }
    
    for(int i = 2; i < b_size; i++) {
        if(written >= desc_size || description[written] == '\0') {
            buffer[i] = '\0';
            drop_written_counter();
            return ESP_OK;
        }

        buffer[i] = description[written];
        written++;
    }
    return ESP_ERR_NOT_FINISHED;
}

void command_obj_t::execute(bool *confirmed, bool *canceled) {
    if (executable) executable(*this, confirmed, canceled);
}

void command_obj_t::drop_written_counter() { written = 0; }

void command_obj_t::change_description(const char *text, size_t t_size) {
    description = (uint8_t*)text;
    desc_size = t_size;
}