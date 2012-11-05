#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

#include <cstdint>
#include <vector>

#include "generic/managed.h"
#include "kazmath/vec3.h"
#include "kazmath/vec4.h"
#include "buffer_object.h"
#include "colour.h"

namespace kglt {

enum AttributeBitMask {
    BM_POSITIONS = 1,
    BM_NORMALS = 2,
    BM_TEXCOORD_0 = 4,
    BM_TEXCOORD_1 = 8,
    BM_TEXCOORD_2 = 16,
    BM_TEXCOORD_3 = 32,
    BM_TEXCOORD_4 = 64,
    BM_DIFFUSE = 128,
    BM_SPECULAR = 256
};

/*
 *  FIXME:
 *  The BufferObject maintained by VertexData includes all attributes, even if some of them
 *  haven't been used - this is ridiculously inefficient. Also the values returned from the *_offset
 *  methods are fixed and don't vary according to the attributes being used. What should happen
 *  is the buffer object should only include the attributes in use, and the offset methods should
 *  throw an exception if they are called on a VertexData instance that doesn't have the associated
 *  attribute in use. Similarly, texture coordinates should only take up the correct size in the
 *  buffer object and not always the full 4 floats.
 */
class VertexData :
    public Managed<VertexData> {

public:
    VertexData();

    void set_texture_coordinate_dimensions(uint8_t coord_index, uint8_t count);

    void clear();
    void move_to_start();
    void move_to(uint16_t index);
    void move_to_end();
    void move_next();

    void done();

    void position(float x, float y, float z);
    void position(const kmVec3& pos);

    void normal(float x, float y, float z);
    void normal(const kmVec3& n);

    void tex_coord0(float u);
    void tex_coord0(float u, float v);
    void tex_coord0(float u, float v, float w);
    void tex_coord0(float x, float y, float z, float w);

    void tex_coord1(float u);
    void tex_coord1(float u, float v);
    void tex_coord1(float u, float v, float w);
    void tex_coord1(float x, float y, float z, float w);

    void tex_coord2(float u);
    void tex_coord2(float u, float v);
    void tex_coord2(float u, float v, float w);
    void tex_coord2(float x, float y, float z, float w);

    void tex_coord3(float u);
    void tex_coord3(float u, float v);
    void tex_coord3(float u, float v, float w);
    void tex_coord3(float x, float y, float z, float w);

    void tex_coord4(float u);
    void tex_coord4(float u, float v);
    void tex_coord4(float u, float v, float w);
    void tex_coord4(float x, float y, float z, float w);

    void diffuse(float r, float g, float b, float a);
    void diffuse(const Colour& colour);

    void specular(float r, float g, float b, float a);
    void specular(const Colour& colour);

    bool has_positions() const { return enabled_bitmask_ & BM_POSITIONS; }
    bool has_normals() const { return enabled_bitmask_ & BM_NORMALS; }
    bool has_texcoord0() const { return enabled_bitmask_ & BM_TEXCOORD_0; }
    bool has_texcoord1() const { return enabled_bitmask_ & BM_TEXCOORD_1; }
    bool has_texcoord2() const { return enabled_bitmask_ & BM_TEXCOORD_2; }
    bool has_texcoord3() const { return enabled_bitmask_ & BM_TEXCOORD_3; }
    bool has_texcoord4() const { return enabled_bitmask_ & BM_TEXCOORD_4; }
    bool has_diffuse() const { return enabled_bitmask_ & BM_DIFFUSE; }
    bool has_specular() const { return enabled_bitmask_ & BM_SPECULAR; }

    uint16_t count() const { return data_.size(); }

    bool operator==(const VertexData& other) const {
        return this->data_ == other.data_;
    }

    bool operator!=(const VertexData& other) const {
        return !(*this == other);
    }

    uint32_t stride() const { return sizeof(Vertex); }

    uint32_t position_offset() const {
        Vertex vert;
        return uint64_t(&vert.position) - uint64_t(&vert);
    }

    uint32_t normal_offset() const {
        Vertex vert;
        return uint64_t(&vert.normal) - uint64_t(&vert);
    }

    uint32_t texcoord0_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[0]) - uint64_t(&vert);
    }

    uint32_t texcoord1_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[1]) - uint64_t(&vert);
    }

    uint32_t texcoord2_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[2]) - uint64_t(&vert);
    }

    uint32_t texcoord3_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[3]) - uint64_t(&vert);
    }

    uint32_t texcoord4_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[4]) - uint64_t(&vert);
    }

    uint32_t diffuse_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[5]) - uint64_t(&vert);
    }

    uint32_t specular_offset() const {
        Vertex vert;
        return uint64_t(&vert.tex_coords[6]) - uint64_t(&vert);
    }

    BufferObject& buffer_object() {
        return buffer_object_;
    }

    const BufferObject& buffer_object() const {
        return buffer_object_;
    }

private:
    int32_t enabled_bitmask_;
    uint16_t tex_coord_dimensions_[8];

    struct Vertex {
        kmVec3 position;
        kmVec3 normal;
        kmVec4 tex_coords[8];
        Colour diffuse;
        Colour specular;

        bool operator==(const Vertex& other) const {
            for(uint8_t i = 0; i < 8; ++i) {

                if(!kmVec4AreEqual(&this->tex_coords[i], &other.tex_coords[i])) {
                    return false;
                }
            }

            return kmVec3AreEqual(&this->position, &other.position) &&
                   kmVec3AreEqual(&this->normal, &other.normal) &&
                    this->diffuse == other.diffuse &&
                    this->specular == other.specular;
        }
    };

    std::vector<Vertex> data_;
    int32_t cursor_position_;
    BufferObject buffer_object_;

    void check_or_add_attribute(AttributeBitMask attr);

    void tex_coordX(uint8_t which, float u);
    void tex_coordX(uint8_t which, float u, float v);
    void tex_coordX(uint8_t which, float u, float v, float w);
    void tex_coordX(uint8_t which, float x, float y, float z, float w);
    void check_texcoord(uint8_t which);
};


class IndexData {
public:
    IndexData();

    void clear() { indices_.clear(); }
    void reserve(uint16_t size) { indices_.reserve(size); }
    void index(uint16_t idx) { indices_.push_back(idx); }
    void done();

    uint16_t count() const { return indices_.size(); }

    const std::vector<uint16_t>& all() const { return indices_; }

    bool operator==(const IndexData& other) const {
        return this->indices_ == other.indices_;
    }

    bool operator!=(const IndexData& other) const {
        return !(*this == other);
    }

    BufferObject& buffer_object() {
        return buffer_object_;
    }

    const BufferObject& buffer_object() const {
        return buffer_object_;
    }
private:
    std::vector<uint16_t> indices_;
    BufferObject buffer_object_;
};


}

#endif // VERTEX_DATA_H
