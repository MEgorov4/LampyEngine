#include <gtest/gtest.h>

#include <Modules/TimeModule/TimeScheduler.h>

using namespace TimeModule;

TEST(TimeSchedulerTests, ExecutesScheduledTaskOnce)
{
    TimeScheduler scheduler;
    bool executed = false;

    auto id = scheduler.schedule([&]() { executed = true; }, 0.0);
    scheduler.update(0.0);

    EXPECT_NE(id, TimeScheduler::InvalidTaskId);
    EXPECT_TRUE(executed);
    EXPECT_EQ(scheduler.getTaskCount(), 0u);
}

TEST(TimeSchedulerTests, CancelPreventsExecution)
{
    TimeScheduler scheduler;
    bool executed = false;
    auto id = scheduler.scheduleRepeating([&]() { executed = true; }, 0.0);

    scheduler.cancel(id);
    scheduler.update(1.0);

    EXPECT_FALSE(executed);
    EXPECT_EQ(scheduler.getTaskCount(), 0u);
}


