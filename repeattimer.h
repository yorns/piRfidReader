#ifndef REPEATTIMER_H
#define REPEATTIMER_H

#include <functional>
#include <chrono>
#include <boost/asio/steady_timer.hpp>

//! traditional timer to call a handler repeatly
class RepeatTimer {

    boost::asio::steady_timer m_timer;
    std::chrono::milliseconds m_duration;
    std::function<void()> m_handler;

public:
    RepeatTimer(boost::asio::io_context& context,
                const std::chrono::milliseconds& duration)
        : m_timer(context), m_duration(duration) {
    }

    void start() {
        m_timer.expires_after(m_duration);
        m_timer.async_wait([this](const boost::system::error_code& error){
            if(!error && m_handler) { m_handler(); start(); } });
    }

    void start(const std::chrono::milliseconds& duration) {
        m_duration = duration;
        start();
    }

    void stop() { m_timer.cancel(); }

    void setHandler(std::function<void()>&& handler)
    { m_handler = std::move(handler); }

    void resetHandler() { m_handler = nullptr; }

};

#endif // REPEATTIMER_H
