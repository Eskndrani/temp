#ifndef TIMER_H
#define TIMER_H

#include <QGraphicsTextItem>
#include <QFont>
#include <QTimer>

class Timer : public QGraphicsTextItem
{
    Q_OBJECT
    
public:
    Timer(QGraphicsItem *parent = nullptr);
    ~Timer();
    
    void start();
    void pause();
    void reset();
    int getTimeLeft() const;
    void tick();
    bool isTimeUp() const;
    
private:
    void updateTimerDisplay();
    void triggerTimeoutEvent();
    
    int m_timeLeft;
    int m_maxTime;
    bool m_isRunning;
    QFont m_font;
};

#endif