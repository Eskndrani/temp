#include "enemy.h"
#include <QDebug>
#include <QPainter>

Enemy::Enemy(QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsItem(parent),
    m_sprite(new Sprite(this)),
    m_isAlive(true),
    m_isMovingLeft(true),
    m_velocityX(-1.5),  // Start moving left
    m_velocityY(0),
    m_speed(1.5),
    m_gravity(0.5),
    m_health(1)
{
    // Set the object name for debugging
    setObjectName("Enemy");
    // Set data for collision detection
    setData(0, "enemy");
}

Enemy::~Enemy()
{
    delete m_sprite;
}

QRectF Enemy::boundingRect() const
{
    // Return bounding rect based on sprite size
    return QRectF(-16, -16, 32, 32);
}

void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_isAlive) return;



#ifdef QT_DEBUG
    painter->setPen(Qt::red);
    painter->drawRect(boundingRect());
#endif
}

void Enemy::move()
{
    if (!m_isAlive) return;

    // Basic movement
    setPos(pos().x() + m_velocityX, pos().y() + m_velocityY);

    // Apply gravity
    m_velocityY += m_gravity;

    // Update movement pattern (can be overridden by subclasses)
    updateMovementPattern();

    // Animate sprite
    m_sprite->animate();
}

void Enemy::takeDamage()
{
    if (!m_isAlive) return;

    m_health--;

    if (m_health <= 0) {
        die();
    }
}

void Enemy::die()
{
    if (!m_isAlive) return;

    m_isAlive = false;

    // Stop all movement
    m_velocityX = 0;
    m_velocityY = 0;

    // Change sprite to death animation
    m_sprite->changeSprite(":/images/enemy_death.png");

    emit enemyDied();
}

bool Enemy::detectPlayer(QGraphicsItem* player) const
{
    if (!player) return false;
    qreal distance = QLineF(pos(), player->pos()).length();
    return distance < 200;
}

void Enemy::updateMovementPattern()
{
    // Basic back-and-forth movement pattern
    // This method can be overridden by derived classes
}

void Enemy::collideWithPlatform()
{
    // Handle platform collision
    // This method can be overridden by derived classes
}
