#include "timer.h"
#include <QDebug>

Timer::Timer(QGraphicsItem *parent)
    : QGraphicsTextItem(parent),
      m_timeLeft(400),
      m_maxTime(400),
      m_isRunning(false)
{
    m_font = QFont("Super Mario 256", 12, QFont::Bold);
    setFont(m_font);
    setDefaultTextColor(Qt::white);
    updateTimerDisplay();
}

Timer::~Timer()
{
}

void Timer::start()
{
    m_isRunning = true;
}

void Timer::pause()
{
    m_isRunning = false;
}

void Timer::reset()
{
    m_timeLeft = m_maxTime;
    m_isRunning = false;
    updateTimerDisplay();
}

int Timer::getTimeLeft() const
{
    return m_timeLeft;
}

void Timer::tick()
{
    if (m_isRunning && m_timeLeft > 0) {
        m_timeLeft--;
        updateTimerDisplay();
        
        if (m_timeLeft <= 0) {
            triggerTimeoutEvent();
        }
    }
}

bool Timer::isTimeUp() const
{
    return m_timeLeft <= 0;
}

void Timer::updateTimerDisplay()
{
    setPlainText("TIME: " + QString::number(m_timeLeft));
}

void Timer::triggerTimeoutEvent()
{
    m_isRunning = false;
}