#pragma once

#include "kaztest/kaztest.h"

#include "kglt/kglt.h"

class QuaternionTest : public TestCase {
public:

    void test_mat3_to_quaternion() {
        kglt::Mat3 mat;
        for(uint32_t i = 0; i < 9; ++i) mat.mat[i] = 0;

        // 90 degree rotation around z
        mat.mat[1] = 1;
        mat.mat[3] = -1;
        mat.mat[8] = 1;

        kglt::Quaternion quat(mat);

        assert_close(quat.x, 0, 0.001);
        assert_close(quat.y, 0, 0.001);
        assert_close(quat.z, 0.707, 0.001);
        assert_close(quat.w, 0.707, 0.001);
    }

};
