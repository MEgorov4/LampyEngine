#include <gtest/gtest.h>

#include <Modules/ScriptModule/ScriptVM.h>
#include <Modules/ScriptModule/IScriptRegister.h>

namespace
{
class DummyRegister final : public ScriptModule::IScriptRegister
{
  public:
    void registerTypes(sol::state&, sol::environment& env) override
    {
        env.set_function("DummyFn", []() { return 42; });
    }
};

ScriptModule::ScriptVMProfile MakeProfile()
{
    ScriptModule::ScriptVMProfile profile;
    profile.name       = "TestVM";
    profile.libraries  = {sol::lib::base, sol::lib::os};
    profile.registerFactories = {[] { return std::make_shared<DummyRegister>(); }};
    return profile;
}
} // namespace

TEST(ScriptVMTests, RegistersExposeFunctions)
{
    ScriptModule::ScriptVM vm("Test");
    auto profile = MakeProfile();
    vm.configure(profile);
    ASSERT_TRUE(vm.init());
    sol::load_result chunk = vm.state().load("return DummyFn()");
    ASSERT_TRUE(chunk.valid());
    sol::function func = chunk;
    sol::set_environment(vm.environment(), func);
    sol::protected_function protectedFunc = func;
    auto result = protectedFunc();
    ASSERT_TRUE(result.valid());
    int value = result.get<int>();
    EXPECT_EQ(value, 42);
}

