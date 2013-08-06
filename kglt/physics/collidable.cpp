#include "../stage.h"
#include "collidable.h"
#include "../object.h"

namespace kglt {

Collidable::Collidable(Object* owner, PhysicsEngine* engine):
    owner_(owner),
    engine_(engine) {

    assert(owner_ || engine_);

    if(owner_) {
        owner->signal_made_responsive().connect([=]() {
            attach_to_responsive_body(owner->responsive_body());
        });

        if(!engine_) {
            engine_ = owner_->stage().scene().physics_engine();
        }
    }

    assert(engine_);
}

}