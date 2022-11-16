#pragma once

#define BINARY_PAYLOAD_SIZE 0x0A000000

#define DLL_EXPORT extern "C" __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

#define BASEGAME "spdata"
#define CLIENT_CONFIG "iw4xsp_config.cfg"

#include <WinSock2.h>
#include <Windows.h>
#include <wincrypt.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include <format>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")

#pragma warning(disable : 4100)
#pragma warning(disable : 4244)

#include <udis86.h>

using namespace std::literals;

// I have witnessed clang re-ordering my included even when
// it was set not to do it. :(
// clang-format off
#include "game/structs.hpp"
#include "game/game.hpp"
// clang-format on
