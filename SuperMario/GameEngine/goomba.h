#ifndef GOOMBA_H
#define GOOMBA_H

#include "enemy.h"
#include <QGraphicsScene>

class Goomba : public Enemy
{
public:
    Goomba(QGraphicsItem *parent = nullptr);
    ~Goomba() override;

    void move() override;
    void onStomped();

protected:
    void updateMovementPattern() override;

private:
    void patrol();

    int m_squashedTimer;
    bool m_isSquashed;
};
#endif
