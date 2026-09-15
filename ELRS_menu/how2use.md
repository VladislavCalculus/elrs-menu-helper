# HOW TO USE:

* [Setup](#setup)
* [Ping response](#ping-response)
* [Parameter entry](#parameter-entry)
* [Menu config](#menu-config)

## Setup

To handle the ELRS menu in your program, follow four simple steps to configure it and communicate with the remote.

1. Create a [Ping response](#ping-response) object.
2. Create [Parameter entry](#parameter-entry) objects—each representing its own field in the ELRS menu—then place them into an array, ending it with `ARR_TERMINATOR` (otherwise, the automatic parameter counter will fail).
3. Create a [Menu config](#menu-config) object, putting in the device address you need, the [Ping response](#ping-response), and the array of [Parameter entry](#parameter-entry).
4. Call the function `handle_exchange`, passing the CRSF buffer (NOTE: before unpacking it) and your [Menu config](#menu-config) object.

Now, the ELRS menu will be handled automatically.
[Back to top](#how-to-use)

## Ping response

Ping response (`crsf_ping_responce_t`) is a structure used to respond to the ELRS menu “PING” command. Its purpose is to provide the device name, serial number, `hwver`, `swver`, and to define the number and type of parameters. The structure is as follows:

```c
const char *name;
uint32_t serial;
uint32_t hwver;
uint32_t swver;
uint8_t param_count;
uint8_t param_proto;
void (*w_func)(uint8_t *buffer, size_t size);
```

Now, about each field:

* `name` — the device name displayed on the top row of the ELRS menu.
* `serial` — a unique number for this physical unit. Lets the radio/cache distinguish different devices (e.g., two identical receivers).
* `hwver` — the hardware revision of the device.
* `swver` — the software/firmware version that the device is currently running.
* `param_count` — the number of parameters that will be sent to the ELRS menu. (NOTE: this can be written as 0 when using the automatic counter in this handler.)
* `param_proto` — tells the radio/Lua script which parameter framing rules your device uses so it can parse your `PARAMETER_*` frames correctly.

  * v1 (baseline): classic layouts (8/16-bit ints, strings, text selections with the simple tail), no trailing `[decimals, unit]` on numbers.
  * v2 (newer): adds numeric meta bytes (decimals & unit), and commonly 32/64-bit ints, etc.
* `w_func` — the function used to write data to the ELRS menu. This allows writing not only via UART, but also via other transmitting methods.

[Back to top](#how-to-use)

## Parameter entry

* [Parameter types](#parameter-types)
* [Parameter entry data](#parameter-entry-data)

A parameter entry (`param_entry_t`) is a struct that fully describes all the necessary information for the ELRS menu to display and use the provided parameter.

```c
const char *name;
param_entry_data *value;
uint8_t entry_root;
crsf_param_type_e param_type;
void (*w_func)(uint8_t* buffer, size_t size);
```

* `name` — the name of the parameter displayed to the left of its value in the menu.
* `value` — the [value](#parameter-entry-data), which is a child of the abstract class `param_entry_data`. It is used to create packets and store the value depending on the entry type. The class should be selected according to the [parameter type](#parameter-types).
* `entry_root` — the folder where the parameter should reside. Use `PARENT_ENTRY_ROUTE_GLOBAL` to display it on the main screen, or put the folder index to place it in a folder (the folder index is its index in the `param_entry_t` array plus 1).
* `param_type` — select the [type](#parameter-types) of the entry you want to use by manually specifying its type here.
* `w_func` — the function used to write data to the ELRS menu. This allows writing not only via UART, but also via other transmitting methods.

[Back to top](#how-to-use)

##### Parameter types

```
CRSF_PARAM_TYPE_UINT8
CRSF_PARAM_TYPE_INT8
CRSF_PARAM_TYPE_UINT16
CRSF_PARAM_TYPE_INT16
CRSF_PARAM_TYPE_TEXT_SELECTION
CRSF_PARAM_TYPE_FOLDER
CRSF_PARAM_TYPE_COMMAND
CRSF_PARAM_TYPE_INFO
```

[Return to parameter entry](#parameter-entry)

##### Parameter entry data

1. **Info**
   A value type used to display plain text. It cannot be interacted with by the user.

```c
info_obj_t(const char *text, size_t t_size);
```

* `text` — the text displayed in the value field.
* `t_size` — the size of the `text` parameter (`sizeof()`).

2. **Integer**
   A value type used to work with integers of different sizes.
   (NOTE: only 8/16-bit integers are supported for now.)

```c
int_obj_t(void* out_ptr, size_t value_width, uint64_t vmin, uint64_t vmax, uint64_t vdef);
```

* `out_ptr` — pointer to the value where the selected number will be written.
* `value_width` — the number of bytes this class should represent (e.g., 1 for uint8/int8, 2 for uint16/int16, …).
* `vmin` — minimum value the user can select on the remote.
* `vmax` — maximum value the user can select on the remote.
* `vdef` — the value displayed when the ELRS menu opens. (NOTE: using `out_ptr` will always display the last selected value.)

3. **Text select**
   A value type that allows selecting one of N options from a predefined list.

```c
text_select_obj_t(uint8_t* out_ptr, const char* options, size_t options_size);
```

* `out_ptr` — an unsigned integer where the output will be written. The output is the index of the selected option.
* `options` — the selectable options, passed as a string where each option is separated by `;`. Example: `"Ready;Not Ready;"`.
* `options_size` — the size of the `options` parameter (`sizeof()`).

4. **Command**
   A value type that displays a button in the ELRS menu. Menu events are passed directly to its handler; the handler must return quickly and must not block or wait for confirmation.

```c
command_obj_t(const char *text, size_t t_size, uint8_t timeout,
              void (*handler)(command_obj_t &obj, command_type_e event));
```

* `text` — the text displayed in the confirmation window if used.
* `t_size` — the size of the `text` parameter (`sizeof()`).
* `timeout` — the menu timeout value.
* `handler` — called immediately for a command event sent by the Lua script. This function has the following parameters:

  * `obj` — the object of this class; allows you to change variables to update displayed text (`obj.change_description(...)`) and type (`obj.type =`).
  * `event` — the event sent by the menu. On `CMD_CLICK`, schedule or flag application work and return. On later `CMD_QUERY` events, publish the current status through `obj`.

The menu library does not create a task or thread. Long-running work belongs to the application: queue it to the platform scheduler or advance it from the application loop/state machine.

There are different command entry types that can be used, each giving context info to the ELRS menu. Types sent by the ELRS menu are handled automatically (they start with `CMD_*`). Your function uses types with `CDM_RESP_*` to send status back to the menu.

* `CDM_RESP_IDLE` — command is **not** running.
* `CMD_CLICK` — user requested to start a command; schedule work and return immediately.
* `CDM_RESP_EXECUTING` — working on the task. Should be active while executing the command (baseline if no other type is required).
* `CDM_RESP_CONFIRM` — opens a confirmation window in the ELRS menu.
* `CMD_CONFIRMED` — user pressed **Confirm**.
* `CMD_CANCEL` — user pressed **Cancel**.
* `CMD_QUERY` — ELRS menu requests an update to avoid timing out.

[Return to parameter entry](#parameter-entry)

## Menu config

Menu config (`lua_elrs_menu_config_t`) is passed to the `handle_exchange()` function to provide the necessary info to handle the ELRS menu on the remote.

```c
uint8_t address;
crsf_ping_responce_t *ping_responce;
param_entry_t *prameters_list;
```

* `address` — the address of your device. If there is no TX, you can use `CRSF_ADDRESS_CRSF_TRANSMITTER`, or, if there is, you can put whatever you desire and the device can be found in **Other devices**.
* `ping_responce` — the [Ping response](#ping-response).
* `prameters_list` — the array of [Parameter entry](#parameter-entry) objects that ends with `ARR_TERMINATOR`.

[Back to top](#how-to-use)
