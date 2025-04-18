#include "mushroom.h"
#include <QDebug>

Mushroom::Mushroom(QGraphicsItem *parent)
    : PowerUp(parent),
      m_isGrowing(true),
      m_growthStage(0)
{
    initializePowerUp();
}

Mushroom::~Mushroom()
{
}

void Mushroom::applyToPlayer(Player *player)
{
    if (player) {
        player->setState(PlayerState::SUPER);
    }
}

QString Mushroom::getType() const
{
    return "mushroom";
}

void Mushroom::grow()
{
    if (m_isGrowing) {
        m_growthStage++;
        if (m_growthStage >= 10) {
            m_isGrowing = false;
            m_velocityX = 2;
        } else {
            setPos(pos().x(), pos().y() - 3);
        }
    }
}

void Mushroom::checkForGrowth()
{
}

void Mushroom::initializePowerUp()
{
    m_sprite = QPixmap(":/images/mushroom.png");
    
    m_velocityX = 0;
    m_velocityY = 0;
    
    setData(0, "mushroom");
}
