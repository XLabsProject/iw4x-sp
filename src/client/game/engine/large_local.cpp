#include <std_include.hpp>

#include "large_local.hpp"

namespace game::engine {
namespace {
constexpr auto PAGE_SIZE = 4096;

int can_use_server_large_local() { return Sys_IsServerThread(); }

void large_local_end(int start_pos) {
  assert(Sys_IsMainThread());
  assert(g_largeLocalBuf);

  *g_largeLocalPos = start_pos;

  assert(((*g_maxLargeLocalPos + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1)) <=
         (*g_minLargeLocalRightPos & ~(PAGE_SIZE - 1)));
}

void large_local_end_right(int start_pos) {
  assert(can_use_server_large_local());
  assert(g_largeLocalBuf);

  *g_largeLocalRightPos = start_pos;

  assert(((*g_maxLargeLocalPos + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1)) <=
         (*g_minLargeLocalRightPos & ~(PAGE_SIZE - 1)));
}

void* large_local_get_buf(int start_pos, int size) {
  assert(Sys_IsMainThread() || can_use_server_large_local());
  assert(g_largeLocalBuf);
  assert(!(size & 127));

  if (Sys_IsMainThread()) {
    return &g_largeLocalBuf[start_pos];
  }

  const auto start_index = start_pos - size;
  assert(start_index >= 0);

  return &g_largeLocalBuf[start_index];
}
} // namespace

large_local::large_local(int size_param) {
  assert(size_param);
  assert(Sys_IsMainThread() || can_use_server_large_local());

  size_param = ((size_param + (128 - 1)) & ~(128 - 1));

  if (Sys_IsMainThread()) {
    this->start_pos_ = LargeLocalBegin(size_param);
  } else {
    this->start_pos_ = LargeLocalBeginRight(size_param);
  }

  this->size_ = size_param;
}

large_local::~large_local() {
  if (this->size_) {
    this->pop_buf();
  }
}

void large_local::pop_buf() {
  assert(this->size_);
  assert(Sys_IsMainThread() || can_use_server_large_local());

  if (Sys_IsMainThread()) {
    large_local_end(this->start_pos_);
  } else {
    large_local_end_right(this->start_pos_);
  }

  this->size_ = 0;
}

void* large_local::get_buf() const {
  assert(this->size_);
  assert(Sys_IsMainThread() || can_use_server_large_local());

  return large_local_get_buf(this->start_pos_, this->size_);
}
} // namespace game::engine
