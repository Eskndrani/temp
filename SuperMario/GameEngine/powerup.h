#ifndef POWERUP_H
#define POWERUP_H

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsScene>
#include "player.h"

class PowerUp : public QGraphicsItem
{
public:
    PowerUp(QGraphicsItem *parent = nullptr);
    virtual ~PowerUp();

    virtual void applyToPlayer(Player *player) = 0;
    virtual QString getType() const = 0;
    virtual void move();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    virtual void initializePowerUp();

    QPixmap m_sprite;
    qreal m_velocityX;
    qreal m_velocityY;
    qreal m_gravity;
    bool m_isMovingLeft;
};
#endif
