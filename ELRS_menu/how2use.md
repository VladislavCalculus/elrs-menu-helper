# Using ELRS Menu

Include the C++17 public header:

```cpp
#include <elrs_menu/elrs_menu.hpp>
```

The library owns the ELRS parameter protocol. Your application supplies two things: the parameter data to expose and a callback that transmits complete outgoing CRSF frames.

## Setup

1. Create the value objects (`info_obj_t`, `int_obj_t`, `text_select_obj_t`, or `command_obj_t`).
2. Place their descriptions in a regular `param_entry_t` array.
3. Configure device identity, the array, its explicit element count, and one transport callback in `elrs_menu_config_t`.
4. Construct one `elrs_menu_t` for that configuration.
5. Once the CRSF receive parser has assembled and CRC-checked a complete frame, call `handle_exchange(menu, frame, frame_size)`.

There is no `ARR_TERMINATOR`, no automatic parameter-count scan, and no per-entry transport callback.

## Device information

`crsf_device_info_t` is returned for a CRSF device-ping request:

```cpp
crsf_device_info_t info{"My device", 0x12345678, 1, 1, 1};
```

Its fields are device name, serial number, hardware version, software version, and parameter protocol version. The parameter count is derived from `elrs_menu_config_t::parameter_count`.

## Parameters

Each entry has four fields:

```cpp
param_entry_t entry{"Name", &value, PARENT_ENTRY_ROUTE_GLOBAL,
                    CRSF_PARAM_TYPE_INFO};
```

- `name`: label shown by the radio.
- `value`: a matching parameter-value object; folders use `nullptr`.
- `entry_root`: `PARENT_ENTRY_ROUTE_GLOBAL`, or the one-based index of a folder entry.
- `param_type`: the CRSF type of this entry.

Supported types are `UINT8`, `INT8`, `UINT16`, `INT16`, `TEXT_SELECTION`, `FOLDER`, `INFO`, and `COMMAND`. Editable CRSF string parameters are not currently implemented.

### Value objects

```cpp
info_obj_t info_value{"Read-only text", sizeof("Read-only text")};

uint8_t mode = 0;
text_select_obj_t mode_value{&mode, "Normal;Sport;", sizeof("Normal;Sport;")};

int16_t limit = 100;
int_obj_t limit_value{&limit, sizeof(limit), true, -500, 500, 100};
```

For a command, the event handler runs synchronously while processing a received CRSF frame. It must change state or schedule work, then return; it must not block.

```cpp
void reset_event(command_obj_t &command, command_type_e event) {
    if (event == CMD_CLICK) {
        command.type = CDM_RESP_EXECUTING;
        // Queue work or set an application state flag here.
    }
}

command_obj_t reset_value{"Resetting", sizeof("Resetting"), 200, reset_event};
```

## Transport and configuration

The write callback receives a complete CRSF frame. `context` lets the application attach any UART, queue, driver, or object without making the library platform-specific.

```cpp
void write_crsf(const uint8_t *data, size_t size, void *context) {
    static_cast<MyTransport *>(context)->write(data, size);
}

param_entry_t parameters[] = {
    {"Status", &info_value, PARENT_ENTRY_ROUTE_GLOBAL, CRSF_PARAM_TYPE_INFO},
    {"Mode", &mode_value, PARENT_ENTRY_ROUTE_GLOBAL, CRSF_PARAM_TYPE_TEXT_SELECTION},
    {"Limit", &limit_value, PARENT_ENTRY_ROUTE_GLOBAL, CRSF_PARAM_TYPE_INT16},
    {"Reset", &reset_value, PARENT_ENTRY_ROUTE_GLOBAL, CRSF_PARAM_TYPE_COMMAND},
};

elrs_menu_config_t config{};
config.device_info = {"My device", 0x12345678, 1, 1, 1};
config.parameters = parameters;
config.parameter_count = sizeof(parameters) / sizeof(parameters[0]);
config.write = write_crsf;
config.write_context = &transport;

elrs_menu_t menu{config};
```

`config.addresses` defaults to the conventional ELRS setup. For unusual CRSF routing, configure `transport_destination`, `device_address`, and `menu_address` before constructing the menu.

## Receiving frames

`frame_size` is the exact number of received bytes, including outer address, length, and CRC. It lets the library reject truncated frames safely. The caller still performs CRC validation and stream assembly.

```cpp
if (crsf_frame_has_valid_crc(frame, received_size)) {
    const bool handled = handle_exchange(menu, frame, received_size);
    if (handled) {
        // This was addressed to this menu instance.
    }
}
```

Use a separate `elrs_menu_t` per independently configured menu device. Each instance holds its own parameter-read progress.
