#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace raw_file {
namespace {
utils::hook::detour db_read_raw_file_hook;

char* db_read_raw_file_stub(const char* filename, char* buf, int size) {
  int file_handle{};
  auto file_size = game::FS_FOpenFileRead(filename, &file_handle);

  if (file_handle != 0) {
    if ((file_size + 1) <= size) {
      game::FS_Read(buf, file_size, file_handle);
      buf[file_size] = '\0';
      game::FS_FCloseFile(file_handle);
      return buf;
    }

    game::FS_FCloseFile(file_handle);
    game::Com_PrintError(
        game::CON_CHANNEL_ERROR,
        "Ignoring raw file '%s' as it exceeds buffer size %i > %i\n", filename,
        file_size, size);
  }

  auto* rawfile =
      game::DB_FindXAssetHeader(game::ASSET_TYPE_RAWFILE, filename).rawfile;
  if (game::DB_IsXAssetDefault(game::ASSET_TYPE_RAWFILE, filename)) {
    return nullptr;
  }

  game::DB_GetRawBuffer(rawfile, buf, size);
  return buf;
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    // Remove fs_game check for moddable rawfiles - allows non-fs_game to modify
    // rawfiles
    utils::hook::nop(0x612932, 2);

    db_read_raw_file_hook.create(0x46DA60, &db_read_raw_file_stub);
  }

  void pre_destroy() override { db_read_raw_file_hook.clear(); }
};
} // namespace raw_file

REGISTER_COMPONENT(raw_file::component)
