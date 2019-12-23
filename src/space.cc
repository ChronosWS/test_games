#include <cassert>
#include <iostream>

#include "camera.cc"
#include "command.cc"
#include "ecs.h"
#include "platform/window.cc"
#include "platform/platform.cc"
#include "game/game.cc"
#include "gfx.cc"
#include "math/math.cc"
#include "renderer/renderer.cc"

#include "game/event_buffer.h"
#include "game/game.h"
#include "math/vec.h"

bool
Initialize()
{
  if (!gfx::Initialize()) return false;
  // Make a square. This thing moves around when clicking.
  auto* transform = kECS.Assign<TransformComponent>(0);
  transform->position = math::Vec3f(400.f, 400.f, 0.f);
  kECS.Assign<RectangleComponent>(0);

  // Make a triangle. This doesn't really do anything.
  transform = kECS.Assign<TransformComponent>(1);
  transform->position = math::Vec3f(200.f, 200.f, 0.f);
  transform->orientation.Set(0.f, math::Vec3f(0.f, 0.f, 1.f));
  kECS.Assign<TriangleComponent>(1);

  camera::MoveTo(math::Vec3f(0.f, 0.f, 0.f));
  camera::AimAt(math::Vec3f(0.f, 0.f, -1.f));

  return true;
}

bool
ProcessInput()
{
  PlatformEvent event;
  while (window::PollEvent(&event)) {
    switch (event.type) {
      case MOUSE_DOWN: {
        if (event.button == BUTTON_LEFT) {
          command::Move* move = game::CreateEvent<command::Move>(command::MOVE);
          move->entity_id = 0;
          // A bit of an optimization. Assume no zoom when converting to world space.
          move->position = event.position;
        }
      } break;
      default: break;
    }
  }

  return true;
}

void
HandleEvent(game::Event event)
{
  switch ((command::Event)event.metadata) {
    case command::MOVE:
      command::Execute(*((command::Move*)event.data));
      break;
    case command::INVALID:
    default:
      assert("Invalid command.");
  }
}

bool
UpdateGame()
{
  TransformComponent* transform = kECS.Get<TransformComponent>(0);
  DestinationComponent* destination = kECS.Get<DestinationComponent>(0);
  if (destination) {
    auto dir =
        math::Normalize(destination->position - transform->position.xy());
    transform->position += dir * 1.f;
    float length_squared =
        math::LengthSquared(transform->position.xy() - destination->position);
    // Remove DestinationComponent so entity stops.
    if (length_squared < 15.0f) {
      kECS.Remove<DestinationComponent>(0);
    }
  }

  return true;
}

void
OnEnd()
{
}

// namespace

int
main(int argc, char** argv)
{
  game::Setup(&Initialize, &ProcessInput, &HandleEvent, &UpdateGame,
              &gfx::Render, &OnEnd);

  if (!game::Run()) {
    std::cerr << "Encountered error running spacey game..." << std::endl;
  }

  return 0;
}
