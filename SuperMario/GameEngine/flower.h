#ifndef FLOWER_H
#define FLOWER_H

#include "powerup.h"

class Flower : public PowerUp
{
public:
    Flower(QGraphicsItem *parent = nullptr);
    ~Flower() override;
    
    void applyToPlayer(Player *player) override;
    QString getType() const override;
    void ignite();
    void move() override;
    
private:
    void startFireAnimation();
    void initializePowerUp() override;
    
    bool m_isGrowing;
    int m_growthStage;
};

#endif