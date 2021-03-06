#ifndef APPLICATION_H
#define APPLICATION_H

#include <cstdint>
#include <memory>
#include <list>
#include <thread>
#include <future>
#include <SDL.h>

#include "types.h"
#include "utils/unicode.h"
#include "screens/screen_manager.h"
#include "generic/property.h"
#include "generic/data_carrier.h"

namespace kglt {

class WindowBase;
class Stage;
class ScreenManager;

class BackgroundLoadException : public std::runtime_error {
public:
    BackgroundLoadException():
        std::runtime_error("An error occurred while running a background task") {}
};

class Application :
    public ScreenManagerInterface {
public:
    Application(const unicode& title=_u("KGLT Application"),
        uint32_t width=1366,
        uint32_t height=768,
        uint32_t bpp=0,
        bool fullscreen=false);

    //Create the window, start do_initialization in a thread, show the loading screen
    //when thread completes, hide the loading screen and run the main loop
    int32_t run();

    Property<Application, WindowBase> window = {this, &Application::window_ };
    Property<Application, generic::DataCarrier> data = { this, &Application::data_carrier_ };

    bool initialized() const { return initialized_; }

    /* ScreenManager interface */
    virtual void register_screen(const std::string& route, ScreenFactory factory) { routes_->register_screen(route, factory); }
    virtual bool has_screen(const std::string& route) const { return routes_->has_screen(route); }
    virtual ScreenBasePtr resolve_screen(const std::string& route) { return routes_->resolve_screen(route); }
    virtual void activate_screen(const std::string& route) { routes_->activate_screen(route); }
    virtual void load_screen_in_background(const std::string& route, bool redirect_after=true) { routes_->load_screen_in_background(route, redirect_after); }
    virtual void unload_screen(const std::string& route) { routes_->unload_screen(route); }
    virtual bool is_screen_loaded(const std::string& route) const { return routes_->is_screen_loaded(route); }
    virtual ScreenBasePtr active_screen() const { return routes_->active_screen(); }
    const std::unordered_map<std::string, ScreenBasePtr> routes() const override { return routes_->routes(); }
    /* End ScreenManager interface */
protected:
    StagePtr stage(StageID stage=StageID());


    bool init();
private:
    std::shared_ptr<WindowBase> window_;
    std::shared_ptr<ScreenManager> routes_;

    bool initialized_ = false;

    virtual bool do_init() = 0;
    virtual void do_step(double dt) {}
    virtual void do_post_step(double dt) {}
    virtual void do_cleanup() {}

    virtual bool while_key_pressed(SDL_Keysym key, double) { return false; }
    virtual bool on_key_press(SDL_Keysym key) { return false; }
    virtual bool on_key_release(SDL_Keysym key) { return false; }

    generic::DataCarrier data_carrier_;
};

}

/**

  USAGE:

  class MyApp: public kglt::App {

  private:
    bool do_init() {
        cube_ = stage.actor(stage()->geom_factory().new_cube(1.0));
        return true;
    }

    void do_step(float dt) {
        cube_.rotate_x(10 * dt);
    }

    void do_cleanup() { }

    Actor& cube_;
  };


  MyApp my_application;
  return my_application.run();
*/

#endif // APPLICATION_H
