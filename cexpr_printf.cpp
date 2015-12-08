#include <cstdint>
#include <utility>
#include <iostream>
#include <tuple>

namespace cexpr {
namespace details {

template <int...>
struct seq {};

template <int N, int... S>
struct gens : gens<N - 1, N - 1, S...> {};

template <int... S>
struct gens<0, S...> {
    typedef seq<S...> type;
};

template <typename Output>
constexpr void itoa(Output& out, std::size_t value) {
    if (value == 0) {
        out.append('0');
    }

    char buf[10] = {};
    int i = sizeof(buf);

    while (value > 0) {
        i--;
        buf[i] = (value % 10) + '0';
        value = value / 10;
    }

    for (; i < sizeof(buf); i++) {
        out.append(buf[i]);
    }
}

template <typename Output, std::size_t format_size>
constexpr bool process_plain(Output& out,
                             const char(&format)[format_size],
                             std::size_t& format_offset) {
    while (true) {
        if (format_offset == format_size) return false;

        if (format[format_offset] == '%') {
            format_offset++;
            return true;
        }

        out.append(format[format_offset++]);
    }
}

template <std::size_t n, typename Output, std::size_t format_size, typename... Args>
constexpr typename std::enable_if<n == std::tuple_size<std::tuple<Args...>>::value, int>::type
helper(Output&& out,
       const char(&format)[format_size],
       std::size_t format_offset,
       const std::tuple<Args...>& args) {
    process_plain(out, format, format_offset);

    return out.size();
}

template <std::size_t n, typename Output, std::size_t format_size, typename... Args>
constexpr typename std::enable_if<n != std::tuple_size<std::tuple<Args...>>::value, int>::type
helper(Output&& out,
       const char(&format)[format_size],
       std::size_t format_offset,
       const std::tuple<Args...>& args) {
    process_plain(out, format, format_offset);

    switch (format[format_offset++]) {
        case 'd':
            itoa(out, std::get<n>(args));
        default:
            break;
    }

    return helper<n + 1>(std::move(out), format, format_offset, args);
}

class Counter {
public:
    constexpr Counter() = default;

    constexpr void append(char c) {
        size_++;
    }

    constexpr std::size_t size() const { return size_; }

private:
    std::size_t size_ = 0;
};

template <std::size_t output_size>
class Writer {
public:
    constexpr Writer(char(&out)[output_size]) : out_(out) {}

    constexpr void append(char c) {
        out_[offset_++] = c;
    }

    constexpr std::size_t size() const { return output_size - offset_; }

private:
    std::size_t offset_ = 0;
    char(&out_)[output_size];
};

}

template <typename T>
struct cexpr_string {
    constexpr cexpr_string() {
        details::helper<0>(details::Writer<sizeof(value)>(value), T::format, 0, T::args);
    }

    char value[details::helper<0>(details::Counter(), T::format, 0, T::args)];
};

}

#define CONSTEXPR_STRING(name, fmt, ...) \
struct name_type { \
    static constexpr char format[] = fmt; \
    static constexpr auto args = std::make_tuple(__VA_ARGS__); \
}; \
constexpr decltype(name_type::format) name_type::format; \
constexpr decltype(name_type::args) name_type::args; \
constexpr cexpr::cexpr_string<name_type> name;

CONSTEXPR_STRING(my_str, "example is %d, %d and %d\n", 1, 2, 3)

int main(int argc, char** argv) {
    std::cout << my_str.value;
    return 0;
}
