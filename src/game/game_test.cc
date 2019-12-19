#include <iostream>

#include "platform/unix_filesystem.cc"
#include "game.cc"
#include "gtest/gtest.h"

bool
DumbInitialize()
{
  return true;
}

bool
DumbProcessInput()
{
  return true;
}

void
DumbHandleEvent(game::Event event)
{
}

bool
DumbUpdate()
{
  return true;
}

bool
DumbRender()
{
  return true;
}

void
DumbEnd()
{
}

// namespace

TEST(Game, HappyPathGameTest)
{
  game::Setup(&DumbInitialize, &DumbProcessInput, &DumbHandleEvent, &DumbUpdate,
              &DumbRender, &DumbEnd);
  uint64_t count = 500;
  game::Run(count);
  ASSERT_EQ(game::Updates(), count);
  // Total game time should be game updates * milliseconds_per_update.
  // 500 * 15 = 7500
  ASSERT_EQ(game::Time(), std::chrono::milliseconds(7500));
}

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}