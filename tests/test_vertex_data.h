#ifndef TEST_VERTEX_DATA_H
#define TEST_VERTEX_DATA_H

#include "kglt/kglt.h"
#include "kaztest/kaztest.h"

#include "global.h"

class VertexDataTest : public KGLTTestCase {
public:
    void test_offsets() {
        kglt::VertexSpecification spec = {
            kglt::VERTEX_ATTRIBUTE_3F,
            kglt::VERTEX_ATTRIBUTE_3F,
            kglt::VERTEX_ATTRIBUTE_2F
        };

        kglt::VertexData::ptr data = kglt::VertexData::create(spec);

        assert_equal(0, (int32_t) data->position_offset());
        assert_equal(sizeof(float) * 3, data->normal_offset());
        assert_equal(sizeof(float) * 6, data->texcoord0_offset());
    }

    void test_basic_usage() {
        kglt::VertexSpecification spec = kglt::VertexSpecification::POSITION_AND_DIFFUSE;
        spec.texcoord0_attribute = kglt::VERTEX_ATTRIBUTE_2F;

        kglt::VertexData data(spec);

        assert_equal(0, data.data_size());

        data.position(0, 0, 0);
        data.tex_coord0(1, 1);

        assert_equal(sizeof(float) * 9, data.data_size());
        data.move_next();
        data.position(0, 0, 0);
        data.tex_coord0(2, 2);
        data.done();

        assert_equal(sizeof(float) * 18, data.data_size());
    }
};

#endif // TEST_VERTEX_DATA_H
