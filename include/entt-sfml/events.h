#pragma once

#include <memory>
#include <chrono>
#include <string>

namespace UI
{

  struct ControllerInputEvent
  {
    entt::entity entity;
    std::string input;
    std::chrono::milliseconds dt;
  };


  struct RenderDrawableEvent
  {
    std::unique_ptr< sf::Drawable > drawable;
  };

}
