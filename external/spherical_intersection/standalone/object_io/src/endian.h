#ifndef ENDIAN_H
#define ENDIAN_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace endian {
enum Endian { little, big };

const Endian system_endian = []() {
	std::uint32_t i = 0x01020304;
	char *c = reinterpret_cast<char *>(&i);
	if (c[0] == 0x01 && c[1] == 0x02 && c[2] == 0x03 && c[3] == 0x04) {
		return Endian::big;
	} else if (c[0] == 0x04 && c[1] == 0x03 && c[2] == 0x02 &&
		   c[3] == 0x01) {
		return Endian::little;
	} else {
		throw std::logic_error("Unknown endian.");
	}
}();

template <typename T>
std::array<unsigned char, sizeof(T)> get_binary(const T &value, Endian endian) {
	std::array<unsigned char, sizeof(T)> data;
	std::memcpy(data.data(), &value, sizeof(T));
	if (endian != system_endian) {
		std::reverse(data.begin(), data.end());
	}
	return data;
}

template <typename T> T parse_binary(unsigned char *data_ptr, Endian endian) {
	T value;
	if (endian != system_endian) {
		for (std::size_t index = 0; index < sizeof(T); index++) {
			std::memcpy(&reinterpret_cast<unsigned char *>(&value)[index],
				    &data_ptr[sizeof(T) - index - 1], 1);
		}
	} else {
		std::memcpy(&value, data_ptr, sizeof(T));
	}
	return value;
}

} // namespace endian

#endif