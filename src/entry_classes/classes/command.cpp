#include "../entr_cls.h"

command_obj_t::command_obj_t(const char *text, size_t t_size, uint8_t _timeout, EventHandler handler) : param_entry_data(NULL) {
    description = (uint8_t*)text;
    desc_size = t_size;
    timeout = _timeout;
    event_handler = handler;
}

size_t command_obj_t::get_size() {
    return desc_size+2;
}

packet_result_t command_obj_t::form_packet(uint8_t* buffer, size_t b_size) {
    if(b_size < 3) return packet_result_t::invalid_argument;
    
    if(written == 0) {
        buffer[0] = type;
    }
    if(written == 0 || written == 1) {
        buffer[1] = timeout;
    }
    
    for(size_t i = 2; i < b_size; i++) {
        if(written >= desc_size || description[written] == '\0') {
            buffer[i] = '\0';
            drop_written_counter();
            return packet_result_t::complete;
        }

        buffer[i] = description[written];
        written++;
    }
    return packet_result_t::incomplete;
}

void command_obj_t::handle_event(command_type_e event) {
    if (event_handler) event_handler(*this, event);
}

void command_obj_t::drop_written_counter() { written = 0; }

void command_obj_t::change_description(const char *text, size_t t_size) {
    description = (uint8_t*)text;
    desc_size = t_size;
}
