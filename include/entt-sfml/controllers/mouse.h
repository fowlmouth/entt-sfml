#pragma once

#include "controller.h"
#include "../events.h"

namespace UI::Controllers
{

struct Mouse : Controller
{
  static const std::unordered_map< sf::Mouse::Button, std::string > button_to_name;
  static const std::unordered_map< std::string, sf::Mouse::Button > name_to_button;

  std::unordered_map< sf::Mouse::Button, std::string > controls;

  Mouse()
  : Controller()
  {
  }

  void update(entt::registry& r, std::chrono::milliseconds dt)
  {
    auto& dispatcher = r.ctx< entt::dispatcher >();
    for(const auto& ctrl : controls)
    {
      if(sf::Mouse::isButtonPressed(ctrl.first))
      {
        dispatcher.enqueue(ControllerInputEvent{ entity, ctrl.second, dt });
        // Interactors::activate_firegroup(r, entity, ctrl.second, dt, 1.0);
      }
    }
  }

  static std::shared_ptr< Mouse > create(const std::unordered_map< std::string, std::string >& map)
  {
    // Does not set the `name` field
    auto controller = std::make_shared< Mouse >();
    for(const auto& item : map)
    {
      const auto& key = item.first;
      const auto& input = item.second;

      auto sf_key = name_to_button.find(key);
      if(sf_key == name_to_button.end())
        continue;

      controller->controls[sf_key->second] = input;
    }
    return controller;
  }
};


const std::unordered_map< sf::Mouse::Button, std::string > Mouse::button_to_name{
  {sf::Mouse::Left, "Left"}, {sf::Mouse::Right, "Right"}, {sf::Mouse::Middle, "Middle"},
  {sf::Mouse::XButton1, "XButton1"}, {sf::Mouse::XButton2, "XButton2"}
};
const std::unordered_map< std::string, sf::Mouse::Button > Mouse::name_to_button{
  {"Left", sf::Mouse::Left}, {"Right", sf::Mouse::Right}, {"Middle", sf::Mouse::Middle},
  {"XButton1", sf::Mouse::XButton1}, {"XButton2", sf::Mouse::XButton2}
};


} // ::Input::Controllers
