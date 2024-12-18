#pragma once

#include <cmath>
#include "vector.hpp"

constexpr double PI = 3.14159265358979323846;
constexpr double RAD_PI = 57.295779513082;
typedef float matrix3x4[3][4];
typedef float matrix4x4[4][4];
#define DEG2RAD(x) ((float)(x) * (float)((float)(PI) / 180.0f))
#define RAD2DEG(x) ((float)(x) * (float)(180.0f / (float)(PI)))

class vmatrix {
private:
    vector m[4][4];

public:
    const matrix3x4& As3x4() const {
        return *((const matrix3x4*)this);
    }
};

namespace math {

    inline void sine_cosine(float radians, float& sine, float& cosine) {
        sine = sin(radians);
        cosine = cos(radians);
    }

    inline float normalize(float input) {
        return (!isfinite(input) ? 0.f : remainder(input, 360.f));
    }

    inline void vector_angles(const vector& forward, vector& angles) {
        float temp, yaw, pitch;

        if (forward.y == 0 && forward.x == 0) {
            yaw = 0;

            if (forward.z > 0) {
                pitch = 270;
            }
            else {
                pitch = 90;
            }
        }
        else {
            yaw = RAD2DEG(atan2f(forward.y, forward.x));

            if (yaw < 0)
                yaw += 360;

            // glm has forced my hand
            auto length_2d = sqrtf(forward.x * forward.x + forward.y * forward.y);
            temp = length_2d;
            pitch = RAD2DEG(atan2f(-forward.z, temp));

            if (pitch < 0)
                pitch += 360;
        }

        angles.x = pitch;
        angles.y = yaw;
        angles.z = 0;
    }

    inline void angle_vectors(const vector& angles, vector* forward) {
        float sp, sy, cp, cy;

        sine_cosine(DEG2RAD(angles.x), sp, cp);
        sine_cosine(DEG2RAD(angles.y), sy, cy);

        if (forward) {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }
    }

    inline void angle_vectors(const vector& angles, vector* forward, vector* right, vector* up) {
        float sr, sp, sy, cr, cp, cy;

        sine_cosine(DEG2RAD(angles.x), sp, cp);
        sine_cosine(DEG2RAD(angles.y), sy, cy);
        sine_cosine(DEG2RAD(angles.z), sr, cr);

        if (forward) {
            forward->x = cp * cy;
            forward->y = cp * sy;
            forward->z = -sp;
        }
        if (right) {
            right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
            right->y = (-1 * sr * sp * sy + -1 * cr * cy);
            right->z = -1 * sr * cp;
        }
        if (up) {
            up->x = (cr * sp * cy + -sr * -sy);
            up->y = (cr * sp * sy + -sr * cy);
            up->z = -1 * sr * cp;
        }
    }

    template <typename T> inline T clamp(const T input, const T min, const T max) {
        if (max <= min)
            return input;

        if (input < min)
            return min;
        if (input > max)
            return max;

        return input;
    }

    inline void vector_transform(const vector& input, const matrix3x4& matrix, vector& output) {
        for (auto i = 0; i < 3; i++)
            output[i] = input.dot((vector&)matrix[i]) + matrix[i][3];
    }

    inline void PointsFromBox(const vector& mins, const vector& maxs, vector* points) {
        points[0][0] = mins[0];
        points[0][1] = mins[1];
        points[0][2] = mins[2];

        points[1][0] = mins[0];
        points[1][1] = mins[1];
        points[1][2] = maxs[2];

        points[2][0] = mins[0];
        points[2][1] = maxs[1];
        points[2][2] = mins[2];

        points[3][0] = mins[0];
        points[3][1] = maxs[1];
        points[3][2] = maxs[2];

        points[4][0] = maxs[0];
        points[4][1] = mins[1];
        points[4][2] = mins[2];

        points[5][0] = maxs[0];
        points[5][1] = mins[1];
        points[5][2] = maxs[2];

        points[6][0] = maxs[0];
        points[6][1] = maxs[1];
        points[6][2] = mins[2];

        points[7][0] = maxs[0];
        points[7][1] = maxs[1];
        points[7][2] = maxs[2];
    }

    inline void BuildTransformedBox(vector* v2, vector const& bbmin, vector const& bbmax, const matrix3x4& m) {
        vector v[8];
        PointsFromBox(bbmin, bbmax, v);

        vector_transform(v[0], m, v2[0]);
        vector_transform(v[1], m, v2[1]);
        vector_transform(v[2], m, v2[2]);
        vector_transform(v[3], m, v2[3]);
        vector_transform(v[4], m, v2[4]);
        vector_transform(v[5], m, v2[5]);
        vector_transform(v[6], m, v2[6]);
        vector_transform(v[7], m, v2[7]);
    }

    inline float remap_val_clamped(float val, float a, float b, float c, float d) {
        if (a == b)
            return val >= b ? d : c;

        float val_ = (val - a) / (b - a);
        val_ = math::clamp(val_, 0.0f, 1.0f);

        return c + (d - c) * val_;
    }

    inline QAngle calc_angle(const vector& src, const vector& dest, bool clamp) {
        vector  angles = {};
        vector  delta = src - dest;

        float hyp = std::sqrtf((delta.x * delta.x) + (delta.y * delta.y));

        angles.x = (float)(atanf(delta.z / hyp) * RAD_PI);
        angles.y = (float)(atanf(delta.y / delta.x) * RAD_PI);

        if (delta.x >= 0.f)
            angles.y += 180.f;

        if (clamp) {
            math::clamp(angles.y, -360.f, 360.f);
            math::clamp(angles.x, -89.f, 89.f);
            angles.z = 0.f;
        }
        else {
            angles.z = 0.f; // fuck roll
        }

        return angles;
    }

    inline float calc_fov(const vector& src, const vector& dest) {
        vector src1 = {};
        math::angle_vectors(src, &src1);

        vector dest1 = {};
        math::angle_vectors(dest, &dest1);

        return src1.dist_to(dest1);
    }

} // namespace math