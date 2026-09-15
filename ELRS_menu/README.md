# ELRS Menu

A small, platform-independent C++17 library for exposing a device configuration menu through the ExpressLRS / CRSF parameter protocol.

It handles device discovery, parameter enumeration, parameter reads and writes, multi-frame parameter replies, and command events. It does not depend on ESP-IDF, FreeRTOS, Arduino, or a particular UART driver.

## Features

- CRSF device-ping and device-information replies
- ELRS parameter-menu enumeration
- `uint8_t`, `int8_t`, `uint16_t`, and `int16_t` parameters
- Text selections, read-only information, folders, and commands
- Non-blocking command-event callbacks
- One menu instance per device/configuration; no global exchange state
- Configurable CRSF routing addresses
- Caller-provided write callback for any UART, queue, socket, or transport

## Requirements

- C++17 compiler
- A CRSF receive parser that assembles complete frames and validates their CRC
- A transport function that can transmit a complete CRSF frame

## Add to a CMake project

### As a subdirectory

```cmake
add_subdirectory(path/to/ELRS_menu)
target_link_libraries(my_firmware PRIVATE ELRS_menu::ELRS_menu)
```

### Install and find the package

```sh
cmake -S . -B build
cmake --install build --prefix /your/prefix
```

```cmake
find_package(ELRS_menu CONFIG REQUIRED)
target_link_libraries(my_firmware PRIVATE ELRS_menu::ELRS_menu)
```

The public header is:

```cpp
#include <elrs_menu/elrs_menu.hpp>
```

## Quick start

Create value objects and describe them with a normal array. There is no sentinel/terminator entry: provide the exact number of parameters.

```cpp
#include <elrs_menu/elrs_menu.hpp>

struct CrsfTransport {
    void write(const uint8_t *data, size_t size);
};

void write_crsf(const uint8_t *data, size_t size, void *context) {
    static_cast<CrsfTransport *>(context)->write(data, size);
}

uint8_t flight_mode = 0;
const char modes[] = "Normal;Sport;";
text_select_obj_t mode_value{&flight_mode, modes, sizeof(modes)};

const char status[] = "Ready";
info_obj_t status_value{status, sizeof(status)};

param_entry_t parameters[] = {
    {"Mode", &mode_value, PARENT_ENTRY_ROUTE_GLOBAL,
     CRSF_PARAM_TYPE_TEXT_SELECTION},
    {"Status", &status_value, PARENT_ENTRY_ROUTE_GLOBAL,
     CRSF_PARAM_TYPE_INFO},
};

CrsfTransport crsf_transport;

elrs_menu_config_t config{};
config.device_info = {"My Device", 0x12345678, 1, 1, 1};
config.parameters = parameters;
config.parameter_count = sizeof(parameters) / sizeof(parameters[0]);
config.write = write_crsf;
config.write_context = &crsf_transport;

elrs_menu_t menu{config};
```

Feed every complete, CRC-valid incoming CRSF frame to the menu instance:

```cpp
void on_crsf_frame(const uint8_t *frame, size_t frame_size) {
    if (handle_exchange(menu, frame, frame_size)) {
        // The frame was handled by this menu instance.
    }
}
```

`frame_size` must be the actual number of available bytes, including the outer address, CRSF length byte, payload, and CRC. The library checks that the CRSF length field fits within this buffer before inspecting request data. CRC validation remains the receive parser's responsibility.

## Commands

Command callbacks execute synchronously from `handle_exchange()`. They must return quickly: use them to set state or enqueue application work, not to perform long operations.

```cpp
void reset_handler(command_obj_t &command, command_type_e event) {
    if (event == CMD_CLICK) {
        command.type = CDM_RESP_EXECUTING;
        // Queue the reset operation, then return.
    }
}

command_obj_t reset_value{"Resetting", sizeof("Resetting"), 200, reset_handler};
```

When the radio sends `CMD_QUERY`, update `command.type` and its description to report the current state.

## CRSF routing

The defaults target the conventional ELRS topology. Set these before constructing `elrs_menu_t` when your routing differs:

```cpp
config.addresses.transport_destination = CRSF_ADDRESS_RADIO_TRANSMITTER;
config.addresses.device_address = CRSF_ADDRESS_CRSF_TRANSMITTER;
config.addresses.menu_address = CRSF_ADDRESS_ELRS_LUA;
```

## Supported parameter types

| Type | Value object |
| --- | --- |
| `CRSF_PARAM_TYPE_UINT8`, `INT8`, `UINT16`, `INT16` | `int_obj_t` |
| `CRSF_PARAM_TYPE_TEXT_SELECTION` | `text_select_obj_t` |
| `CRSF_PARAM_TYPE_INFO` | `info_obj_t` |
| `CRSF_PARAM_TYPE_COMMAND` | `command_obj_t` |
| `CRSF_PARAM_TYPE_FOLDER` | `nullptr` |

Editable CRSF string parameters are not implemented.

## More detail

See [how2use.md](how2use.md) for a fuller API guide and [class_examples.txt](class_examples.txt) for a complete example.

## License

This project is released under the [MIT License](LICENSE). You may use, modify,
and distribute it, including in commercial projects, provided that the copyright
and license notice are retained. It is provided without warranty.
