#include "../entr_cls.h"

uint8_t text_select_obj_t::count_options(const char* s, size_t n) {
    uint8_t cnt = 0; bool in = false;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] == ';') { if (in) { ++cnt; in = false; } }
        else if (s[i] != '\0') { in = true; }
    }
    if (in) ++cnt;
    return cnt;
}

inline uint8_t text_select_obj_t::tail_byte(size_t idx) const {
    switch (idx) {
        case 0: return *out;
        case 1: return 0;
        case 2: return n_options ? (uint8_t)(n_options - 1) : 0;
        case 3: return 0;
        case 4: return 0;
        default: return 0;
    }
}

text_select_obj_t::text_select_obj_t(uint8_t* out_ptr, const char* options, size_t options_size) : param_entry_data(out_ptr) {
    selection = reinterpret_cast<const uint8_t*>(options);
    str_len   = strnlen(options, options_size);
    n_options = count_options(options, str_len);
}

size_t text_select_obj_t::get_size() {
    return str_len + 1 + TAIL_LEN;
}

packet_result_t text_select_obj_t::form_packet(uint8_t* buffer, size_t b_size) {
    if (!buffer || b_size == 0) return packet_result_t::invalid_argument;

    const size_t total = get_size();
    size_t out_i = 0;

    while (out_i < b_size && written < total) {
        uint8_t byte;
        if (written < str_len) {
            byte = selection[written];
        } else if (written == str_len) {
            byte = 0;
        } else {
            const size_t tidx = written - (str_len + 1);
            byte = tail_byte(tidx);
        }

        buffer[out_i++] = byte;
        ++written;
    }

    if (written >= total) {
        written = 0;
        return packet_result_t::complete;
    }
    return packet_result_t::incomplete;
}

void text_select_obj_t::drop_written_counter() { written = 0; }
