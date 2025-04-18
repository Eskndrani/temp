#include "powerup.h"
#include <QDebug>

PowerUp::PowerUp(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_velocityX(2),
      m_velocityY(0),
      m_gravity(0.5),
      m_isMovingLeft(false)
{
    initializePowerUp();
    
    setData(0, "powerup");
}

PowerUp::~PowerUp()
{
}

void PowerUp::move()
{
    setPos(pos().x() + m_velocityX, pos().y() + m_velocityY);
    
    m_velocityY += m_gravity;
    
    QList<QGraphicsItem*> collidingItems = scene()->collidingItems(this);
    
    for (QGraphicsItem* item : collidingItems) {
        if (item->data(0).toString().contains("platform") || 
            item->data(0).toString() == "questionBlock" ||
            item->data(0).toString() == "pipe") {
            
            QRectF powerupRect = mapRectToScene(boundingRect());
            QRectF itemRect = item->mapRectToScene(item->boundingRect());
            
            if (powerupRect.bottom() >= itemRect.top() && 
                powerupRect.bottom() - m_velocityY <= itemRect.top()) {
                setPos(pos().x(), itemRect.top() - boundingRect().height() / 2);
                m_velocityY = 0;
            } else if (powerupRect.right() >= itemRect.left() && 
                     powerupRect.right() - m_velocityX <= itemRect.left()) {
                m_velocityX = -qAbs(m_velocityX);
                m_isMovingLeft = true;
            } else if (powerupRect.left() <= itemRect.right() && 
                     powerupRect.left() - m_velocityX >= itemRect.right()) {
                m_velocityX = qAbs(m_velocityX);
                m_isMovingLeft = false;
            }
        }
    }
    
    if (pos().y() > scene()->sceneRect().height()) {
        scene()->removeItem(this);
    }
}

QRectF PowerUp::boundingRect() const
{
    return QRectF(-16, -16, 32, 32);
}

void PowerUp::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->drawPixmap(-16, -16, 32, 32, m_sprite);
    
    #ifdef QT_DEBUG
    painter->setPen(Qt::blue);
    painter->drawRect(boundingRect());
    #endif
}

void PowerUp::initializePowerUp()
{
}