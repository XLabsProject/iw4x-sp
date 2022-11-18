#pragma once

#ifdef interface
#undef interface
#endif

namespace steam {
template <std::size_t...>
struct argument_size_calculator final : std::integral_constant<std::size_t, 0> {
};

template <std::size_t X, std::size_t... Xs>
struct argument_size_calculator<X, Xs...> final
    : std::integral_constant<std::size_t,
                             X + ((argument_size_calculator<Xs...>::value +
                                   (sizeof(void*) - 1)) &
                                  ~(sizeof(void*) - 1))> {};

class interface final {
public:
  class method final {
  public:
    void* pointer = nullptr;
    std::size_t param_size = 0;
  };

  class method_result final {
  public:
    std::string name;
    std::size_t param_size = 0;
    bool name_found = false;
    bool param_size_found = false;
  };

  interface();
  interface(void* interface_ptr);

  operator bool() const;

  template <typename T, typename... Args>
  T invoke(const std::string& method_name, Args... args) {
    if (!this->interface_ptr_) {
      throw std::runtime_error("Invalid interface pointer");
    }

    const auto method_result = this->find_method(method_name);
    if (!method_result.pointer) {
      throw std::runtime_error("Unable to find desired method");
    }

    constexpr std::size_t passed_argc =
        argument_size_calculator<sizeof(Args)...>::value;
    if (passed_argc != method_result.param_size) {
      throw std::runtime_error("Invalid argument count");
    }

    return static_cast<T(__thiscall*)(void*, Args...)>(method_result.pointer)(
        this->interface_ptr_, args...);
  }

  template <typename T, typename... Args>
  T invoke(const std::size_t table_entry, Args... args) {
    if (!this->interface_ptr_) {
      throw std::runtime_error("Invalid interface pointer");
    }

    return static_cast<T(__thiscall*)(void*, Args...)>(
        (*this->interface_ptr_)[table_entry])(this->interface_ptr_, args...);
  }

private:
  void*** interface_ptr_;
  std::unordered_map<std::string, method> methods_;

  method find_method(const std::string& name);
  method search_method(const std::string& name);

  method_result analyze_method(const void* method_ptr);

  bool is_rdata(void* pointer);
};
} // namespace steam
