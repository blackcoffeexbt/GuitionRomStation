#pragma once
#include "RomImage.h"
#include <algorithm>
namespace port {
struct LoadedRom {
  uint8_t *data = nullptr;
  size_t bytes = 0;
  const char *error = nullptr;
};
using RomProgress = void (*)(size_t, size_t);
template <class Reader, class Allocate, class Release>
LoadedRom readRom(Reader &file, size_t fileBytes, Allocate allocate, Release release,
                  RomProgress progress = nullptr) {
  auto layout = romLayout(fileBytes);
  if (!layout)
    return {nullptr, 0, layout.error};
  auto *data = static_cast<uint8_t *>(allocate(layout.mapped));
  if (!data)
    return {nullptr, 0, "Not enough PSRAM; current game is kept"};
  bool ok = file.seek(layout.offset);
  size_t done = 0;
  while (ok && done < layout.payload) {
    size_t wanted = std::min(size_t(4096), layout.payload - done);
    if (file.read(data + done, wanted) != wanted) {
      ok = false;
      break;
    }
    done += wanted;
    if (progress)
      progress(done, layout.payload);
  }
  const char *error = ok ? romHeader(data, layout.payload) : "Read failed; current game is kept";
  if (error) {
    release(data);
    return {nullptr, 0, error};
  }
  mirrorSmallRom(data, layout);
  return {data, layout.mapped, nullptr};
}
} // namespace port
