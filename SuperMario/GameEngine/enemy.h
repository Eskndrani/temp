#ifndef ENEMY_H
#define ENEMY_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include "sprite.h"

class Enemy : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Enemy(QGraphicsItem *parent = nullptr);
    ~Enemy() override;

    // QGraphicsItem methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    // Enemy behaviors
    virtual void move();
    virtual void takeDamage();
    virtual void die();
    bool detectPlayer(QGraphicsItem* player) const;

    // State queries
    bool isAlive() const { return m_isAlive; }
    bool isMovingLeft() const { return m_isMovingLeft; }

protected:
    virtual void updateMovementPattern();
    virtual void collideWithPlatform();

    Sprite* m_sprite;
    bool m_isAlive;
    bool m_isMovingLeft;
    qreal m_velocityX;
    qreal m_velocityY;
    qreal m_speed;
    qreal m_gravity;
    int m_health;

signals:
    void enemyDied();
};

#endif // ENEMY_H
