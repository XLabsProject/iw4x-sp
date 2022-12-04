#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace assets {
namespace {
utils::hook::detour db_read_raw_file_hook;
utils::hook::detour com_load_info_string_hook;

char* db_read_raw_file_stub(const char* filename, char* buf, int size) {
  auto file_handle = 0;
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

const char* com_load_info_string_fast_file(const char* file_name,
                                           const char* file_desc,
                                           const char* ident,
                                           char* load_buffer) {
  const static DWORD func = 0x602FA0;
  const char* result{};

  __asm {
    pushad;

    mov ebx, load_buffer;
    mov edi, file_name;
    push ident;
    push file_desc;
    call func;
    add esp, 0x8;
    mov result, eax;

    popad;
  }

  return result;
}

const char* com_load_info_string_load_obj(const char* file_name,
                                          const char* file_desc,
                                          const char* ident,
                                          char* load_buffer) {
  int file_handle;

  const auto file_len =
      game::FS_FOpenFileByMode(file_name, &file_handle, game::FS_READ);
  if (file_len < 0) {
    game::Com_DPrintf(game::CON_CHANNEL_SYSTEM,
                      "Could not load %s [%s] as rawfile", file_desc,
                      file_name);
    return nullptr;
  }

  const auto ident_len = static_cast<int>(std::strlen(ident));
  game::FS_Read(load_buffer, ident_len, file_handle);
  load_buffer[ident_len] = '\0';

  if (std::strncmp(load_buffer, ident, ident_len) != 0) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is not a %s\n",
                    file_name, file_desc);
    return nullptr;
  }

  if ((file_len - ident_len) >= 0x4000) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is too long of a %s to parse\n",
                    file_name, file_desc);
    return nullptr;
  }

  game::FS_Read(load_buffer, file_len - ident_len, file_handle);
  load_buffer[file_len - ident_len] = '\0';
  game::FS_FCloseFile(file_handle);

  return load_buffer;
}

const char* com_load_info_string_stub(const char* file_name,
                                      const char* file_desc, const char* ident,
                                      char* load_buffer) {
  const auto* buffer =
      com_load_info_string_load_obj(file_name, file_desc, ident, load_buffer);
  if (!buffer) {
    buffer = com_load_info_string_fast_file(file_name, file_desc, ident,
                                            load_buffer);
  }

  if (!game::Info_Validate(buffer)) {
    game::Com_Error(game::ERR_DROP,
                    "\x15"
                    "File [%s] is not a valid %s\n",
                    file_name, file_desc);
    return nullptr;
  }

  return buffer;
}
} // namespace

class raw_file final : public component_interface {
public:
  void post_load() override {
    // Remove fs_game check for moddable raw files
    // allows non-fs_game to modify raw files
    utils::hook::nop(0x612932, 2);

    db_read_raw_file_hook.create(0x46DA60, &db_read_raw_file_stub);

    com_load_info_string_hook.create(0x42DB20, &com_load_info_string_stub);
  }

  void pre_destroy() override { db_read_raw_file_hook.clear(); }
};
} // namespace assets

REGISTER_COMPONENT(assets::raw_file)
