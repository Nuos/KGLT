#pragma once

#include "rigid_body.h"

namespace kglt {
namespace controllers {

class RaycastVehicle;

typedef sig::signal<void (RaycastVehicle*)> RollDetectedSignal;
typedef sig::signal<void (RaycastVehicle*)> VehicleGroundedSignal;
typedef sig::signal<void (RaycastVehicle*)> VehicleAirbourneSignal;

class RaycastVehicle:
    public RigidBody,
    public Managed<RaycastVehicle> {

    DEFINE_SIGNAL(RollDetectedSignal, signal_roll_detected);
    DEFINE_SIGNAL(VehicleGroundedSignal, signal_vehicle_grounded);
    DEFINE_SIGNAL(VehicleAirbourneSignal, signal_vehicle_airbourne);

public:    
    RaycastVehicle(Controllable *object,
        RigidBodySimulation::ptr simulation,
        float wheel_height
    );

    bool init() override;

    using Managed<RaycastVehicle>::create;
    using Managed<RaycastVehicle>::ptr;

    void accelerate() {
        drive_force_ += acceleration_;
    }

    void decelerate() {
        drive_force_ += deceleration_;
    }

    void turn_left() {
        turn_force_ -= turn_speed_;
    }

    void turn_right() {
        turn_force_ += turn_speed_;
    }

private:
    float acceleration_ = 100.0f;
    float deceleration_ = -100.0f;
    float turn_speed_ = 720.0f;

    float wheel_height_ = 2.0;

    Vec3 wheel_position_local_[4];
    Vec3 ground_detector_local_;

    /* This is a ray which protudes directly upwards, if it detects a hit
     * and the collision surface normal is pointing downwards, then we fire a signal
     * for the user to deal with it
     */
    Ray roll_detector_;
    Ray wheels_[4]; // 4 corner ground detection

    /* Additional central downwards ray
     * This prevents the body of the car falling through the floor
     * if fewer than 3 wheels are in touch with the ground. It also gives the
     * illusion of the car "bumping" over something in the road if it falls
     * between the wheels */
    Ray ground_detector_;

    bool roll_detected() const { return false; }
    bool is_rolling_ = false;

    bool is_grounded_ = false;

    /* Rays must be recalculated each frame as they need to be in world coordinates
     * and so transformed by the current body transformation */
    void recalculate_rays();

    float turn_force_ = 0.0f;
    float drive_force_ = 0.0f;

    void do_fixed_update(double dt) override;
    void do_pre_update(double dt) {
        // Before we read any input or anything, clear the forces
        drive_force_ = 0.0f;
        turn_force_ = 0.0f;
    }
};


}
}
