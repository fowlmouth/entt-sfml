#pragma once

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
    sf::Drawable* drawable;
  };

}