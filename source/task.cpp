#include "task.hpp"
#include <iostream>

namespace work
{
    Task::Task() : m_curState{State::kEnd},
                   m_pauseFlag{false},
                   m_endFlag{true},
                   m_spTask{}
    {
    }

    Task::~Task()
    {
        m_curState = State::kEnd;
        m_pauseFlag.store(false);
        m_endFlag.store(true);
        m_spTask.reset();
    }

    auto Task::start() -> std::optional<bool>
    {
        if (m_curState == State::kRunning || nullptr != m_spTask.get())
        {
            return false;
        }
        m_curState = State::kRunning;
        m_endFlag.store(false);
        m_spTask.reset(new std::thread(&Task::run, this));
        return true;
    }

    auto Task::stop() -> std::optional<bool>
    {
        if (m_spTask.get() == nullptr)
        {
            return true;
        }
        m_curState = State::kEnd;
        m_endFlag.store(true);

        if (!m_spTask->joinable())
        {
            std::cout << "joinable : false\n";
            return false;
        }

        m_spTask->join();
        m_spTask.reset();
        m_cv.notify_one();
        return true;
    }

    auto Task::pause() -> std::optional<bool>
    {
        if (m_curState == State::kRunning || m_endFlag.load() == false || m_pauseFlag.load() == false || m_spTask.get() != nullptr)
        {
            m_curState = State::kPaused;
            m_pauseFlag.store(true);
            return true;
        }
        return false;
    }

    auto Task::resume() -> std::optional<bool>
    {
        if (m_curState == State::kPaused || m_endFlag.load() == false || m_pauseFlag.load() == true || m_spTask.get() != nullptr)
        {
            m_curState = State::kRunning;
            m_pauseFlag.store(false);
            m_cv.notify_one();
            return true;
        }
        return false;
    }

    auto Task::run() -> void
    {
        do
        {
            // std::cout << ((m_endFlag.load() == false) ? "running" : "stoped") << std::endl;
            this->threadProcess();

            while (m_pauseFlag.load() == true)
            {
                std::unique_lock<std::mutex> locker(m_lock);
                m_cv.wait(locker);
            }
        } while (m_endFlag.load() == false);

        m_pauseFlag.store(false);
        m_endFlag.store(true);
    }
}
