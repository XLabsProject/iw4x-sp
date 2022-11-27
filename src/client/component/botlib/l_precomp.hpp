#pragma once

namespace pc {
int load_source_handle(const char* filename, const char** builtin_defines);
int free_source_handle(int handle);
} // namespace pc
