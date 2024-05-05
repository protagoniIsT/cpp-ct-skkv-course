#ifndef QUAT_HPP
#define QUAT_HPP

#include <cmath>

template<typename T>
struct matrix_t {
    T data[16];
};

template<typename T>
struct vector3_t {
    T x, y, z;
};

template<typename T>
class Quat {
public:
    Quat() : m_value{0, 0, 0, 0} {}

    Quat(T a, T b, T c, T d) : m_value{b, c, d, a} {}

    Quat(T angle, bool mode, const vector3_t<T> &axes) {
        if (!mode) {
            angle = static_cast<T>(angle * M_PI / 180);
        }
        T norm = std::sqrt(axes.x * axes.x + axes.y * axes.y + axes.z * axes.z);
        m_value[3] = std::cos(angle / 2);
        m_value[0] = axes.x / norm * std::sin(angle / 2);
        m_value[1] = axes.y / norm * std::sin(angle / 2);
        m_value[2] = axes.z / norm * std::sin(angle / 2);
    }

    const T *data() const { return m_value; };

    Quat<T> operator+(const Quat<T> &q2) const {
        return Quat(m_value[3] + q2.m_value[3], m_value[0] + q2.m_value[0], m_value[1] + q2.m_value[1], m_value[2] + q2.m_value[2]);
    }

    Quat<T> &operator+=(const Quat<T> &q2) {
        *this = *this + q2;
        return *this;
    }

    Quat<T> operator-(const Quat<T> &q2) const {
        return Quat(m_value[3] - q2.m_value[3], m_value[0] - q2.m_value[0], m_value[1] - q2.m_value[1], m_value[2] - q2.m_value[2]);
    }

    Quat<T> &operator-=(const Quat<T> &q2) {
        *this = *this - q2;
        return *this;
    }

    Quat<T> operator*(const Quat<T> &q2) const {
        T a1 = m_value[3], b1 = m_value[0], c1 = m_value[1], d1 = m_value[2];
        T a2 = q2.m_value[3], b2 = q2.m_value[0], c2 = q2.m_value[1], d2 = q2.m_value[2];
        return Quat(
                a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2,
                a1 * b2 + a2 * b1 + c1 * d2 - c2 * d1,
                a1 * c2 + a2 * c1 + d1 * b2 - d2 * b1,
                a1 * d2 + a2 * d1 + b1 * c2 - b2 * c1);
    }

    Quat<T> operator*(const T &s) const {
        return Quat(m_value[3] * s, m_value[0] * s, m_value[1] * s, m_value[2] * s);
    }

    Quat<T> operator*(const vector3_t<T> &vec) const {
        Quat<T> q2 = Quat(0, vec.x, vec.y, vec.z);
        return *this * q2;
    }

    Quat<T> operator~() const { return Quat(m_value[3], -m_value[0], -m_value[1], -m_value[2]); }

    bool operator==(const Quat<T> &q2) const {
        return (m_value[3] == q2.m_value[3] && m_value[0] == q2.m_value[0] && m_value[1] == q2.m_value[1] &&
                m_value[2] == q2.m_value[2]);
    }

    bool operator!=(const Quat<T> &q2) const { return !(*this == q2); }

    explicit operator T() const {
        return std::sqrt(m_value[3] * m_value[3] + m_value[0] * m_value[0] + m_value[1] * m_value[1] + m_value[2] * m_value[2]);
    }

    matrix_t<T> rotation_matrix() const {
        Quat<T> q = normalize();
        T s = q.m_value[3], x = q.m_value[0], y = q.m_value[1], z = q.m_value[2];
        T r_11 = 1 - 2 * y * y - 2 * z * z, r_12 = 2 * x * y - 2 * z * s, r_13 = 2 * x * z + 2 * y * s,
          r_21 = 2 * x * y + 2 * z * s, r_22 = 1 - 2 * x * x - 2 * z * z, r_23 = 2 * y * z - 2 * x * s,
          r_31 = 2 * x * z - 2 * y * s, r_32 = 2 * y * z + 2 * x * s, r_33 = 1 - 2 * x * x - 2 * y * y;
        return {r_11, r_21, r_31, 0,
                r_12, r_22, r_32, 0,
                r_13, r_23, r_33, 0,
                0,    0,    0,    1};
    }

    matrix_t<T> matrix() const {
        return {m_value[3], -m_value[0], -m_value[1], -m_value[2], m_value[0], m_value[3], -m_value[2], m_value[1],
                m_value[1], m_value[2], m_value[3], -m_value[0], m_value[2], -m_value[1], m_value[0], m_value[3]};
    }

    T angle(bool mode = true) const {
        return mode ? 2 * std::acos(m_value[3]) : 2 * std::acos(m_value[3]) * 180 / M_PI;
    }

    vector3_t<T> apply(const vector3_t<T> &vec) const {
        Quat<T> q = normalize();
        Quat<T> qv = q * vec;
        Quat<T> qq = qv * (~q);
        return {qq.m_value[0], qq.m_value[1], qq.m_value[2]};
    }

private:
    T m_value[4];

    Quat<T> normalize() const {
        T norm = T(*this);
        return *this * (1 / norm);
    }
};
#endif
