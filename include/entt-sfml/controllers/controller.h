#pragma once

#include <string>
#include <iostream>

namespace UI
{
namespace Controllers
{

struct Controller
{
  std::string name;
  entt::entity entity = entt::null;
  unsigned joystick_id = 0;

  virtual ~Controller()
  {
    std::cout << "destroy controller " << name << std::endl;
  }

  virtual void update(entt::registry&, std::chrono::milliseconds) = 0;

  void release()
  {
    entity = entt::null;
  }
};

} // ::Input::Controllers
} // ::Input
