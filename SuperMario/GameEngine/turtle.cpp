#include "turtle.h"
#include "qgraphicsscene.h"
#include <QDebug>

Turtle::Turtle(QGraphicsItem *parent)
    : Enemy(parent),
    m_inShell(false),
    m_isSpinning(false),
    m_shellTimer(0)
{
    m_sprite->changeSprite(":/images/turtle.png");

    m_speed = 1.5;
    m_velocityX = -m_speed;
    m_health = 2;

    // Set data for collision detection
    setData(0, "enemy");
    setData(1, "turtle");
}

Turtle::~Turtle()
{
    // Destructor implementation needed for vtable
}

void Turtle::move()
{
    if (!m_isAlive) return;

    setPos(pos().x() + m_velocityX, pos().y() + m_velocityY);

    m_velocityY += m_gravity;

    if (m_inShell && m_shellTimer > 0) {
        m_shellTimer--;
        if (m_shellTimer <= 0 && !m_isSpinning) {
            m_inShell = false;
            m_sprite->changeSprite(":/images/turtle.png");
            m_velocityX = m_isMovingLeft ? -m_speed : m_speed;
        }
    }

    updateMovementPattern();

    m_sprite->animate();
}

void Turtle::enterShell()
{
    if (!m_isAlive || m_inShell) return;

    m_inShell = true;
    m_isSpinning = false;
    m_shellTimer = 180;

    m_sprite->changeSprite(":/images/redt.png");

    m_velocityX = 0;
}

void Turtle::spinShell()
{
    if (!m_isAlive || !m_inShell) return;

    m_isSpinning = true;

    if (m_velocityX == 0) {
        m_velocityX = 5.0;
    } else {
        m_velocityX = -m_velocityX;
    }
}

void Turtle::takeDamage()
{
    if (!m_isAlive) return;

    if (!m_inShell) {
        enterShell();
    } else if (!m_isSpinning) {
        spinShell();
    } else {
        Enemy::takeDamage();
    }
}

void Turtle::updateMovementPattern()
{
    if (m_inShell) {
        if (m_isSpinning) {
            QList<QGraphicsItem*> collidingItems = scene()->collidingItems(this);

            for (QGraphicsItem* item : collidingItems) {
                if (item->data(0).toString() == "platform") {
                    m_velocityX = -m_velocityX;
                    break;
                } else if (item->data(0).toString() == "enemy" && item != this) {
                    Enemy* otherEnemy = dynamic_cast<Enemy*>(item);
                    if (otherEnemy) {
                        otherEnemy->takeDamage();
                    }
                }
            }
        }
    } else {
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
    }

    if (!m_inShell) {
        m_sprite->flip(!m_isMovingLeft);
    }
}

void Turtle::withdrawIntoShell()
{
    // This is an empty method implementation
    // Can be used in the future to provide additional shell withdrawal behavior
    enterShell();
}
