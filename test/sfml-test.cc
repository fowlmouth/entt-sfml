#include "entt-sfml/entt-sfml.h"
#include <chrono>
#include <cstdlib>

struct Transform
{
  float x,y, radians;
};

struct Velocity
{
  float x,y;
};

struct Shape
{
  std::unique_ptr< sf::Shape > shape;
};

struct TestRegistry
: entt::registry,
  UI::RegistryMixin< TestRegistry >
{
  
  TestRegistry(sf::RenderWindow* window)
  {
    ui_init(window);
  }

};

inline float randf(float min, float max)
{
  return (float)rand() / (float)RAND_MAX * (max - min) + min;
}
inline int randi(int min, int max)
{
  return (int)((float)rand() / (float)RAND_MAX * (max - min) + min);
}

auto create_a_shape(entt::registry& registry, int w, int h)
{
  auto entity = registry.create();

  auto& transform = registry.assign< Transform >(entity);
  transform.x = randf(0, w);
  transform.y = randf(0, h);
  transform.radians = randf(0, M_PI * 2);

  auto& velocity = registry.assign< Velocity >(entity);
  velocity.x = randf(-50, 50);
  velocity.y = randf(-50, 50);

  auto& shape = registry.assign< Shape >(entity);
  if(randf(0.0, 1.0) < 0.5)
    shape.shape = std::make_unique< sf::CircleShape >(randf(5, 25));
  else
  {
    sf::Vector2f size( randf(5, 25), randf(15, 30) );
    shape.shape = std::make_unique< sf::RectangleShape >(size);
  }

  static sf::Color colors[4] = {
    sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow
  };
  shape.shape->setFillColor(colors[ randi(0, 3) ]);

  return entity;
}

int main()
{
  const int w = 800, h = 600;

  sf::RenderWindow window(sf::VideoMode(w,h), "test");
  window.setFramerateLimit(60);

  TestRegistry registry(&window);

  for(int i = 0; i < 10; ++i)
    create_a_shape(registry, w, h);

  auto last_tick = std::chrono::high_resolution_clock::now();

  while(window.isOpen())
  {
    sf::Event event;
    while(window.pollEvent(event))
    {
      switch(event.type)
      {
      case sf::Event::Closed:
        window.close();
        break;

      case sf::Event::KeyPressed:
        if(event.key.code == sf::Keyboard::Escape)
          window.close();
        break;

      default:
        break;
      }
    }

    // Update stage
    auto time_now = std::chrono::high_resolution_clock::now();
    auto time_delta = std::chrono::duration_cast< std::chrono::milliseconds >(
      time_now - last_tick );
    last_tick = time_now;

    float time_delta_float = time_delta.count() / 1000.0;

    registry.view< Transform, Velocity >().each(
      [&](auto entity, auto& transform, auto& velocity)
      {
        transform.x += velocity.x * time_delta_float;
        transform.y += velocity.y * time_delta_float;
        if(transform.x < 0 || transform.x > w)
        {
          velocity.x *= -1;
          transform.x += velocity.x * 2 * time_delta_float;
        }
        if(transform.y < 0 || transform.y > h)
        {
          velocity.y *= -1;
          transform.y += velocity.y * 2 * time_delta_float;
        }
      }
    );

    registry.view< Transform, Shape >().each(
      [&](auto entity, auto& transform, auto& shape)
      {
        shape.shape->setPosition(transform.x, transform.y);
        shape.shape->setRotation(transform.radians * 180 / M_PI);
        registry.ctx< entt::dispatcher >()
          .enqueue(UI::RenderDrawableEvent{ shape.shape.get() });
      }
    );

    window.clear(sf::Color::Black);
    registry.ctx< entt::dispatcher >().update< UI::RenderDrawableEvent >();
    window.display();
  }
  
  return 0;
}
