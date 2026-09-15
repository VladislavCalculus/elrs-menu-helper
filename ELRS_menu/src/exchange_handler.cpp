#include "elrs_exhange.h"

void elrs_menu_t::reset() { requester_ = 0; next_parameter_ = 0; }

bool handle_exchange(elrs_menu_t &menu, const uint8_t *frame, std::size_t frame_size) {
    const elrs_menu_config_t *config = &menu.config_;
    if (!frame || !config->write || !config->device_info.name ||
        (config->parameter_count && !config->parameters) || config->parameter_count > 255 || frame_size < 6) return false;
    const std::size_t frame_length = static_cast<std::size_t>(frame[1]) + 2;
    if (frame[1] < 4 || frame_length > frame_size) return false;
    const uint8_t type = frame[2];
    const uint8_t requester = frame[4];
    if (type == CRSF_TYPE_DEVICE_PING) {
        crsf_device_ping_response(requester, *config); menu.requester_ = requester; menu.next_parameter_ = 0; return true;
    }
    if (frame[3] != config->addresses.device_address) return false;
    if (type == CRSF_FRAMETYPE_PARAMETER_READ) {
        if (menu.requester_ && menu.next_parameter_ < config->parameter_count) {
            crsf_send_param_entry_reply(static_cast<uint8_t>(menu.next_parameter_ + 1), *config, config->parameters[menu.next_parameter_++]);
            if (menu.next_parameter_ == config->parameter_count) menu.reset();
        }
        return true;
    }
    if (type == CRSF_FRAMETYPE_PARAMETER_WRITE) {
        const uint8_t id = frame[5];
        if (id == 0) { crsf_device_ping_response(requester, *config); menu.requester_ = requester; menu.next_parameter_ = 0; }
        else if (id != 0xff && id <= config->parameter_count) crsf_device_write(frame, frame_length, *config, config->parameters[id - 1]);
        return true;
    }
    return true;
}
