# Clean Shutdown Checklist

Use this checklist when adding modules or background work that run outside of the main tick loop.

1. **Threads / Async tasks**
   - Wrap every `std::thread` in a helper that can `requestStop()` and `join()` during module shutdown.
   - Store opaque handles when registering work with `TimeScheduler`. Cancel those handles from `shutdown()`.

2. **Asset / Resource watchers**
   - Call `AssetManager::watchDirectory()` only for long-lived paths.
   - Remove every watch ID and join the watcher thread before destroying the manager.

3. **Scheduler tasks**
   - Use `TimeScheduler::TaskId` and cancel it explicitly.
   - Avoid capturing `this` unless the module controls task lifetime.

4. **Diagnostics**
   - Use `EngineCore::Foundation::Diagnostics::LogActiveThreads()` after shutdown work to confirm no extra threads remain.
   - Inspect the log for `MemorySystem` leak warnings; they list remaining bytes per tag.

5. **Testing**
   - Add a regression test (e.g., GoogleTest) when introducing new shutdown-sensitive code.
   - The test should start the module, trigger shutdown, and assert that all handles resolve within a timeout.


