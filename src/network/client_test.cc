#include "gtest/gtest.h"

#include <iostream>

#include "client.cc"
#include "platform/platform.cc"
#include "server.cc"

static int kCallbacksCalled = 0;

void
OnClientConnected(int client_id)
{
  std::cout << "Client " << client_id << " connected..." << std::endl;
  ASSERT_EQ(client_id, 0);
  ++kCallbacksCalled;
}

void
OnServerMsgReceived(int client_id, uint8_t* data, int size)
{
  std::cout << "Client " << client_id << " sent msg..." << std::endl;
  ASSERT_EQ(size, 5);
  ASSERT_EQ(strncmp((char*)data, "hello", size), 0);
  ++kCallbacksCalled;
}

void
OnClientMsgReceived(uint8_t* data, int size)
{
  std::cout << "Server sent msg..." << std::endl;
  ++kCallbacksCalled;
}

TEST(Server, ClientHappyPath)
{
  network::server::Setup(&OnClientConnected, &OnServerMsgReceived);
  ASSERT_TRUE(network::server::Start("7890"));

  platform::sleep_ms(1000);

  network::client::Setup(&OnClientMsgReceived);
  ASSERT_TRUE(network::client::Start("127.0.0.1", "7890"));

  char msg[6] = "hello";

  // Client says hi to server!
  network::client::Send((uint8_t*)&msg[0], 5);
  platform::sleep_ms(1000);

  // Server says hi to client!
  network::server::Send(0, (uint8_t*)&msg[0], 5);
  platform::sleep_ms(1000);

  ASSERT_EQ(kCallbacksCalled, 3);

  network::server::Stop();
  network::client::Stop();
}

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
