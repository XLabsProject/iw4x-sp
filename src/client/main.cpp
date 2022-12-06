#include "std_include.hpp"
#include <utils/nt.hpp>
#include <utils/io.hpp>

#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <gsl/gsl>

DECLSPEC_NORETURN void WINAPI exit_hook(const int code) {
  component_loader::pre_destroy();
  std::exit(code);
}

FARPROC load_binary() {
  loader loader;
  utils::nt::library self;

  loader.set_import_resolver(
      [self](const std::string& library, const std::string& function) -> void* {
        if (library == "steam_api.dll") {
          return self.get_proc<FARPROC>(function);
        }
        if (function == "ExitProcess") {
          return exit_hook;
        }

        return component_loader::load_import(library, function);
      });

  std::string data;
  if (!utils::io::read_file("iw4sp.exe", &data)) {
    throw std::runtime_error(
        "Failed to read game binary (iw4sp.exe)!\nPlease select the correct "
        "path in the launcher settings.");
  }

  return loader.load(self, data);
}

void enable_dpi_awareness() {
  const utils::nt::library user32{"user32.dll"};
  const auto set_dpi =
      user32 ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
                   "SetProcessDpiAwarenessContext")
             : nullptr;
  if (set_dpi) {
    set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  }
}

void apply_environment() {
  wchar_t* buffer{};
  std::size_t size{};
  if (_wdupenv_s(&buffer, &size, L"XLABS_MW2_INSTALL") != 0 ||
      buffer == nullptr) {
    throw std::runtime_error("Please use the X Labs launcher to run the game!");
  }

  const auto _ = gsl::finally([&] { std::free(buffer); });

  SetCurrentDirectoryW(buffer);
  SetDllDirectoryW(buffer);
}

int main() {
  FARPROC entry_point;
  enable_dpi_awareness();

  std::srand(std::uint32_t(time(nullptr)) ^
             ~(GetTickCount() * GetCurrentProcessId()));

  {
    auto premature_shutdown = true;
    const auto _0 = gsl::finally([&premature_shutdown] {
      if (premature_shutdown) {
        component_loader::pre_destroy();
      }
    });

    try {
      apply_environment();

      if (!component_loader::post_start())
        return 0;

      entry_point = load_binary();
      if (!entry_point) {
        throw std::runtime_error("Unable to load binary into memory");
      }

      if (!component_loader::post_load())
        return 0;

      premature_shutdown = false;
    } catch (const std::exception& ex) {
      MessageBoxA(nullptr, ex.what(), "ERROR", MB_ICONERROR);
      return 1;
    }
  }

  return entry_point();
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int) { return main(); }
