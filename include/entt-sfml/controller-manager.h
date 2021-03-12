#pragma once

#include "controllers/controller.h"
#include <iostream>

namespace UI
{
  struct Controller
  {
    std::shared_ptr< ::UI::Controllers::Controller > controller = nullptr;

    Controller(Controller&& i)
    : controller(std::move(i.controller))
    {}

    Controller()
    {
    }

    ~Controller()
    {
      release();
    }

    Controller& operator= (Controller&& i)
    {
      controller = std::move(i.controller);
      return *this;
    }

    void release()
    {
      if(controller)
      {
        controller->release();
        controller = nullptr;
      }
    }
  };

  void on_remove_controller(entt::registry& r, entt::entity entity)
  {
    Controller& c = r.get<Controller>(entity);
    c.release();
  }




  struct ControllerManager
  {
    using ControllerReference = std::shared_ptr< UI::Controllers::Controller >;

    std::unordered_map< std::string, ControllerReference > controllers;

    void add_controller(const std::string& name, const ControllerReference& controller)
    {
      controllers[name] = controller;
    }
    
    void update(entt::registry& r, std::chrono::milliseconds dt)
    {
      for(const auto& controller_pair : controllers)
      {
        auto& ctr = controller_pair.second;
        if(ctr && r.valid(ctr->entity))
          ctr->update(r, dt);
      }
    }

    bool take_controller(const std::string& name, entt::registry& r, entt::entity entity)
    {
      if(! r.valid(entity))
        return false;

      auto iter = controllers.find(name);
      if(iter == controllers.end())
      {
        std::cout << "Controller not found: " << name << std::endl;
        for(const auto& item : controllers)
          std::cout << "  " << item.first << std::endl;
        
        return false;
      }

      auto& ctrl = iter->second;
      ctrl->entity = entity;

      r.get_or_emplace< UI::Controller >(entity).controller = ctrl;

      return true;
    }
  };

  template<typename Registry>
  struct UpdateControllers
  {
    void operator() (Registry& registry, std::chrono::milliseconds dt)
    {
      auto& man = registry.template ctx< ControllerManager >();
      man.update(registry, dt);
    }
  };



}