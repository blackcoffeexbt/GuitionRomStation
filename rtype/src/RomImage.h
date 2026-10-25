#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
namespace port {
constexpr size_t MaxRomBytes = 1024 * 1024;
struct RomLayout {
  size_t offset = 0, payload = 0, mapped = 0;
  const char *error = nullptr;
  explicit operator bool() const { return error == nullptr; }
};
inline RomLayout romLayout(size_t bytes) {
  RomLayout out;
  out.offset = bytes % 8192 == 512 ? 512 : 0;
  out.payload = bytes - out.offset;
  if (out.payload < 8192 || out.payload > MaxRomBytes || out.payload % 8192) {
    out.error = "ROM must be 8 KB to 1 MB, in 8 KB units";
    return out;
  }
  // Bank windows are 16 KB. Only an 8 KB image may need mirroring.
  if (out.payload > 8192 && out.payload % 16384) {
    out.error = "ROM banks must be 16 KB aligned";
    return out;
  }
  out.mapped = out.payload < 16384 ? 16384 : out.payload;
  return out;
}
inline uint8_t romRegion(const uint8_t *data, size_t size) {
  for (size_t at : {size_t(0x7ff0), size_t(0x3ff0), size_t(0x1ff0)})
    if (at + 16 <= size && !std::memcmp(data + at, "TMR SEGA", 8))
      return data[at + 15] >> 4;
  return 0;
}
inline const char *romHeader(const uint8_t *data, size_t size) {
  for (size_t at : {size_t(0x7ff0), size_t(0x3ff0), size_t(0x1ff0)}) {
    if (at + 16 <= size && !std::memcmp(data + at, "TMR SEGA", 8)) {
      uint8_t region = data[at + 15] >> 4;
      if (region == 5 || region == 6 || region == 7)
        return "Game Gear ROMs are not supported";
      if (region != 3 && region != 4)
        return "Not a supported Master System header";
      return nullptr;
    }
  }
  return "Missing Master System cartridge header";
}
inline void mirrorSmallRom(uint8_t *data, const RomLayout &layout) {
  if (layout.payload == 8192)
    std::memcpy(data + 8192, data, 8192);
}
inline bool smsFilename(const char *name) {
  if (!name || name[0] == '.' || !*name)
    return false;
  for (const char *p = name; *p; ++p)
    if (*p == '/' || *p == '\\' || uint8_t(*p) < 32)
      return false;
  size_t n = std::strlen(name);
  return n > 4 && name[n - 4] == '.' && (name[n - 3] == 's' || name[n - 3] == 'S') &&
         (name[n - 2] == 'm' || name[n - 2] == 'M') && (name[n - 1] == 's' || name[n - 1] == 'S');
}
} // namespace port
