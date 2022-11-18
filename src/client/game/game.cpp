#include <std_include.hpp>

namespace game {
int FS_FOpenFileReadForThread(const char* filename, int* file,
                              FsThread thread) {
  const static DWORD func = 0x630380;
  int result{};

  __asm {
    pushad;

    mov eax, file;
    push thread;
    push filename;
    call func;
    add esp, 0x8;
    mov result, eax;

    popad;
  }

  return result;
}

void IN_KeyDown(kbutton_t* b) {
  const static DWORD func = 0x57A350;

  __asm {
    pushad;

    mov esi, b;
    call func;

    popad;
  }
}

void IN_KeyUp(kbutton_t* b) {
  const static DWORD func = 0x57A3F0;

  __asm {
    pushad;

    mov esi, b;
    call func;

    popad;
  }
}

bool ScrPlace_IsFullScreenActive() {
  return *activeScreenPlacementMode == SCRMODE_FULL;
}

ScreenPlacement* ScrPlace_GetUnsafeFullPlacement() {
  return scrPlaceFullUnsafe;
}

bool Sys_TryEnterCriticalSection(CriticalSection critSect) {
  assert(static_cast<std::uint32_t>(critSect) <
         static_cast<std::uint32_t>(CRITSECT_COUNT));

  return TryEnterCriticalSection(&s_criticalSection[critSect]) != FALSE;
}

int PC_Int_Parse(int handle, int* i) {
  const static DWORD func = 0x62DF10;
  int result{};

  __asm {
     pushad;

     mov eax, handle;
     mov esi, i;
     call func;
     mov result, eax;

     popad;
  }

  return result;
}

int PC_Float_Parse(int handle, float* f) {
  const static DWORD func = 0x62DE40;
  int result{};

  __asm {
    pushad;

    mov eax, handle;
    mov esi, f;
    call func;
    mov result, eax;

    popad;
  }

  return result;
}
} // namespace game
