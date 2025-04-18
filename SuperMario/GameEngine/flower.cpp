#include "flower.h"
#include <QDebug>

Flower::Flower(QGraphicsItem *parent)
    : PowerUp(parent),
      m_isGrowing(true),
      m_growthStage(0)
{
    initializePowerUp();
}

Flower::~Flower()
{
}

void Flower::applyToPlayer(Player *player)
{
    if (player) {
        player->setState(PlayerState::FIRE);
    }
}

QString Flower::getType() const
{
    return "flower";
}

void Flower::ignite()
{
    startFireAnimation();
}

void Flower::move()
{
    if (m_isGrowing) {
        m_growthStage++;
        if (m_growthStage >= 10) {
            m_isGrowing = false;
        } else {
            setPos(pos().x(), pos().y() - 3);
        }
    }
}

void Flower::startFireAnimation()
{
}

void Flower::initializePowerUp()
{
    m_sprite = QPixmap(":/images/flower.png");
    
    m_velocityX = 0;
    m_velocityY = 0;
    
    setData(0, "flower");
}
