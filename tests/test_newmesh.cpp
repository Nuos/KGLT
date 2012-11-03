#include <unittest++/UnitTest++.h>

#include "kglt/kglt.h"
#include "kglt/newmesh.h"
#include "kglt/entity.h"
#include "kglt/vertex_data.h"

using namespace kglt::newmesh;

MeshID generate_test_mesh(kglt::Scene& scene) {
    MeshID mid = scene.new_newmesh();
    Mesh& mesh = scene.newmesh(mid);

    kglt::VertexData& data = mesh.shared_data();

    data.position(-1.0, -1.0, 0.0);
    data.move_next();

    data.position( 1.0, -1.0, 0.0);
    data.move_next();

    data.position( 1.0, 1.0, 0.0);
    data.move_next();

    data.position(-1.0, 1.0, 0.0);
    data.move_next();

    data.done();

    SubMesh& submesh = mesh.submesh(mesh.new_submesh(0));

    submesh.index_data().index(0);
    submesh.index_data().index(1);
    submesh.index_data().index(2);

    submesh.index_data().index(0);
    submesh.index_data().index(2);
    submesh.index_data().index(3);

    //Draw a line between the first two vertices
    SubMesh& sm = mesh.submesh(mesh.new_submesh(0, kglt::MESH_ARRANGEMENT_LINES));
    sm.index_data().index(0);
    sm.index_data().index(1);

    return mid;
}

SUITE(test_mesh) {

TEST(basic_usage) {    
    kglt::Window::ptr window = kglt::Window::create();
    kglt::Scene& scene = window->scene();
    Mesh& mesh = scene.newmesh(generate_test_mesh(scene));

    kglt::VertexData& data = mesh.shared_data();

    CHECK(data.has_positions());
    CHECK(!data.has_normals());
    CHECK(!data.has_texcoord0());
    CHECK(!data.has_texcoord1());
    CHECK(!data.has_texcoord2());
    CHECK(!data.has_texcoord3());
    CHECK(!data.has_texcoord4());
    CHECK(!data.has_diffuse());
    CHECK(!data.has_specular());
    CHECK_EQUAL(4, data.count());

    CHECK_EQUAL(2, mesh.submesh_count());
}

TEST(entity_from_mesh) {
    kglt::Window::ptr window = kglt::Window::create();
    kglt::Scene& scene = window->scene();

    Mesh& mesh = scene.newmesh(generate_test_mesh(scene));

    kglt::Entity& entity = scene.entity(scene.new_entity());

    CHECK(!entity.has_mesh());

    entity.set_mesh(mesh.id());

    CHECK(entity.has_mesh());

    //The entity's MeshID should match the mesh we set
    CHECK(mesh.id() == entity.mesh());

    //The entity should report the same data as the mesh, the same subentity count
    //as well as the same shared vertex data
    CHECK_EQUAL(mesh.submesh_count(), entity.subentity_count());
    CHECK(mesh.shared_data().count() == entity.shared_data().count());

    //Likewise for subentities, they should just proxy to the submesh
    CHECK_EQUAL(mesh.submesh(0).material(), entity.subentity(0).material());
    CHECK(mesh.submesh(0).index_data() == entity.subentity(0).index_data());
    CHECK(mesh.submesh(0).vertex_data() == entity.subentity(0).vertex_data());

    //We should be able to override the material on a subentity though
    entity.subentity(0).override_material(1);

    CHECK_EQUAL(1, entity.subentity(0).material());
}

TEST(scene_methods) {
    kglt::Window::ptr window = kglt::Window::create();
    kglt::Scene& scene = window->scene();

    Mesh& mesh = scene.newmesh(scene.new_newmesh()); //Create a mesh
    kglt::Entity& entity = scene.entity(scene.new_entity());
    entity.set_mesh(mesh.id());

    CHECK(mesh.id() == entity.mesh());
}

}