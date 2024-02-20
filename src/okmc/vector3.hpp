#pragma once

#include <cmath>

template <typename T>
class Vector3 {
 public:
    T x, y, z;

    T distance(Vector3<T> another);
};

template <typename T>
T Vector3<T>::distance(Vector3<T> another) {
    T dx = x - another.x;
    T dy = y - another.y;
    T dz = z - another.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
