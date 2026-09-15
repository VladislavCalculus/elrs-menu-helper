#ifndef __entr_cls__
#define __entr_cls__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <cstring>
#include <esp_err.h>
#include "stdint.h"

struct param_entry_data {
    uint8_t *out;

    param_entry_data(uint8_t *outer_v) : out(outer_v) {}
    virtual ~param_entry_data() = default;
    virtual size_t get_size() = 0;
    virtual esp_err_t form_packet(uint8_t buffer[], size_t b_size) = 0;
};

class text_select_obj_t final: public param_entry_data {
    private:
        const uint8_t *selection = nullptr;
        size_t str_len = 0;
        size_t written = 0;
        static constexpr size_t TAIL_LEN = 5;
        static uint8_t count_options(const char* s, size_t n);
        uint8_t n_options = 0;
        inline uint8_t tail_byte(size_t idx) const;
    public:
        text_select_obj_t(uint8_t* out_ptr, const char* options, size_t options_size);
        size_t get_size() override;
        esp_err_t form_packet(uint8_t* buffer, size_t b_size) override;
        void drop_written_counter();
};

class info_obj_t final: public param_entry_data {
    private:
        const char *info;
        size_t info_size;
        size_t written = 0;
        
    public:
        info_obj_t(const char *text, size_t t_size);
        size_t get_size() override;
        esp_err_t form_packet(uint8_t* buffer, size_t b_size) override;
        void change_info_text(const char *text, size_t t_size);
};

typedef enum {
    CDM_RESP_IDLE = 0x00,
    CMD_CLICK = 0x01,
    CDM_RESP_EXECUTING = 0x02,
    CDM_RESP_CONFIRM = 0x03,
    CMD_CONFIRMED = 0x04,
    CMD_CANCEL = 0x05,
    CMD_QUERY = 0x06
} command_type_e;

class command_obj_t final: public param_entry_data {
    private:
        using ExecFn = void (*)(command_obj_t &obj, bool *confirmed, bool *canceled);
        ExecFn executable = nullptr;

        uint8_t timeout = 200;
        uint8_t *description = nullptr;
        size_t desc_size;

        size_t written = 0;
        void drop_written_counter();
    
    public:
        
        command_type_e type = CDM_RESP_IDLE;

        command_obj_t(const char *text, size_t t_size, uint8_t _timeout, ExecFn execution_func);
        size_t get_size() override;
        esp_err_t form_packet(uint8_t* buffer, size_t b_size) override;
        void execute(bool *confirmed, bool *canceled);
        void change_description(const char *text, size_t t_size);
};

class int_obj_t final : public param_entry_data {
    private:
        static constexpr size_t MAXW = 8;

        size_t width;
        bool is_signed;

        uint8_t minv[MAXW]{};
        uint8_t maxv[MAXW]{};
        uint8_t defv[MAXW]{};

        size_t written = 0;
    public:
        // Signed constructor
        int_obj_t(void* out_ptr, size_t value_width, bool value_is_signed, int64_t vmin, int64_t vmax, int64_t vdef);
        int_obj_t(void* out_ptr, size_t value_width, uint64_t vmin, uint64_t vmax, uint64_t vdef);
        size_t value_width() const;
        size_t get_size() override;
        esp_err_t form_packet(uint8_t* buffer, size_t b_size) override;
        bool apply_write(const uint8_t* src, size_t len);
};

#ifdef __cplusplus
}
#endif

#endif // __entr_cls__