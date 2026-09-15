#ifndef ELRS_MENU_HPP
#define ELRS_MENU_HPP

#include <cstddef>
#include <cstdint>

#include "elrs_menu/entry_classes.hpp"

constexpr uint8_t CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8;
constexpr uint8_t CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA;
constexpr uint8_t CRSF_ADDRESS_CRSF_RECEIVER = 0xEC;
constexpr uint8_t CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE;
constexpr uint8_t CRSF_ADDRESS_ELRS_LUA = 0xEF;

constexpr uint8_t PARENT_ENTRY_ROUTE_GLOBAL = 0x00;

enum crsf_param_type_e : uint8_t {
    CRSF_PARAM_TYPE_UINT8 = 0,
    CRSF_PARAM_TYPE_INT8 = 1,
    CRSF_PARAM_TYPE_UINT16 = 2,
    CRSF_PARAM_TYPE_INT16 = 3,
    CRSF_PARAM_TYPE_TEXT_SELECTION = 9,
    CRSF_PARAM_TYPE_STRING = 10, // Reserved; generic editable strings are not implemented.
    CRSF_PARAM_TYPE_FOLDER = 11,
    CRSF_PARAM_TYPE_INFO = 12,
    CRSF_PARAM_TYPE_COMMAND = 13,
};

struct param_entry_t {
    const char *name = "";
    param_entry_data *value = nullptr;
    uint8_t entry_root = PARENT_ENTRY_ROUTE_GLOBAL;
    crsf_param_type_e param_type = CRSF_PARAM_TYPE_INFO;
};

struct crsf_device_info_t {
    const char *name = "";
    uint32_t serial = 0;
    uint32_t hardware_version = 0;
    uint32_t software_version = 0;
    uint8_t parameter_protocol = 0;
};

struct crsf_menu_addresses_t {
    // Destination of the outer CRSF frame (normally the radio transmitter).
    uint8_t transport_destination = CRSF_ADDRESS_RADIO_TRANSMITTER;
    // Address assigned to this CRSF device and used as its extended source.
    uint8_t device_address = CRSF_ADDRESS_CRSF_TRANSMITTER;
    // Extended destination for parameter-entry replies. ELRS Lua uses 0xEF.
    uint8_t menu_address = CRSF_ADDRESS_ELRS_LUA;
};

using crsf_menu_write_fn = void (*)(const uint8_t *buffer, size_t size, void *context);

struct elrs_menu_config_t {
    crsf_menu_addresses_t addresses;
    crsf_device_info_t device_info;
    param_entry_t *parameters = nullptr;
    size_t parameter_count = 0;
    crsf_menu_write_fn write = nullptr;
    void *write_context = nullptr;
};

class elrs_menu_t {
public:
    explicit elrs_menu_t(const elrs_menu_config_t &config) : config_(config) {}

    void reset();

private:
    elrs_menu_config_t config_;
    uint8_t requester_ = 0;
    size_t next_parameter_ = 0;

    friend bool handle_exchange(elrs_menu_t &menu, const uint8_t *frame, size_t frame_size);
};

// Handle one complete CRSF frame. frame_size includes address, length, and CRC.
// The caller remains responsible for CRC validation in its CRSF parser.
bool handle_exchange(elrs_menu_t &menu, const uint8_t *frame, size_t frame_size);

#endif
