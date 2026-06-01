// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
  TimedDoor* door;

  void SetUp() override {
    door = new TimedDoor(0);
  }

  void TearDown() override {
    delete door;
  }
};

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
  EXPECT_EQ(door->getTimeOut(), 0);
}

TEST_F(TimedDoorTest, DoorInitiallyClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, ThrowStateNoThrowWhenClosed) {
  EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, ThrowStateThrowsWhenOpen) {
  try { door->unlock(); } catch (const std::runtime_error&) {}
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, UnlockThrowsIfDoorStaysOpen) {
  EXPECT_THROW(door->unlock(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorIsOpenAfterUnlockThrows) {
  EXPECT_THROW(door->unlock(), std::runtime_error);
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenOpen) {
  try { door->unlock(); } catch (const std::runtime_error&) {}
  DoorTimerAdapter adapter(*door);
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutNoThrowWhenClosed) {
  DoorTimerAdapter adapter(*door);
  EXPECT_NO_THROW(adapter.Timeout());
}

TEST_F(TimedDoorTest, TimerCallsClientTimeout) {
  MockTimerClient mockClient;
  EXPECT_CALL(mockClient, Timeout()).Times(1);
  Timer timer;
  timer.tregister(0, &mockClient);
}

TEST_F(TimedDoorTest, CloseBeforeTimeoutPreventsException) {
  TimedDoor timedDoor(1);
  std::thread closer([&timedDoor]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    timedDoor.lock();
  });
  EXPECT_NO_THROW(timedDoor.unlock());
  closer.join();
}

TEST(MockDoorTest, LockIsCalled) {
  MockDoor mockDoor;
  EXPECT_CALL(mockDoor, lock()).Times(1);
  mockDoor.lock();
}

TEST(MockDoorTest, UnlockIsCalled) {
  MockDoor mockDoor;
  EXPECT_CALL(mockDoor, unlock()).Times(1);
  mockDoor.unlock();
}

TEST(MockDoorTest, IsOpenedReturnsValue) {
  MockDoor mockDoor;
  EXPECT_CALL(mockDoor, isDoorOpened())
      .WillOnce(::testing::Return(true));
  EXPECT_TRUE(mockDoor.isDoorOpened());
}
