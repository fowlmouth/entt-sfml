#pragma once

#include "asset-cache.h"
#include "controller-manager.h"
#include "controllers/keyboard.h"
#include "events.h"

#ifdef FOWL_ENTT_MRUBY
# include <mruby/proc.h>
# include <mruby/hash.h>
# include "mruby-bindings.h"
#endif

namespace UI
{


struct FontCache : AssetCache< sf::Font, FontCache >
{
  std::unique_ptr< sf::Font > load(const std::string& file)
  {
    sf::Font font;
    if(! font.loadFromFile(file))
      return nullptr;
    return std::make_unique< sf::Font >(font);
  }
};

struct TextureCache : AssetCache< sf::Texture, TextureCache >
{
  std::unique_ptr< sf::Texture > load(const std::string& file)
  {
    sf::Texture tex;
    if(!tex.loadFromFile(file))
      return nullptr;
    return std::make_unique< sf::Texture >(tex);
  }
};

template< typename Derived >
struct RegistryMixin
{
  using Self = RegistryMixin< Derived >;
  Derived& derived() { return *static_cast< Derived* >(this); }

  void ui_render_drawable(const RenderDrawableEvent& event)
  {
    auto& registry = derived();
    auto window = registry.template ctx< sf::RenderWindow* >();
    window->draw(*event.drawable);
  }

  UI::FontCache& ui_font_cache()
  {
    return derived().template ctx< UI::FontCache >();
  }

  UI::TextureCache& ui_texture_cache()
  {
    return derived().template ctx< UI::TextureCache >();
  }

  sf::RenderWindow* ui_render_window()
  {
    return derived().template ctx< sf::RenderWindow* >();
  }

  UI::ControllerManager& ui_controller_manager()
  {
    return derived().template ctx< UI::ControllerManager >();
  }

  void ui_init(sf::RenderWindow* window)
  {
    derived().template set< sf::RenderWindow* >(window);

    if(! derived().template try_ctx< entt::dispatcher >())
      derived().template set< entt::dispatcher >();
    
    derived().template ctx< entt::dispatcher >()
      .template sink< RenderDrawableEvent >()
      .template connect< &Self::ui_render_drawable >(this);

    derived().template set< UI::FontCache >();
    derived().template set< UI::TextureCache >();
    derived().template set< UI::ControllerManager >();

    derived().template on_destroy< UI::Controller >()
      .template connect< &UI::on_remove_controller >();
  }

#ifdef FOWL_ENTT_MRUBY

  using MRubyRegistryMixin = MRuby::RegistryMixin< Derived >;

  std::unordered_map< std::string, RProc* > ui_mrb_controller_handlers;

  static mrb_value ui_mrb_registry_set_controller_callback(mrb_state* mrb, mrb_value self)
  {
    char* control;
    mrb_value block;
    mrb_int args_read = mrb_get_args(mrb, "z&", &control, &block);
    if(args_read != 1 || mrb_nil_p(block))
      return mrb_nil_value();
    std::string control_str(control);
    
    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto& handlers = registry->ui_mrb_controller_handlers;
    auto iter = handlers.find(control_str);
    if(iter != handlers.end())
    {
      RProc* old_handler = iter->second;
      mrb_value old_handler_obj = mrb_obj_value(old_handler);
      mrb_gc_unregister(mrb, old_handler_obj);
    }

    mrb_gc_register(mrb, block);
    handlers[control_str] = mrb_proc_ptr(block);

    return self;
  }

  static mrb_value ui_mrb_registry_take_controller(mrb_state* mrb, mrb_value self)
  {
    char* controller_name;
    mrb_int entity;
    if(mrb_get_args(mrb, "iz", &entity, &controller_name) != 2)
      return mrb_nil_value();
    std::string controller(controller_name);

    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto& controller_man = registry->ui_controller_manager();
    if(controller_man.take_controller(controller, *registry, (entt::entity)entity))
      return mrb_true_value();
    return mrb_false_value();
  }

  static mrb_value ui_mrb_registry_controllers(mrb_state* mrb, mrb_value self)
  {
    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();
    auto& controller_manager = registry->ui_controller_manager();

    std::size_t num_controllers = controller_manager.controllers.size();
    mrb_value controllers[num_controllers];

    auto iter = controller_manager.controllers.begin();
    for(std::size_t i = 0; i < num_controllers && iter != controller_manager.controllers.end(); ++i, ++iter)
    {
      const auto& ctrl = iter->second;
      controllers[i] = mrb_str_new_cstr(mrb, ctrl->name.c_str());
    }

    return mrb_ary_new_from_values(mrb, num_controllers, controllers);
  }

  static mrb_value ui_mrb_registry_close_window(mrb_state* mrb, mrb_value self)
  {
    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto window = registry->ui_render_window();
    if(!window)
      return mrb_nil_value();

    window->close();
    return mrb_true_value();
  }

  static mrb_value ui_mrb_registry_window_size(mrb_state* mrb, mrb_value self)
  {
    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto window = registry->ui_render_window();
    if(!window)
      return mrb_nil_value();

    auto window_size = window->getSize();
    mrb_value array_values[2] = {
      mrb_fixnum_value(window_size.x),
      mrb_fixnum_value(window_size.y)
    };

    return mrb_ary_new_from_values(mrb, 2, array_values);
  }

  static mrb_value ui_mrb_registry_set_window_size(mrb_state* mrb, mrb_value self)
  {
    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto window = registry->ui_render_window();
    if(!window)
      return mrb_nil_value();

    mrb_int width, height;
    if(mrb_get_args(mrb, "ii", &width, &height) != 2)
      return mrb_nil_value();

    sf::Vector2u new_size(width, height);
    window->setSize(new_size);
    return mrb_true_value();
  }

  static mrb_value ui_mrb_registry_create_controller(mrb_state* mrb, mrb_value self)
  {
    // Expects 3 args, a string controller name, string controller type, and a hash map
    char *ctrl_name, *ctrl_type;
    mrb_value ctrl_keys;
    if(mrb_get_args(mrb, "zzH", &ctrl_name, &ctrl_type, &ctrl_keys) != 3)
      return mrb_nil_value();

    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    using userdata_type = std::unordered_map< std::string, std::string >;
    userdata_type ctrl_map;

    mrb_hash_foreach_func* fn = [](mrb_state* mrb, mrb_value key, mrb_value value, void* ud) -> int
    {
      userdata_type& ctrl_map = *(userdata_type*)ud;
      std::string key_ = mrb_string_value_cstr(mrb, &key);
      std::string value_ = mrb_string_value_cstr(mrb, &value);
      ctrl_map[key_] = value_;
      return 0;
    };
    mrb_hash_foreach(mrb, mrb_hash_ptr(ctrl_keys), fn, &ctrl_map);

    std::shared_ptr< UI::Controllers::Controller > controller;
    if(strcmp(ctrl_type, "keyboard") == 0)
      controller = UI::Controllers::Keyboard::create(ctrl_map);

    if(! controller)
      return mrb_nil_value();

    controller->name = ctrl_name;

    auto& controller_manager = registry->ui_controller_manager();
    controller_manager.controllers[ctrl_name] = controller;

    return mrb_str_new_cstr(mrb, ctrl_name);
  }

  static mrb_value ui_mrb_draw(mrb_state* mrb, mrb_value self)
  {
    mrb_value hash;
    if(mrb_get_args(mrb, "H", &hash) != 1)
      return mrb_nil_value();

    Derived* registry = Derived::mrb_value_to_registry(mrb, self);
    if(!registry)
      return mrb_nil_value();

    auto& dispatcher = registry->template ctx< entt::dispatcher >();

    std::string str;
    sf::Vector2f vfval;
    sf::Color cval;
    float fval;
    MRuby::HashReader reader(mrb, hash);

    auto set_styles = [&](auto& resource){
      if(reader.read_hash("outline_color", cval))
        resource.setOutlineColor(cval);
      if(reader.read_hash("outline_thickness", fval))
        resource.setOutlineThickness(fval);
      if(reader.read_hash("fill_color", cval))
        resource.setFillColor(cval);
      if(reader.read_hash("origin", vfval))
        resource.setOrigin(vfval);
      if(reader.read_hash("scale", vfval))
        resource.setScale(vfval);
      if(reader.read_hash("x", fval))
        resource.setPosition(fval, resource.getPosition().y);
      if(reader.read_hash("y", fval))
        resource.setPosition(resource.getPosition().x, fval);
    };

    if(reader.read_hash("shape", str))
    {
      std::unique_ptr< sf::Shape > shape;
      if(str == "circle")
        shape = std::make_unique< sf::CircleShape >(
          reader.read_default("radius", 5.f));
      else if(str == "rect")
        shape = std::make_unique< sf::RectangleShape >(sf::Vector2f(
          reader.read_default("width", 10.f),
          reader.read_default("height", 5.f)));
      else
        return mrb_nil_value();

      set_styles(*shape);
      // if(reader.read_hash("outline_color", cval))
      //   shape->setOutlineColor(cval);
      // if(reader.read_hash("outline_thickness", fval))
      //   shape->setOutlineThickness(fval);
      // if(reader.read_hash("fill_color", cval))
      //   shape->setFillColor(cval);
      // if(reader.read_hash("origin", vfval))
      //   shape->setOrigin(vfval);
      // if(reader.read_hash("scale", vfval))
      //   shape->setScale(vfval);
      // if(reader.read_hash("x", fval))
      //   shape->setPosition(fval, shape->getPosition().y);
      // if(reader.read_hash("y", fval))
      //   shape->setPosition(shape->getPosition().x, fval);

      dispatcher.template enqueue< UI::RenderDrawableEvent >({ std::move(shape) });
      return reader.self;
    }
    if(reader.read_hash("text", str))
    {
      std::unique_ptr< sf::Text > text = std::make_unique< sf::Text >();
      text->setString(str);

      if(reader.read_hash("font", str))
      {
        auto font = registry->template ctx< UI::FontCache >().get(str);
        if(font)
          text->setFont(*font);
        else
          std::cout << "Unknown font " << str << std::endl;
      }
      if(reader.read_hash("size", fval))
        text->setCharacterSize(fval);

      set_styles(*text);
      dispatcher.template enqueue< UI::RenderDrawableEvent >({ std::move(text) });
      return reader.self;
    }

    return mrb_nil_value();
  }

  void ui_mrb_handle_controller_input_event(const ControllerInputEvent& event)
  {
    // Handle ControllerInputEvent event
    auto iter = ui_mrb_controller_handlers.find(event.input);
    if(iter == ui_mrb_controller_handlers.end())
      return;

    auto proc = iter->second;

    mrb_state* mrb = derived().mrb;

    mrb_value argv[2]{
      mrb_gv_get(mrb, mrb_intern_lit(mrb, "$registry")),
      mrb_fixnum_value((mrb_int)event.entity)
    };

    mrb_yield_argv(mrb, mrb_obj_value(proc), 2, argv);
  }

  void ui_mrb_init(mrb_state* state)
  {
    // Set up some methods, if Derived includes MRubyRegistryMixin
    if constexpr (std::is_base_of< MRubyRegistryMixin, Derived >::value)
    {
      auto registry_class = MRuby::Class{ state, mrb_class_get(state, "Registry") };
      registry_class
        .define_method("set_controller_callback", ui_mrb_registry_set_controller_callback, MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK())
        .define_method("take_controller", ui_mrb_registry_take_controller, MRB_ARGS_REQ(2))
        .define_method("controllers", ui_mrb_registry_controllers, MRB_ARGS_REQ(0))
        .define_method("create_controller", ui_mrb_registry_create_controller, MRB_ARGS_REQ(3))
        .define_method("close_window", ui_mrb_registry_close_window, MRB_ARGS_REQ(0))
        .define_method("window_size", ui_mrb_registry_window_size, MRB_ARGS_REQ(0))
        .define_method("set_window_size", ui_mrb_registry_set_window_size, MRB_ARGS_REQ(2))
        .define_method("draw", ui_mrb_draw, MRB_ARGS_REQ(1))
      ;

      auto& dispatcher = derived().template ctx< entt::dispatcher >();
      dispatcher
        .template sink< ControllerInputEvent >()
        .template connect< & Self::ui_mrb_handle_controller_input_event >(this);
    }
  }

  void ui_mrb_cleanup() const
  {
    // Doesn't really matter since the mrb_state* gets deleted anyways
    mrb_state* mrb = derived().mrb;
    for(const auto& item : derived().ui_mrb_controller_handlers)
    {
      const auto& proc = item.second;
      if(proc)
      {
        mrb_gc_unregister(mrb, mrb_obj_value(proc));
      }
    }
  }

#endif

};

}
