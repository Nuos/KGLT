#include "../kazbase/exceptions.h"
#include "../scene.h"
#include "../subscene.h"

#include "mesh.h"
#include "geom_factory.h"

namespace kglt {

GeomFactory::GeomFactory(Scene& scene):
    scene_(scene) {

}

EntityID GeomFactory::new_line(SubSceneID ss, const kmVec3& start, const kmVec3& end) {
    kglt::SubScene& subscene = scene_.subscene(ss);

    kglt::MeshPtr mesh_ptr = subscene.mesh(subscene.new_mesh()).lock();
    kglt::Mesh& mesh = *mesh_ptr;

    mesh.shared_data().position(start);
    mesh.shared_data().diffuse(kglt::Colour::white);
    mesh.shared_data().tex_coord0(0.0, 0.0);
    mesh.shared_data().tex_coord1(0.0, 0.0);
    mesh.shared_data().tex_coord2(0.0, 0.0);
    mesh.shared_data().tex_coord3(0.0, 0.0);
    mesh.shared_data().normal(0, 0, 1);
    mesh.shared_data().move_next();

    mesh.shared_data().position(end);
    mesh.shared_data().diffuse(kglt::Colour::white);
    mesh.shared_data().tex_coord0(0.0, 0.0);
    mesh.shared_data().tex_coord1(0.0, 0.0);
    mesh.shared_data().tex_coord2(0.0, 0.0);
    mesh.shared_data().tex_coord3(0.0, 0.0);
    mesh.shared_data().normal(0, 0, 1);
    mesh.shared_data().move_next();

    mesh.shared_data().done();

    //Create a submesh that uses the shared data
    SubMeshIndex sm = mesh.new_submesh(
        mesh.resource_manager().scene().default_material_id(),
        MESH_ARRANGEMENT_LINES,
        true
    );

    mesh.submesh(sm).index_data().index(0);
    mesh.submesh(sm).index_data().index(1);
    mesh.submesh(sm).index_data().done();

    return subscene.new_entity(mesh.id());
}

EntityID GeomFactory::new_rectangle_outline(SubSceneID ss, const float width, const float height) {
    kglt::SubScene& subscene = scene_.subscene(ss);

    kglt::MeshPtr mesh_ptr = subscene.mesh(subscene.new_mesh()).lock();
    procedural::mesh::rectangle_outline(mesh_ptr, width, height);

    return subscene.new_entity(mesh_ptr->id());
}

EntityID GeomFactory::new_rectangle(SubSceneID ss, const float width, const float height) {
    throw NotImplementedError(__FILE__, __LINE__);
}

EntityID GeomFactory::new_capsule(SubSceneID, const float diameter, const float length) {
    throw NotImplementedError(__FILE__, __LINE__);
}

EntityID GeomFactory::new_sphere(SubSceneID ss, const kmVec3& position, const float diameter) {
    throw NotImplementedError(__FILE__, __LINE__);
}

EntityID GeomFactory::new_cube(SubSceneID ss, const kmVec3& position, const float diameter) {
    throw NotImplementedError(__FILE__, __LINE__);
}

}