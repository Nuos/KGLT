#include "kglt/kglt.h"
#include "kglt/shortcuts.h"
#include "kglt/additional.h"

using kglt::extra::Sprite;
using kglt::extra::SpriteStripLoader;
using kglt::extra::Background;

int main(int argc, char* argv[]) {
    //Set up logging to stdio
    logging::get_logger("/")->add_handler(logging::Handler::ptr(new logging::StdIOHandler));

    kglt::Window::ptr window = kglt::Window::create();
    window->set_title("KGLT Parallax Sample");

    kglt::Scene& scene = window->scene();

    //Automatically calculate an orthographic projection, taking into account the aspect ratio
    //and the passed height. For example, passing a height of 2.0 would mean the view would extend
    //+1 and -1 in the vertical direction, -1.0 - +1.0 near/far, and width would be calculated from the aspect
    //window.scene().pass().viewport().configure(kglt::VIEWPORT_TYPE_BLACKBAR_16_BY_9);
    scene.camera().set_orthographic_projection_from_height((float) 224 / (float) 40, 16.0 / 9.0);

    Background::ptr background = Background::create(scene);

    //Alternatively window.scene().background().add_layer("sample_data/parallax/back_layer.png", BACKGROUND_FILL);
    background->add_layer("sample_data/parallax/back_layer.png");
    background->add_layer("sample_data/parallax/middle_layer.png");
    background->add_layer("sample_data/parallax/front_layer.png");

    double width = background->layer(0).width();
    double height = width / (16.0 / 9.0);
    background->set_visible_dimensions(width, height); //The visible height in pixels (ortho)

    //Load the strip of sprites into separate textures
    SpriteStripLoader loader(scene, "sample_data/sonic.png", 64);
    std::vector<kglt::TextureID> frames = loader.load_frames();

    //Construct a Sprite object that takes care of handling materials, meshes etc.
    Sprite::ptr sprite = Sprite::create(scene);
    sprite->add_animation("running", container::slice(frames, 31, 35), 0.5);
    sprite->set_render_dimensions(1.5, 1.5);
    sprite->move_to(0.0, -2.0, -1.0);

    //First, load a default font
    kglt::FontID fid = scene.new_font();
    kglt::Font& font = scene.font(fid);
    font.initialize("sample_data/sample.ttf", 12);

    //Set the default font for the UI
    kglt::UI& interface = scene.ui();
    interface.set_default_font_id(fid);

    //Create a label
    kglt::ui::LabelID label_id = scene.ui().new_label();
    kglt::ui::Label& label = scene.ui().label(label_id);

    label.set_foreground_colour(kglt::Colour(1.0, 1.0, 0.0, 1.0));
    label.set_text("Score: 8739204");
    label.set_position(0.02, 0.9);

    while(window->update()) {
        background->layer(0).scroll_x(0.1 * window->delta_time());
        background->layer(1).scroll_x(0.2 * window->delta_time());
        background->layer(2).scroll_x(1.0 * window->delta_time());
    }

    return 0;
}

