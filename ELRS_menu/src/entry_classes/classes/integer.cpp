#include "../entr_cls.h"
#include <algorithm>
#include <limits>
#include <cstring>

namespace {
// ---- wire (big-endian) helpers ----
inline void store_u_be(uint8_t* dst, uint64_t v, size_t w) {
    for (size_t i = 0; i < w; ++i) dst[w - 1 - i] = (uint8_t)((v >> (8*i)) & 0xFF);
}
inline void store_s_be(uint8_t* dst, int64_t v, size_t w) { store_u_be(dst, (uint64_t)v, w); }
inline uint64_t load_u_be(const uint8_t* src, size_t w) {
    uint64_t v = 0; for (size_t i = 0; i < w; ++i) v = (v << 8) | src[i]; return v;
}
inline int64_t load_s_be(const uint8_t* src, size_t w) {
    uint64_t u = load_u_be(src, w);
    unsigned bits = (unsigned)(w * 8);
    if (bits < 64 && (u >> (bits - 1)) & 1ULL) u |= (~0ULL) << bits; // sign-extend
    return (int64_t)u;
}

// ---- native (host) read/write helpers ----
inline int64_t read_native_s(const uint8_t* p, size_t w) {
    switch (w) {
        case 1: return *(const int8_t*)p;
        case 2: { int16_t t; std::memcpy(&t, p, 2); return t; }
        case 4: { int32_t t; std::memcpy(&t, p, 4); return t; }
        case 8: { int64_t t; std::memcpy(&t, p, 8); return t; }
        default: return 0;
    }
}
inline uint64_t read_native_u(const uint8_t* p, size_t w) {
    switch (w) {
        case 1: return *(const uint8_t*)p;
        case 2: { uint16_t t; std::memcpy(&t, p, 2); return t; }
        case 4: { uint32_t t; std::memcpy(&t, p, 4); return t; }
        case 8: { uint64_t t; std::memcpy(&t, p, 8); return t; }
        default: return 0;
    }
}
inline void write_native_s(uint8_t* p, int64_t v, size_t w) {
    switch (w) {
        case 1: *(int8_t*)p  = (int8_t)v;  break;
        case 2: { int16_t t = (int16_t)v; std::memcpy(p, &t, 2); break; }
        case 4: { int32_t t = (int32_t)v; std::memcpy(p, &t, 4); break; }
        case 8: { int64_t t = (int64_t)v; std::memcpy(p, &t, 8); break; }
    }
}
inline void write_native_u(uint8_t* p, uint64_t v, size_t w) {
    switch (w) {
        case 1: *(uint8_t*)p  = (uint8_t)v;  break;
        case 2: { uint16_t t = (uint16_t)v; std::memcpy(p, &t, 2); break; }
        case 4: { uint32_t t = (uint32_t)v; std::memcpy(p, &t, 4); break; }
        case 8: { uint64_t t = (uint64_t)v; std::memcpy(p, &t, 8); break; }
    }
}

// clamps for signed/unsigned ranges by width
inline uint64_t u_max_by_width(size_t w) {
    return (w == 8) ? std::numeric_limits<uint64_t>::max() : ((1ULL << (8*w)) - 1ULL);
}
inline int64_t s_min_by_width(size_t w) {
    return (w == 8) ? std::numeric_limits<int64_t>::min() : -(1LL << ((8*w) - 1));
}
inline int64_t s_max_by_width(size_t w) {
    return (w == 8) ? std::numeric_limits<int64_t>::max() :  ((1LL << ((8*w) - 1)) - 1);
}
} // namespace

// ----- constructors -----
int_obj_t::int_obj_t(void* out_ptr, size_t value_width, bool value_is_signed,
                     int64_t vmin, int64_t vmax, int64_t vdef)
: param_entry_data(static_cast<uint8_t*>(out_ptr))
, width(value_width)
, is_signed(value_is_signed)
{
    if (!(width == 1 || width == 2 || width == 4 || width == 8)) width = 1;

    if (is_signed) {
        if (vmin > vmax) std::swap(vmin, vmax);
        vmin = std::max(s_min_by_width(width), std::min(s_max_by_width(width), vmin));
        vmax = std::max(s_min_by_width(width), std::min(s_max_by_width(width), vmax));
        vdef = std::max(vmin, std::min(vmax, vdef));
        store_s_be(minv, vmin, width);
        store_s_be(maxv, vmax, width);
        store_s_be(defv, vdef, width);
        // initialize host variable if you want deterministic first display:
        write_native_s(out, vdef, width);
    } else {
        uint64_t umin = (vmin < 0) ? 0ULL : (uint64_t)vmin;
        uint64_t umax = (vmax < 0) ? 0ULL : (uint64_t)vmax;
        uint64_t udef = (vdef < 0) ? 0ULL : (uint64_t)vdef;
        if (umin > umax) std::swap(umin, umax);
        const uint64_t ucap = u_max_by_width(width);
        umin = std::min(umin, ucap);
        umax = std::min(umax, ucap);
        udef = std::min(std::max(udef, umin), umax);
        store_u_be(minv, umin, width);
        store_u_be(maxv, umax, width);
        store_u_be(defv, udef, width);
        write_native_u(out, udef, width);
    }
}

int_obj_t::int_obj_t(void* out_ptr, size_t value_width,
                     uint64_t vmin, uint64_t vmax, uint64_t vdef)
: param_entry_data(static_cast<uint8_t*>(out_ptr))
, width(value_width)
, is_signed(false)
{
    if (!(width == 1 || width == 2 || width == 4 || width == 8)) width = 1;
    if (vmin > vmax) std::swap(vmin, vmax);
    const uint64_t ucap = u_max_by_width(width);
    vmin = std::min(vmin, ucap);
    vmax = std::min(vmax, ucap);
    vdef = std::min(std::max(vdef, vmin), vmax);
    store_u_be(minv, vmin, width);
    store_u_be(maxv, vmax, width);
    store_u_be(defv, vdef, width);
    write_native_u(out, vdef, width);
}

size_t int_obj_t::value_width() const { return width; }
size_t int_obj_t::get_size() { return width * 4 + 2; }

// ----- stream to wire (BE) -----
esp_err_t int_obj_t::form_packet(uint8_t* buffer, size_t b_size) {
    if (!buffer || b_size == 0) return ESP_ERR_INVALID_ARG;
    uint8_t frame[4 * MAXW + 2];
    const size_t base = 4 * width;

    // current from host (native) -> wire (BE)
    if (is_signed) {
        int64_t cur = read_native_s(out, width);
        store_s_be(frame + 0*width, cur, width);
    } else {
        uint64_t cur = read_native_u(out, width);
        store_u_be(frame + 0*width, cur, width);
    }

    // precomputed BE min/max/def
    std::memcpy(frame + 1*width, minv, width);
    std::memcpy(frame + 2*width, maxv, width);
    std::memcpy(frame + 3*width, defv, width);

    frame[base + 0] = 0; // decimals
    frame[base + 1] = 0; // unit

    const size_t total  = base + 2;
    const size_t remain = total - written;
    const size_t chunk  = (b_size < remain) ? b_size : remain;

    std::memcpy(buffer, frame + written, chunk);
    written += chunk;

    if (written >= total) { written = 0; return ESP_OK; }
    return ESP_ERR_NOT_FINISHED;
}

// ----- parse write from wire (BE) -> host (native) -----
bool int_obj_t::apply_write(const uint8_t* src, size_t len) {
    if (!src || len < width) return false;

    if (is_signed) {
        int64_t v    = load_s_be(src, width);
        int64_t vmin = load_s_be(minv, width);
        int64_t vmax = load_s_be(maxv, width);
        if (v < vmin) v = vmin;
        if (v > vmax) v = vmax;
        write_native_s(out, v, width);
    } else {
        uint64_t v    = load_u_be(src, width);
        uint64_t vmin = load_u_be(minv, width);
        uint64_t vmax = load_u_be(maxv, width);
        if (v < vmin) v = vmin;
        if (v > vmax) v = vmax;
        write_native_u(out, v, width);
    }
    return true;
}
