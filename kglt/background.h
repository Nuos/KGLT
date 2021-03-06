#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "generic/managed.h"
#include "generic/identifiable.h"
#include "utils/parent_setter_mixin.h"
#include "object.h"

namespace kglt {

class BackgroundManager;

enum BackgroundResizeStyle {
    BACKGROUND_RESIZE_ZOOM,
    BACKGROUND_RESIZE_SCALE
};

class Background:
    public Managed<Background>,
    public generic::Identifiable<BackgroundID>,
    public Updateable,
    public Nameable,
    public Printable {

public:
    Background(BackgroundID background_id, BackgroundManager *manager);

    bool init() override;
    void cleanup() override;
    void update(double dt) override;

    void set_horizontal_scroll_rate(float x_rate);
    void set_vertical_scroll_rate(float y_rate);
    void set_texture(TextureID texture_id);
    void set_resize_style(BackgroundResizeStyle style);

    //Ownable interface
    void ask_owner_for_destruction();

    //Printable interface
    unicode __unicode__() const override;

    //Nameable interface
    const bool has_name() const override;
    void set_name(const unicode &name) override;
    const unicode name() const override;

private:
    BackgroundManager* manager_;
    unicode name_;

    void update_camera(const Viewport& viewport);

    StageID stage_id_;
    CameraID camera_id_;
    PipelineID pipeline_id_;
    MaterialID material_id_;
    ActorID actor_id_;

    BackgroundResizeStyle style_ = BACKGROUND_RESIZE_ZOOM;
    float x_rate_ = 0.0;
    float y_rate_ = 0.0;
};

}

#endif // BACKGROUND_H
