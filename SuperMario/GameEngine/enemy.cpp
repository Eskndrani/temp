#include "enemy.h"
#include <QDebug>
#include <QPainter>

Enemy::Enemy(QGraphicsItem *parent)
    : QGraphicsItem(parent),  // QGraphicsItem must be first for multiple inheritance
      QObject(nullptr),       // Then QObject
      m_sprite(new Sprite()),  // Don't pass 'this' as parent to avoid double ownership
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
    
    // Initialize sprite (add default image)
    m_sprite->changeSprite(":/images/enemy_default.png");
}

Enemy::~Enemy()
{
    // Now safe to delete since we don't pass 'this' as parent
    delete m_sprite;
}

QRectF Enemy::boundingRect() const
{
    // Return bounding rect based on sprite size if available
    if (m_sprite && !m_sprite->pixmap().isNull()) {
        return QRectF(-m_sprite->pixmap().width()/2, 
                     -m_sprite->pixmap().height()/2,
                     m_sprite->pixmap().width(), 
                     m_sprite->pixmap().height());
    }
    // Fallback to default size
    return QRectF(-16, -16, 32, 32);
}

void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_isAlive) return;

    // Draw the sprite - THIS WAS MISSING
    if (m_sprite && !m_sprite->pixmap().isNull()) {
        painter->drawPixmap(-m_sprite->pixmap().width()/2, 
                          -m_sprite->pixmap().height()/2,
                          m_sprite->pixmap());
    }

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
    if (m_sprite) {
        m_sprite->animate();
    }
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
    if (m_sprite) {
        m_sprite->changeSprite(":/images/enemy_death.png");
    }

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
    // Here's a simple implementation rather than leaving it empty
    
    // Check if we need to change direction (e.g., at screen edges or obstacles)
    // This is a simple example - you'd likely have more complex logic in actual game
    if (pos().x() < -200) { // Left boundary
        m_isMovingLeft = false;
        m_velocityX = m_speed;
    } else if (pos().x() > 200) { // Right boundary
        m_isMovingLeft = true;
        m_velocityX = -m_speed;
    }
}

void Enemy::collideWithPlatform()
{
    // Basic implementation for platform collision
    m_velocityY = 0; // Stop falling
}

// Add a helper method to access the sprite
Sprite* Enemy::sprite() const
{
    return m_sprite;
}

// Override type() for better type checking
int Enemy::type() const
{
    // Register a custom type value
    return UserType + 1;
}
