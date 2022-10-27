#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/memory.hpp>

#include "filesystem.hpp"

namespace asset_restrict {
namespace {
// IW4x does this
game::XAssetEntry entry_pool[789312];

game::XAssetHeader reallocate_asset_pool(game::XAssetType type,
                                         const int size) {
  const auto entry_size =
      reinterpret_cast<int (*)()>(game::DB_GetXAssetSizeHandlers[type])();
  const game::XAssetHeader pool_entry = {
      utils::memory::allocate(entry_size * size)};
  game::DB_XAssetPool[type] = pool_entry.data;
  game::g_poolSize[type] = size;

  return pool_entry;
}

void load_map_entities(game::MapEnts* entry) {
  char filename[MAX_PATH]{};

  _snprintf_s(filename, _TRUNCATE, "%s.ents", entry->name);

  static std::string map_entities;
  filesystem::file ent_file(filename, game::FS_THREAD_DATABASE);
  if (ent_file.exists()) {
    map_entities = ent_file.get_buffer();
    entry->entityString = map_entities.data();
    entry->numEntityChars = static_cast<int>(map_entities.size()) + 1;
  }
}

void load_asset(game::XAssetType type, void** entry) {
  if (entry) {
    if (type == game::ASSET_TYPE_MAP_ENTS) {
      load_map_entities(static_cast<game::MapEnts*>(*entry));
    }
  }
}

void __declspec(naked) load_asset_stub() {
  __asm {
    pushad;

    push [esp + 0x20 + 0x8];
    push [esp + 0x20 + 0x8];
    call load_asset;
    add esp, 0x8;

    popad;

    sub esp, 0x14;
    mov eax, dword ptr [esp + 0x1C];

    push 0x581EE7;
    ret;
  }
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x581EE0, load_asset_stub, HOOK_JUMP).install()->quick();
    utils::hook::nop(0x581EE5, 2);

    patch_entry_pool_sp();
    reallocate_asset_pool(game::ASSET_TYPE_GAMEWORLD_MP, 1);
  }

  static void patch_entry_pool_sp() {
    // Apply new size
    utils::hook::set<std::uint32_t>(0x581740, sizeof(entry_pool) /
                                                  sizeof(game::XAssetEntry));
    utils::hook::set<game::XAssetEntry*>(0x581721, entry_pool + 1);
    utils::hook::set<game::XAssetEntry*>(0x581732, entry_pool + 1);

    utils::hook::signature signature(0x411000, 0x200000);

    signature.add({"\x60\xB3\xB2\x00", "xxxx", [](char* address) {
                     utils::hook::set<game::XAssetEntry*>(address, entry_pool);
                   }});

    signature.process();
  }
};
} // namespace asset_restrict

REGISTER_COMPONENT(asset_restrict::component)
