#include <thread>

#include "utils/gl_error.h"
#include "window_base.h"
#include "ui/interface.h"
#include "input_controller.h"
#include "loaders/texture_loader.h"
#include "loaders/material_script.h"
#include "loaders/opt_loader.h"
#include "loaders/ogg_loader.h"
#include "loaders/rml_loader.h"
#include "loaders/obj_loader.h"
#include "loaders/tiled_loader.h"
#include "loaders/particle_script.h"
#include "loaders/heightmap_loader.h"
#include "loaders/q2bsp_loader.h"
#include "loaders/wal_loader.h"
#include "loaders/md2_loader.h"
#include "loaders/pcx_loader.h"

#include "sound.h"
#include "camera.h"
#include "watcher.h"
#include "message_bar.h"
#include "render_sequence.h"
#include "stage.h"
#include "overlay.h"
#include "virtual_gamepad.h"
#include "screens/loading.h"
#include "utils/gl_thread_check.h"
#include "utils/gl_error.h"

#include "panels/stats_panel.h"
#include "panels/partitioner_panel.h"

namespace kglt {

WindowBase::WindowBase():
    Source(this),
    StageManager(this),
    OverlayManager(this),    
    CameraManager(this),
    resource_manager_(new ResourceManager(this)),
    initialized_(false),
    width_(-1),
    height_(-1),
    is_running_(true),
    idle_(*this),
    resource_locator_(ResourceLocator::create()),
    frame_counter_time_(0),
    frame_counter_frames_(0),
    frame_time_in_milliseconds_(0),
    total_time_(0),
    background_manager_(new BackgroundManager(this)) {

    ktiGenTimers(1, &fixed_timer_);
    ktiBindTimer(fixed_timer_);
    ktiStartFixedStepTimer(WindowBase::STEPS_PER_SECOND);

    ktiGenTimers(1, &variable_timer_);
    ktiBindTimer(variable_timer_);
    ktiStartGameTimer();

    kazlog::get_logger("/")->add_handler(kazlog::Handler::ptr(new kazlog::StdIOHandler));

}

WindowBase::~WindowBase() {
    //FIXME: Make WindowBase Managed<> and put this in cleanup()
    virtual_gamepad_.reset();
    loading_.reset();
    message_bar_.reset();
    watcher_.reset();
    background_manager_.reset();
    render_sequence_.reset();

    Sound::shutdown_openal();

    delete resource_manager_;
}

RenderSequencePtr WindowBase::render_sequence() {
    return render_sequence_;
}

LoaderPtr WindowBase::loader_for(const unicode &filename, LoaderHint hint) {
    unicode final_file = resource_locator->locate_file(filename);

    std::vector<std::pair<LoaderTypePtr, LoaderPtr>> possible_loaders;

    for(LoaderTypePtr loader_type: loaders_) {
        if(loader_type->supports(final_file)) {
            auto new_loader = loader_type->loader_for(final_file, resource_locator->read_file(final_file));
            new_loader->set_resource_locator(this->resource_locator_.get());

            possible_loaders.push_back(
                std::make_pair(loader_type, new_loader)
            );
        }
    }

    if(possible_loaders.size() == 1) {
        return possible_loaders.front().second;
    } else if(possible_loaders.size() > 1) {
        if(hint != LOADER_HINT_NONE) {
            for(auto& p: possible_loaders) {
                if(p.first->has_hint(hint)) {
                    return p.second;
                }
            }
        }

        throw std::logic_error(_u("More than one possible loader was found for '{0}'. Please specify a hint.").format(filename).encode());
    }

    return LoaderPtr();
}


LoaderPtr WindowBase::loader_for(const unicode& loader_name, const unicode &filename) {
    unicode final_file = resource_locator->locate_file(filename);

    for(LoaderTypePtr loader_type: loaders_) {
        if(loader_type->name() == loader_name) {
            if(loader_type->supports(final_file)) {
                return loader_type->loader_for(final_file, resource_locator->read_file(final_file));
            } else {
                throw std::logic_error(_u("Loader '{0}' does not support file '{1}'").format(loader_name, filename).encode());
            }
        }
    }

    return LoaderPtr();
}

LoaderTypePtr WindowBase::loader_type(const unicode& loader_name) const {
    for(LoaderTypePtr loader_type: loaders_) {
        if(loader_type->name() == loader_name) {
            return loader_type;
        }
    }
    return LoaderTypePtr();
}

void WindowBase::create_defaults() {
    message_bar_ = MessageBar::create(*this);
    loading_ = screens::Loading::create(*this);

    //This needs to happen after SDL or whatever is initialized
    input_controller_ = InputController::create(*this);
}

bool WindowBase::_init(int width, int height, int bpp, bool fullscreen) {
    GLThreadCheck::init();

    set_width(width);
    set_height(height);

    bool result = create_window(width, height, bpp, fullscreen);

    // Initialize the render_sequence once we have a renderer
    render_sequence_ = std::make_shared<RenderSequence>(this);

    if(result && !initialized_) {        
        //watcher_ = Watcher::create(*this);

        L_INFO("Registering loaders");

        //Register the default resource loaders
        register_loader(std::make_shared<kglt::loaders::TextureLoaderType>());
        register_loader(std::make_shared<kglt::loaders::MaterialScriptLoaderType>());
        register_loader(std::make_shared<kglt::loaders::KGLPLoaderType>());
        register_loader(std::make_shared<kglt::loaders::OPTLoaderType>());
        register_loader(std::make_shared<kglt::loaders::OGGLoaderType>());
        register_loader(std::make_shared<kglt::loaders::RMLLoaderType>());
        register_loader(std::make_shared<kglt::loaders::OBJLoaderType>());
        register_loader(std::make_shared<kglt::loaders::TiledLoaderType>());
        register_loader(std::make_shared<kglt::loaders::HeightmapLoaderType>());
        register_loader(std::make_shared<kglt::loaders::Q2BSPLoaderType>());
        register_loader(std::make_shared<kglt::loaders::WALLoaderType>());
        register_loader(std::make_shared<kglt::loaders::MD2LoaderType>());
        register_loader(std::make_shared<kglt::loaders::PCXLoaderType>());

        L_INFO("Initializing OpenAL");
        Sound::init_openal();

        L_INFO("Initializing the default resources");
        shared_assets->init();

        create_defaults();

        register_panel(1, std::make_shared<StatsPanel>(this));
        register_panel(2, std::make_shared<PartitionerPanel>(this));

        using std::bind;

        //C++11 lambda awesomeness! input_controller isn't initialized yet
        //so we connect ESCAPE in an idle task
        idle->add_once([=]() {
            //Bind the stop_running method to the ESCAPE key
            input_controller().keyboard().key_pressed_connect(
                SDL_SCANCODE_ESCAPE, bind(&WindowBase::stop_running, this)
            );
        });

        initialized_ = true;
    }

    return result;
}

void WindowBase::register_panel(uint8_t function_key, std::shared_ptr<Panel> panel) {
    PanelEntry entry;
    entry.panel = panel;
    entry.keyboard_connection = input_controller_->keyboard().key_pressed_connect((SDL_Scancode) (int(SDL_SCANCODE_F1) + (function_key - 1)), [panel](SDL_Keysym sym) {
        if(panel->is_active()) {
            panel->deactivate();
        } else {
            panel->activate();
        }
    });

    panels_[function_key] = entry;
}

void WindowBase::unregister_panel(uint8_t function_key) {
    panels_[function_key].keyboard_connection.disconnect();
    panels_.erase(function_key);
}

void WindowBase::set_logging_level(LoggingLevel level) {
    kazlog::get_logger("/")->set_level((kazlog::LOG_LEVEL) level);
}

void WindowBase::update(double dt) {
    if(is_paused()) {
        dt = 0.0; //If the application window is not displayed, don't send a deltatime down
        //it's still accessible through get_deltatime if the user needs it
    }

    background_manager_->update(dt);
    StageManager::update(dt);
}

void WindowBase::fixed_update(double dt) {
    if(is_paused()) return;
    StageManager::fixed_update(dt);
}

bool WindowBase::run_frame() {
    signal_frame_started_();

    ktiBindTimer(variable_timer_);
    ktiUpdateFrameTime();

    delta_time_ = ktiGetDeltaTime();
    total_time_ += delta_time_;

    frame_counter_time_ += delta_time_;
    frame_counter_frames_++;

    if(frame_counter_time_ >= 1.0) {
        stats->set_frames_per_second(frame_counter_frames_);

        frame_time_in_milliseconds_ = 1000.0 / double(frame_counter_frames_);
        frame_counter_frames_ = 0;
        frame_counter_time_ = 0.0;
    }

    pre_update(delta_time_);

    //Update any playing sounds
    update_source(delta_time_);
    input_controller().update(delta_time_);

    update(delta_time_);
    post_update(delta_time_);


    ktiBindTimer(fixed_timer_);
    ktiUpdateFrameTime();
    double fixed_step = ktiGetDeltaTime();

    check_events();

    while(ktiTimerCanUpdate()) {
        pre_fixed_update(fixed_step);
        signal_step_(fixed_step); //Trigger any steps
        fixed_update(fixed_step); // Run the fixed updates on controllers
        post_fixed_update(fixed_step);
    }

    fixed_step_interp_ = ktiGetAccumulatorValue();
    signal_post_step_(fixed_step_interp_);

    shared_assets->update(delta_time_);
    idle_.execute(); //Execute idle tasks before render

    /* Don't run the render sequence if we don't have a context, and don't update the resource
     * manager either because that probably needs a context too! */
    {
        std::lock_guard<std::mutex> rendering_lock(context_lock_);
        if(has_context()) {
            render_sequence()->run();

            signal_pre_swap_();

            swap_buffers();
            GLChecker::end_of_frame_check();

            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    signal_frame_finished_();

    if(!is_running_) {
        signal_shutdown_();

        watcher_.reset();
        loading_.reset();

        //Shutdown the input controller
        input_controller_.reset();
    }

    return is_running_;

}

void WindowBase::register_loader(LoaderTypePtr loader) {
    if(std::find(loaders_.begin(), loaders_.end(), loader) != loaders_.end()) {
        throw std::logic_error("Tried to add the same loader twice");
    }

    loaders_.push_back(loader);
}

Mouse& WindowBase::mouse() {
    return input_controller_->mouse();
}

Joypad& WindowBase::joypad(uint8_t idx) {
    return input_controller_->joypad(idx);
}

uint8_t WindowBase::joypad_count() const {
    return input_controller_->joypad_count();
}

double WindowBase::fixed_step_interp() const {
    return fixed_step_interp_;
}

void WindowBase::set_paused(bool value) {
    if(value == is_paused_) return;

    if(value) {
        L_INFO("Pausing application");
    } else {
        L_INFO("Unpausing application");
    }

    is_paused_ = value;
}

void WindowBase::set_has_context(bool value) {
    if(value == has_context_) return;

    has_context_ = value;
}

void WindowBase::enable_virtual_joypad(VirtualDPadDirections directions, int button_count, bool flipped) {
    if(virtual_gamepad_) {
        virtual_gamepad_.reset();
    }

    virtual_gamepad_ = VirtualGamepad::create(*this, directions, button_count);
    input_controller().init_virtual_joypad();

    if(flipped) {
        virtual_gamepad_->flip();
    }
}

void WindowBase::disable_virtual_joypad() {
    /*
     * This is a little hacky, but basically when we disable the virtual gamepad's
     * UI, it triggers a series of events that release any buttons which are currently
     * held down. That inevitably calls has_virtual_gamepad, so if we trigger that whole
     * series just by calling reset() has_virtual_gamepad then returns false and the world ends.
     *
     * So, here we disable the gamepad's pipeline before wiping it out, so those events are triggered
     * while the virtual gamepad still exists. Then the pipeline is deleted by reset()
     */

    if(virtual_gamepad_) {
        virtual_gamepad_->_disable_rendering();
        virtual_gamepad_.reset();
    }
}

void WindowBase::handle_mouse_motion(int x, int y, bool pos_normalized) {
    if(pos_normalized) {
        x *= width();
        y *= height();
    }

    OverlayManager::each([=](Overlay* object) {
        object->__handle_mouse_move(x, y);
    });
}

void WindowBase::handle_mouse_button_down(int button) {
    OverlayManager::each([=](Overlay* object) {
        object->__handle_mouse_down(button);
    });
}

void WindowBase::handle_mouse_button_up(int button) {
    OverlayManager::each([=](Overlay* object) {
        object->__handle_mouse_up(button);
    });
}

void WindowBase::handle_touch_down(int finger_id, int x, int y) {
    OverlayManager::each([=](Overlay* object) {
        object->__handle_touch_down(finger_id, x, y);
    });
}

void WindowBase::handle_touch_motion(int finger_id, int x, int y) {
    OverlayManager::each([=](Overlay* object) {
        object->__handle_touch_motion(finger_id, x, y);
    });
}

void WindowBase::handle_touch_up(int finger_id, int x, int y) {
    OverlayManager::each([=](Overlay* object) {
        object->__handle_touch_up(finger_id, x, y);
    });
}

/**
 * @brief WindowBase::reset
 *
 * Destroys all stages, and overlays, and releases all loadables. Then resets the
 * window to its original state.
 */
void WindowBase::reset() {
    idle->execute(); //Execute any idle tasks before we go deleting things

    render_sequence()->delete_all_pipelines();

    CameraManager::destroy_all();
    OverlayManager::destroy_all();
    StageManager::destroy_all();
    background_manager_.reset(new BackgroundManager(this));

    create_defaults();
}

/* PipelineHelperAPIInterface */
PipelinePtr WindowBase::pipeline(PipelineID pid){
    return render_sequence_->pipeline(pid);
}

bool WindowBase::enable_pipeline(PipelineID pid) {
    /*
     * Enables the specified pipeline, returns true if the pipeline
     * was enabled, or false if it was already enabled
     */
    auto pipeline = render_sequence_->pipeline(pid);
    bool state = pipeline->is_active();
    pipeline->activate();
    return state != pipeline->is_active();
}

bool WindowBase::disable_pipeline(PipelineID pid) {
    /*
     * Disables the specified pipeline, returns true if the pipeline
     * was disabled, or false if it was already disabled
     */
    auto pipeline = render_sequence_->pipeline(pid);
    bool state = pipeline->is_active();
    pipeline->deactivate();
    return state != pipeline->is_active();
}

void WindowBase::delete_pipeline(PipelineID pid) {
    render_sequence_->delete_pipeline(pid);
}

bool WindowBase::has_pipeline(PipelineID pid) const {
    return render_sequence_->contains(pid);
}

bool WindowBase::is_pipeline_enabled(PipelineID pid) const {
    return render_sequence_->pipeline(pid)->is_active();
}
/* End PipelineHelperAPIInterface */


}
