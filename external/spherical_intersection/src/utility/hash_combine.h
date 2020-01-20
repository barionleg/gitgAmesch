#ifndef SPHERICAL_INTERSECTION_IMPLEMENTATION_UTILITY_HASH_COMBINE_H
#define SPHERICAL_INTERSECTION_IMPLEMENTATION_UTILITY_HASH_COMBINE_H

#include <functional>

//! @cond DEV

namespace spherical_intersection {
namespace utility {
//! @brief Combines a given hash with the hash of a given value,
//! @param seed the given hash,
//! @param v a reference to the given value,
//! @return The combined hashes.
template <class T>
inline void hash_combine(
    std::size_t &seed,
    const T &v) { // TODO taken from https://stackoverflow.com/a/2595226
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
} // namespace utility
} // namespace spherical_intersection

//! @endcond

#endif
