#include <gtest/gtest.h>

#include <Modules/ScriptModule/IScriptRegister.h>
#include <Modules/ScriptModule/ScriptVMManager.h>
#include <Modules/ScriptModule/ScriptVMProfile.h>

#include <stdexcept>

namespace
{
class ThrowingRegister final : public ScriptModule::IScriptRegister
{
  public:
    void registerTypes(sol::state&, sol::environment& env) override
    {
        env.set_function("ThrowCpp", []() -> int {
            throw std::runtime_error("Throwing from C++");
        });
    }
};
} // namespace

class ScriptCommandRunner
{
  public:
    explicit ScriptCommandRunner(bool withThrowingRegister = false)
    {
        ScriptModule::ScriptVMProfileMap profiles;
        ScriptModule::ScriptVMProfile devProfile;
        devProfile.name      = "DevTest";
        devProfile.libraries = {sol::lib::base, sol::lib::math, sol::lib::string};
        if (withThrowingRegister)
        {
            devProfile.registerFactories.push_back([] { return std::make_shared<ThrowingRegister>(); });
        }
        profiles[ScriptModule::ScriptVMType::Dev] = devProfile;

        m_manager.initializeAll(profiles);
    }

    ~ScriptCommandRunner()
    {
        m_manager.shutdownAll();
    }

    bool processCommand(const std::string& command)
    {
        ScriptModule::ScriptVM* target = m_manager.devVM();
        if (!target)
            target = m_manager.runtimeVM();
        if (!target)
            return false;
        return target->runString(command);
    }

  private:
    ScriptModule::ScriptVMManager m_manager;
};

TEST(ScriptProcessCommandTests, ValidCommandReturnsTrue)
{
    ScriptCommandRunner runner;
    EXPECT_TRUE(runner.processCommand("return 2 + 2"));
}

TEST(ScriptProcessCommandTests, InvalidSyntaxReturnsFalseButDoesNotCrash)
{
    ScriptCommandRunner runner;
    EXPECT_FALSE(runner.processCommand("return function("));
}

TEST(ScriptProcessCommandTests, HandlesNonAsciiInputGracefully)
{
    ScriptCommandRunner runner;
    EXPECT_FALSE(runner.processCommand("ж"));
}

TEST(ScriptProcessCommandTests, CppExceptionInFunctionIsHandled)
{
    ScriptCommandRunner runner(true);
    EXPECT_FALSE(runner.processCommand("return ThrowCpp()"));
}

