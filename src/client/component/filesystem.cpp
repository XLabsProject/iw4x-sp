#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "game_module.hpp"
#include "filesystem.hpp"

namespace filesystem {
namespace {
const char* sys_default_install_path_stub() {
  static auto current_path = std::filesystem::current_path().string();
  return current_path.data();
}
} // namespace

std::vector<std::string> vectored_file_list(const std::string& path,
                                            const std::string& extension) {
  std::vector<std::string> file_list;

  auto num_files = 0;
  const auto** files = game::FS_ListFiles(path.data(), extension.data(),
                                          game::FS_LIST_ALL, &num_files, 10);

  if (files) {
    for (auto i = 0; i < num_files; ++i) {
      if (files[i]) {
        file_list.emplace_back(files[i]);
      }
    }

    game::FS_FreeFileList(files, 10);
  }

  return file_list;
}

std::string get_binary_directory() {
  const auto dir = game_module::get_host_module().get_folder();
  return utils::string::replace(dir, "/", "\\");
}

file::file(std::string name, game::FsThread thread) : name_(std::move(name)) {
  assert(!this->name_.empty());

  int handle{};
  const auto len =
      game::FS_FOpenFileReadForThread(name_.data(), &handle, thread);

  if (!handle) {
    this->valid_ = false;
    return;
  }

  auto* buf = static_cast<char*>(game::Hunk_AllocateTempMemory(len + 1));
  game::FS_Read(buf, len, handle);
  buf[len] = '\0';

  game::FS_FCloseFile(handle);

  this->valid_ = true;
  this->buffer_.append(buf, len);
}

bool file::exists() const { return this->valid_; }

const std::string& file::get_buffer() const { return this->buffer_; }

const std::string& file::get_name() const { return this->name_; }

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x465B90, sys_default_install_path_stub, HOOK_CALL)
        .install()
        ->quick(); // Sys_CreateFile
  }
};
} // namespace filesystem

REGISTER_COMPONENT(filesystem::component)
