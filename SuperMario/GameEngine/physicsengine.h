#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <QObject>
#include <QPointF>
#include "player.h"

class PhysicsEngine : public QObject
{
    Q_OBJECT
    
public:
    PhysicsEngine(QObject *parent = nullptr);
    ~PhysicsEngine();
    
    void applyGravity(Player* player);
    void applyForce(Player* player, QPointF force);
    void calculateTrajectory(Player* player);
    
private:
    void updatePhysicsParameters();
    void resolveCollisions(QGraphicsItem* item1, QGraphicsItem* item2);
    
    qreal m_gravity;
    qreal m_friction;
    qreal m_bounceRestitution;
};

#endif