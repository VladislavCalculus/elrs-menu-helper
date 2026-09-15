#ifndef ELRS_MENU_EXCHANGE_INTERNAL_H
#define ELRS_MENU_EXCHANGE_INTERNAL_H
#include <cstddef>
#include <cstdint>
#include "elrs_menu/elrs_menu.hpp"
constexpr std::size_t MAX_PACKET_SIZE = 64;
constexpr std::size_t MAX_PAYLOAD_SIZE = 56;
constexpr uint8_t CRSF_LEN_MAX = 62;
constexpr uint8_t CRSF_TYPE_DEVICE_PING = 0x28;
constexpr uint8_t CRSF_TYPE_DEVICE_INFO = 0x29;
constexpr uint8_t CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B;
constexpr uint8_t CRSF_FRAMETYPE_PARAMETER_READ = 0x2C;
constexpr uint8_t CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D;
void crsf_device_ping_response(uint8_t requester, const elrs_menu_config_t &config);
void crsf_send_param_entry_reply(uint8_t entry_id, const elrs_menu_config_t &config, param_entry_t &entry);
void crsf_device_write(const uint8_t *packet, std::size_t packet_size, const elrs_menu_config_t &config, param_entry_t &entry);
#endif
