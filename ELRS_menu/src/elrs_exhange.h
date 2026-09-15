#ifndef __elrs_exhange__
#define __elrs_exhange__

#include "stdint.h"
#include "driver/uart.h"
#include <string.h>
#include "user_dfn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PACKET_SIZE 64
#define MAX_PAYLOAD_SIZE 56

#define WIRE_LITTLE_ENDIAN 0

#if WIRE_LITTLE_ENDIAN
#  define NEED_SWAP (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#else
#  define NEED_SWAP (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif

#define CRSF_TYPE_DEVICE_PING                     0x28
#define CRSF_TYPE_DEVICE_INFO                     0x29

#define CRSF_ADDR_TX                              0xEA
#define CRSF_BROADCAST                            0xEE
#define CRSF_LEN_MAX                               62

#define CRSF_FRAMETYPE_LINK_STATICTICS            0x14
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED         0x16
#define CRSF_FRAMETYPE_DEVICE_PING                0x28
#define CRSF_FRAMETYPE_DEVICE_INFO                0x29
#define CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY   0x2B
#define CRSF_FRAMETYPE_PARAMETER_READ             0x2C
#define CRSF_FRAMETYPE_PARAMETER_WRITE            0x2D

#define PRINT_BITS(x) do { \
    for (int i = sizeof(x)*8 - 1; i >= 0; i--) { \
        printf("%d", ((x) >> i) & 1); \
    } \
    printf("\n"); \
} while(0)

#define OUT_EXT_ADDR CRSF_ADDRESS_CRSF_TRANSMITTER
void crsf_device_ping_response(uint8_t ping_extsrc, crsf_ping_responce_t *r_cfg, param_entry_t e_cfg[]);
void crsf_send_param_entry_reply(uint8_t entry_id, uint8_t req_extsrc, param_entry_t e_cfg);
void crsf_device_write(uint8_t packet[], param_entry_t *e_cfg);

#ifdef __cplusplus
}
#endif

#endif // __elrs_exhange__
