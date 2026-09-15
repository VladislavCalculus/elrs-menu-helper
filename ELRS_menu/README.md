ELRS Menu is a C++17 library that serves CRSF parameter-menu requests from ELRS Lua.

Include `<elrs_menu/elrs_menu.hpp>`, provide a transport callback, create an `elrs_menu_t`, and give each complete received CRSF frame to `handle_exchange()`.

```cpp
void write_crsf(const uint8_t *data, size_t size, void *context) {
    static_cast<MyUart *>(context)->write(data, size);
}

param_entry_t parameters[] = {
    {"Mode", &mode_entry, PARENT_ENTRY_ROUTE_GLOBAL, CRSF_PARAM_TYPE_TEXT_SELECTION},
};

elrs_menu_config_t config{};
config.device_info = {"My device", 123, 1, 1, 1};
config.parameters = parameters;
config.parameter_count = sizeof(parameters) / sizeof(parameters[0]);
config.write = write_crsf;
config.write_context = &uart;
elrs_menu_t menu{config};

// frame_size is the actual number of received bytes, including address, length and CRC.
const bool was_menu_frame = handle_exchange(menu, frame, frame_size);
```

There is no terminator entry and no global exchange state. `crsf_menu_addresses_t` defaults to the usual ELRS topology but allows the outer transport destination, device address, and Lua address to be changed.
