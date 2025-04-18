#ifndef MUSHROOM_H
#define MUSHROOM_H

#include "powerup.h"

class Mushroom : public PowerUp
{
public:
    Mushroom(QGraphicsItem *parent = nullptr);
    ~Mushroom() override;
    
    void applyToPlayer(Player *player) override;
    QString getType() const override;
    void grow();
    
private:
    void checkForGrowth();
    void initializePowerUp() override;
    
    bool m_isGrowing;
    int m_growthStage;
};

#endif