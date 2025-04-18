#include "effect.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>

Effect::Effect(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_effectType("none"),
      m_intensity(5),
      m_fadeIn(false),
      m_fadeOut(false),
      m_duration(1000),
      m_elapsed(0),
      m_isPlaying(false)
{
    // Create a timer for animation updates
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Effect::updateEffect);
}

Effect::~Effect()
{
    // The timer is parented to this object, so it will be deleted automatically
}

void Effect::playEffect()
{
    m_isPlaying = true;
    m_elapsed = 0;
    updateEffectParameters();
    update();
    
    // Start the timer to update the effect every 16ms (approx 60fps)
    m_timer->start(16);
}

void Effect::updateEffect()
{
    m_elapsed += 16;
    if (m_elapsed < m_duration) {
        updateEffectParameters();
        update();
    } else {
        m_isPlaying = false;
        m_timer->stop();
    }
}

void Effect::setIntensity(int intensity)
{
    m_intensity = intensity;
}

void Effect::fadeIn()
{
    m_fadeIn = true;
    m_fadeOut = false;
}

void Effect::fadeOut()
{
    m_fadeIn = false;
    m_fadeOut = true;
}

void Effect::setEffectType(const QString& type)
{
    m_effectType = type;
    initializeEffect();
}

QRectF Effect::boundingRect() const
{
    return QRectF(-50, -50, 100, 100);
}

void Effect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!m_isPlaying) {
        return;
    }
    
    qreal opacity = 1.0;
    if (m_fadeIn) {
        opacity = static_cast<qreal>(m_elapsed) / m_duration;
    } else if (m_fadeOut) {
        opacity = 1.0 - static_cast<qreal>(m_elapsed) / m_duration;
    }
    
    painter->setOpacity(opacity);
    
    if (m_effectType == "explosion") {
        int size = m_intensity * 5;
        painter->setBrush(QBrush(QColor(255, 100, 0, 200)));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(-size/2, -size/2, size, size);
    } else if (m_effectType == "sparkle") {
        int size = m_intensity * 2;
        painter->setBrush(QBrush(QColor(255, 255, 0, 200)));
        painter->setPen(Qt::NoPen);
        
        for (int i = 0; i < 8; i++) {
            qreal angle = i * M_PI / 4;
            QPointF point(size * cos(angle), size * sin(angle));
            painter->drawEllipse(point, 2, 2);
        }
    } else if (m_effectType == "smoke") {
        int size = m_intensity * 3;
        painter->setBrush(QBrush(QColor(200, 200, 200, 100)));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(-size/2, -size/2, size, size);
    }
}

void Effect::initializeEffect()
{
    if (m_effectType == "explosion") {
        m_duration = 500;
        m_intensity = 10;
    } else if (m_effectType == "sparkle") {
        m_duration = 800;
        m_intensity = 5;
    } else if (m_effectType == "smoke") {
        m_duration = 1500;
        m_intensity = 8;
        fadeOut();
    }
}

void Effect::updateEffectParameters()
{
    if (m_effectType == "explosion") {
        m_intensity = 10 + (m_elapsed * 20) / m_duration;
    } else if (m_effectType == "sparkle") {
        qreal progress = static_cast<qreal>(m_elapsed) / m_duration;
        m_intensity = 5 + 3 * sin(progress * 2 * M_PI);
    } else if (m_effectType == "smoke") {
        m_intensity = 8 + (m_elapsed * 15) / m_duration;
        setPos(pos().x(), pos().y() - 0.5);
    }
}
