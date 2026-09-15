#include "elrs_exhange.h"
#include <cstring>

static uint8_t crc8_data(const uint8_t *data, std::size_t count) {
    uint8_t crc = 0;
    for (std::size_t i = 0; i < count; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = crc & 0x80 ? static_cast<uint8_t>((crc << 1) ^ 0xD5) : static_cast<uint8_t>(crc << 1);
    }
    return crc;
}
static void put_u32_le(uint8_t *out, uint32_t v) {
    out[0] = static_cast<uint8_t>(v); out[1] = static_cast<uint8_t>(v >> 8);
    out[2] = static_cast<uint8_t>(v >> 16); out[3] = static_cast<uint8_t>(v >> 24);
}

void crsf_send_param_entry_reply(uint8_t entry_id,
                                 const elrs_menu_config_t &config, param_entry_t &entry) {
    param_entry_data *data = entry.value;
    const std::size_t total = data ? data->get_size() : 0;
    std::size_t sent = 0;
    for (;;) {
        uint8_t payload[MAX_PAYLOAD_SIZE]{};
        std::size_t payload_size = 0;
        if (sent == 0) {
            payload[payload_size++] = entry.entry_root;
            payload[payload_size++] = static_cast<uint8_t>(entry.param_type);
            const std::size_t name_size = std::strlen(entry.name) + 1;
            const std::size_t copy = name_size < MAX_PAYLOAD_SIZE - payload_size ? name_size : MAX_PAYLOAD_SIZE - payload_size;
            std::memcpy(payload + payload_size, entry.name, copy); payload_size += copy;
        }
        bool complete = !data;
        if (data) {
            const std::size_t cap = MAX_PAYLOAD_SIZE - payload_size;
            if (!cap) return;
            const packet_result_t result = data->form_packet(payload + payload_size, cap);
            if (result == packet_result_t::invalid_argument) return;
            const std::size_t wrote = result == packet_result_t::incomplete ? cap : total - sent;
            payload_size += wrote; sent += wrote; complete = result == packet_result_t::complete;
        }
        const std::size_t remaining = total - sent;
        const uint8_t left = remaining ? static_cast<uint8_t>((remaining + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE) : 0;
        const uint8_t length = static_cast<uint8_t>(1 + 2 + 2 + payload_size + 1);
        if (length > CRSF_LEN_MAX) return;
        uint8_t output[MAX_PACKET_SIZE]{};
        std::size_t i = 0;
        output[i++] = config.addresses.transport_destination; output[i++] = length; output[i++] = CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY;
        output[i++] = config.addresses.menu_address; output[i++] = config.addresses.device_address;
        output[i++] = entry_id; output[i++] = left;
        std::memcpy(output + i, payload, payload_size); i += payload_size;
        output[i++] = crc8_data(output + 2, length - 1);
        config.write(output, i, config.write_context);
        if (remaining == 0 || complete) return;
    }
}

void crsf_device_write(const uint8_t *packet, std::size_t size,
                       const elrs_menu_config_t &config, param_entry_t &entry) {
    constexpr std::size_t payload = 6;
    if (!entry.value || size <= payload) return;
    if (entry.param_type >= CRSF_PARAM_TYPE_UINT8 && entry.param_type <= CRSF_PARAM_TYPE_INT16) {
        auto *number = static_cast<int_obj_t *>(entry.value);
        if (size >= payload + number->value_width()) number->apply_write(packet + payload, number->value_width());
    } else if (entry.param_type == CRSF_PARAM_TYPE_TEXT_SELECTION) {
        *entry.value->out = packet[payload];
    } else if (entry.param_type == CRSF_PARAM_TYPE_COMMAND) {
        auto *command = static_cast<command_obj_t *>(entry.value);
        const auto event = static_cast<command_type_e>(packet[payload]);
        switch (event) {
        case CMD_CLICK: case CMD_CONFIRMED: case CMD_CANCEL: command->handle_event(event); break;
        case CMD_QUERY: command->handle_event(event); crsf_send_param_entry_reply(packet[5], config, entry); break;
        default: break;
        }
    }
}

void crsf_device_ping_response(uint8_t requester, const elrs_menu_config_t &config) {
    const crsf_device_info_t &info = config.device_info;
    constexpr std::size_t fixed = 14;
    std::size_t name_size = std::strlen(info.name) + 1;
    if (1 + 2 + name_size + fixed + 1 > CRSF_LEN_MAX) name_size = CRSF_LEN_MAX - (1 + 2 + fixed + 1);
    const uint8_t length = static_cast<uint8_t>(1 + 2 + name_size + fixed + 1);
    uint8_t output[MAX_PACKET_SIZE]{}; std::size_t i = 0;
    output[i++] = config.addresses.transport_destination; output[i++] = length; output[i++] = CRSF_TYPE_DEVICE_INFO;
    output[i++] = requester; output[i++] = config.addresses.device_address;
    std::memcpy(output + i, info.name, name_size); i += name_size;
    put_u32_le(output + i, info.serial); i += 4; put_u32_le(output + i, info.hardware_version); i += 4;
    put_u32_le(output + i, info.software_version); i += 4;
    output[i++] = static_cast<uint8_t>(config.parameter_count); output[i++] = info.parameter_protocol;
    output[i++] = crc8_data(output + 2, length - 1);
    config.write(output, i, config.write_context);
}
