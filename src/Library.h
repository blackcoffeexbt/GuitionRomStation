#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <cstdio>
namespace port {
constexpr int LibraryCapacity = 128, LibraryNameBytes = 192, LibraryRows = 5;
struct RomEntry {
  char name[LibraryNameBytes]{};
  uint32_t bytes = 0;
};
struct LibraryView {
  char rows[LibraryRows][56]{};
  char sizes[LibraryRows][12]{};
  char message[88]{};
  char folder[24]{};
  int selected = 0, first = 0, count = 0;
  bool hasGame = false, loading = false;
  uint32_t revision = 0;
};
class LibraryModel {
  uint8_t previous = 0;
  uint32_t repeatAt = 0;

public:
  enum Button : uint8_t {
    Up = 1,
    Down = 2,
    Left = 4,
    Right = 8,
    Accept = 16,
    Back = 32,
    Refresh = 64
  };
  enum class Action { None, Load, Close, Scan };
  int selected = 0, count = 0;
  void open(uint8_t held) {
    previous = held;
    repeatAt = 0;
  }
  int first() const { return selected / LibraryRows * LibraryRows; }
  void move(int delta) {
    if (count) {
      selected = (selected + delta) % count;
      if (selected < 0)
        selected += count;
    } else
      selected = 0;
  }
  Action update(uint8_t buttons, uint32_t now) {
    uint8_t edge = buttons & ~previous;
    uint8_t directions = buttons & 15;
    bool repeat =
        directions && directions == (previous & 15) && repeatAt && int32_t(now - repeatAt) >= 0;
    if (directions && (edge & 15 || repeat)) {
      move(directions & Up     ? -1
           : directions & Down ? 1
           : directions & Left ? -LibraryRows
                               : LibraryRows);
      repeatAt = now + (repeat ? 110 : 350);
    } else if (!directions)
      repeatAt = 0;
    previous = buttons;
    if (edge & Accept)
      return count ? Action::Load : Action::None;
    if (edge & Back)
      return Action::Close;
    if (edge & Refresh)
      return Action::Scan;
    return Action::None;
  }
  int touchRow(int y) const {
    int row = (y - 64) / 38;
    return y >= 64 && y < 254 && row < LibraryRows && first() + row < count ? first() + row : -1;
  }
};
} // namespace port
