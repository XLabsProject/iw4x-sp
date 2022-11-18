#include <std_include.hpp>

#include <utils/smbios.hpp>
#include <utils/cryptography.hpp>
#include <utils/string.hpp>

#include "auth.hpp"

namespace auth {
namespace {
std::string get_hw_profile_guid() {
  HW_PROFILE_INFO info;
  if (!GetCurrentHwProfileA(&info)) {
    return {};
  }

  return {info.szHwProfileGuid, sizeof(info.szHwProfileGuid)};
}

std::string get_protected_data() {
  std::string input = "X-Labs-IW4x-Auth";

  DATA_BLOB data_in{}, data_out{};
  data_in.pbData = reinterpret_cast<std::uint8_t*>(input.data());
  data_in.cbData = static_cast<DWORD>(input.size());
  if (CryptProtectData(&data_in, nullptr, nullptr, nullptr, nullptr,
                       CRYPTPROTECT_LOCAL_MACHINE, &data_out) != TRUE) {
    return {};
  }

  const auto size = std::min<std::uint32_t>(data_out.cbData, 52);
  std::string result(reinterpret_cast<char*>(data_out.pbData), size);
  LocalFree(data_out.pbData);

  return result;
}

std::string get_hdd_serial() {
  DWORD serial{};
  if (!GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr,
                             nullptr, 0)) {
    return {};
  }

  return utils::string::va("{:08X}", serial);
}

std::string get_key_entropy() {
  std::string entropy{};
  entropy.append(utils::smbios::get_uuid());
  entropy.append(get_hw_profile_guid());
  entropy.append(get_protected_data());
  entropy.append(get_hdd_serial());

  if (entropy.empty()) {
    entropy.resize(32);
    utils::cryptography::random::get_data(entropy.data(), entropy.size());
  }

  return entropy;
}

utils::cryptography::ecc::key& get_key() {
  static auto key =
      utils::cryptography::ecc::generate_key(512, get_key_entropy());
  return key;
}

} // namespace

std::uint64_t get_guid() { return get_key().get_hash(); }
} // namespace auth
