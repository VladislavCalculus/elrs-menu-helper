This is ELRS menu handling library.
The main purpose of it is to handle exchange between ELRS menu lua script and your device.
Configuration of handler goes as follows:

1) create ping response structure
2) create N number of parameters, than equals to your number of parameters passed into ping response.
NOTE: always end this array with `ARR_TERMINATOR`.
3) create lua_elrs_menu_config_t
4) Use handle_exchange function: pass CRSF uint8_t buffer (before unpacking into 16 channels) and your lua_elrs_menu_config_t

more detailed information can be found in `how2use.md`