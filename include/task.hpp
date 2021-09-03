#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <optional>
#include <functional>
#include <condition_variable>

namespace work
{
    class Task
    {
        enum class State : char
        {
            kEnd,
            kPaused,
            kRunning,
        };

    public:
        Task();
        virtual ~Task();

        auto start() -> std::optional<bool>;
        auto stop() -> std::optional<bool>;
        auto pause() -> std::optional<bool>;
        auto resume() -> std::optional<bool>;

    protected:
        virtual auto threadProcess() -> void = 0;
        auto isRunning() -> bool { return m_curState == State::kRunning; }

    private:
        auto run() -> void;

    private:
        State m_curState;
        std::atomic_bool m_pauseFlag;
        std::atomic_bool m_endFlag;
        std::unique_ptr<std::thread> m_spTask;
        std::mutex m_lock;
        std::condition_variable m_cv;
    };
}
