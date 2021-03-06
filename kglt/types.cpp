#include "types.h"

#include "utils/random.h"


namespace kglt {

Radians to_radians(const Degrees& degrees) {
    return Radians(kmDegreesToRadians(degrees.value_));
}

Degrees to_degrees(const Radians& radians) {
    return Degrees(kmRadiansToDegrees(radians.value_));
}

bool operator==(const Vec2& lhs, const Vec2& rhs) {
    auto ret = kmVec2AreEqual(&lhs, &rhs);
    return ret;
}

bool operator!=(const Vec2& lhs, const Vec2& rhs) {
    return !(lhs == rhs);
}

kglt::Vec2 operator*(float lhs, const kglt::Vec2& rhs) {
    kglt::Vec2 result;
    kmVec2Scale(&result, &rhs, lhs);
    return result;
}

kglt::Vec3 operator*(float lhs, const kglt::Vec3& rhs) {
    kglt::Vec3 result;
    kmVec3Scale(&result, &rhs, lhs);
    return result;
}

kglt::Vec3 operator/(float lhs, const kglt::Vec3& rhs) {
    kglt::Vec3 result;
    kmVec3Scale(&result, &rhs, 1.0 / lhs);
    return result;
}

Vec3 Vec3::random_deviant(const Degrees& angle, const Vec3 up) const {
    //Lovingly adapted from ogre
    Vec3 new_up = (up == Vec3()) ? perpendicular() : up;

    Quaternion q;
    kmQuaternionRotationAxisAngle(&q, this, random_gen::random_float(0, 1) * (PI * 2.0));
    kmQuaternionMultiplyVec3(&new_up, &q, &new_up);
    kmQuaternionRotationAxisAngle(&q, &new_up, Radians(angle).value_);

    Vec3 ret;
    kmQuaternionMultiplyVec3(&ret, &q, this);

    return ret;
}

Vec3 Vec3::perpendicular() const {
    //Lovingly adapted from Ogre
    static const float square_zero = (float)(1e-06 * 1e-06);
    Vec3 perp = this->cross(Vec3(1, 0, 0));

    if(perp.length_squared() < square_zero) {
        //This vector is the X-axis, so use another
        perp = this->cross(Vec3(0, 1, 0));
    }
    return perp.normalized();
}

kglt::Vec3 operator-(const kglt::Vec3& vec) {
    return kglt::Vec3(-vec.x, -vec.y, -vec.z);
}

kglt::Quaternion operator-(const kglt::Quaternion& q) {
    return kglt::Quaternion(-q.x, -q.y, -q.z, -q.w);
}

std::ostream& operator<<(std::ostream& stream, const Vec2& vec) {
    stream << "(" << vec.x << "," << vec.y << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Vec3& vec) {
    stream << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Vec4& vec) {
    stream << "(" << vec.x << "," << vec.y << "," << vec.z << "," << vec.w << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Quaternion& quat) {
    stream << "(" << quat.x << "," << quat.y << "," << quat.z << "," << quat.w << ")";
    return stream;
}

Quaternion Quaternion::look_rotation(const Vec3& direction, const Vec3& up=Vec3(0, 1, 0)) {
    Quaternion res;
    kmQuaternionLookRotation(&res, &direction, &up);
    kmQuaternionNormalize(&res, &res);
    return res;
}

Quaternion::Quaternion(const Mat3& rot_matrix) {
    /* FIXME: This should be in kazmath */

    float m12 = rot_matrix.mat[7];
    float m21 = rot_matrix.mat[5];
    float m02 = rot_matrix.mat[6];
    float m20 = rot_matrix.mat[2];
    float m10 = rot_matrix.mat[1];
    float m01 = rot_matrix.mat[3];
    float m00 = rot_matrix.mat[0];
    float m11 = rot_matrix.mat[4];
    float m22 = rot_matrix.mat[8];
    float t = m00 + m11 + m22;
    // we protect the division by s by ensuring that s>=1
    if (t >= 0) { // by w
        float s = sqrt(t + 1);
        w = 0.5 * s;
        s = 0.5 / s;
        x = (m21 - m12) * s;
        y = (m02 - m20) * s;
        z = (m10 - m01) * s;
    } else if ((m00 > m11) && (m00 > m22)) { // by x
        float s = sqrt(1 + m00 - m11 - m22);
        x = s * 0.5;
        s = 0.5 / s;
        y = (m10 + m01) * s;
        z = (m02 + m20) * s;
        w = (m21 - m12) * s;
    } else if (m11 > m22) { // by y
        float s = sqrt(1 + m11 - m00 - m22);
        y = s * 0.5;
        s = 0.5 / s;
        x = (m10 + m01) * s;
        z = (m21 + m12) * s;
        w = (m02 - m20) * s;
    } else { // by z
        float s = sqrt(1 + m22 - m00 - m11);
        z = s * 0.5;
        s = 0.5 / s;
        x = (m02 + m20) * s;
        y = (m21 + m12) * s;
        w = (m10 - m01) * s;
    }
}


void Mat4::extract_rotation_and_translation(Quaternion& rotation, Vec3& translation) {
    Mat3 rot;
    kmMat4ExtractRotationMat3(this, &rot);
    kmQuaternionRotationMatrix(&rotation, &rot);
    kmMat4ExtractTranslationVec3(this, &translation);
}

Mat3 Mat3::from_rotation_x(float pitch) {
    Mat3 ret;
    kmMat3FromRotationX(&ret, pitch);
    return ret;
}

Mat3 Mat3::from_rotation_y(float yaw) {
    Mat3 ret;
    kmMat3FromRotationY(&ret, yaw);
    return ret;
}

Mat3 Mat3::from_rotation_z(float roll) {
    Mat3 ret;
    kmMat3FromRotationZ(&ret, roll);
    return ret;
}

Mat4 Mat4::from_lookat(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Mat4 ret;
    kmMat4LookAt(&ret, &eye, &target, &up);
    return ret;
}

Degrees::Degrees(const Radians &rhs) {
    value_ = kmRadiansToDegrees(rhs.value_);
}

Radians::Radians(const Degrees &rhs) {
    value_ = kmDegreesToRadians(rhs.value_);
}

}
