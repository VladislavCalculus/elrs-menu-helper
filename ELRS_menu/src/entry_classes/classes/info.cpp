#include "../entr_cls.h"

info_obj_t::info_obj_t(const char text[], size_t t_size) : param_entry_data(NULL) {
    info = text;
    info_size = t_size;
}

size_t info_obj_t::get_size() {
    const size_t n = strnlen(info, info_size);
    return n + 1;
}

esp_err_t info_obj_t::form_packet(uint8_t* buffer, size_t b_size) {
    if (!buffer || b_size == 0) return ESP_ERR_INVALID_ARG;

    const size_t total = get_size();
    size_t out_i = 0;

    while (out_i < b_size && written < total) {
        buffer[out_i++] = (written < total - 1) ? (uint8_t)info[written] : (uint8_t)0;
        ++written;
    }

    if (written >= total) { written = 0; return ESP_OK; }
    return ESP_ERR_NOT_FINISHED;
}

void info_obj_t::change_info_text(const char *text, size_t t_size) {
    info = text;
    info_size = t_size;
}