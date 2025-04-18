#include "goomba.h"
#include <QDebug>

Goomba::Goomba(QGraphicsItem *parent)
    : Enemy(parent),
      m_squashedTimer(0),
      m_isSquashed(false)
{
    m_sprite->changeSprite(":/images/goomba.png");
    
    m_speed = 1.0;
    m_velocityX = -m_speed;
    m_health = 1;
}

Goomba::~Goomba()
{
}

void Goomba::move()
{
    if (!m_isAlive) return;
    
    if (m_isSquashed) {
        m_squashedTimer--;
        if (m_squashedTimer <= 0) {
            m_isAlive = false;
        }
        return;
    }
    
    setPos(pos().x() + m_velocityX, pos().y() + m_velocityY);
    
    m_velocityY += m_gravity;
    
    updateMovementPattern();
    
    m_sprite->animate();
}

void Goomba::onStomped()
{
    if (!m_isAlive || m_isSquashed) return;
    
    m_isSquashed = true;
    m_squashedTimer = 30;
    
    m_sprite->changeSprite(":/images/goombas.png");
    
    m_velocityX = 0;
    m_velocityY = 0;
}

void Goomba::updateMovementPattern()
{
    patrol();
}

void Goomba::patrol()
{
    QList<QGraphicsItem*> collidingItems = scene()->collidingItems(this);
    
    for (QGraphicsItem* item : collidingItems) {
        if (item->data(0).toString() == "platform") {
            QRectF enemyRect = mapRectToScene(boundingRect());
            QRectF itemRect = item->mapRectToScene(item->boundingRect());
            
            if (enemyRect.right() >= itemRect.left() && enemyRect.right() <= itemRect.left() + 5) {
                m_velocityX = -m_speed;
                m_isMovingLeft = true;
            } else if (enemyRect.left() <= itemRect.right() && enemyRect.left() >= itemRect.right() - 5) {
                m_velocityX = m_speed;
                m_isMovingLeft = false;
            }
        }
    }
    
    m_sprite->flip(!m_isMovingLeft);
}