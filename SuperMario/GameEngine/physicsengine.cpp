#include "physicsengine.h"
#include <QDebug>

PhysicsEngine::PhysicsEngine(QObject *parent)
    : QObject(parent),
      m_gravity(0.5),
      m_friction(0.1),
      m_bounceRestitution(0.5)
{
}

PhysicsEngine::~PhysicsEngine()
{
}

void PhysicsEngine::applyGravity(Player* player)
{
    if (!player || !player->isAlive()) return;
    
    QPointF force(0, m_gravity);
    applyForce(player, force);
}

void PhysicsEngine::applyForce(Player* player, QPointF force)
{
    if (!player || !player->isAlive()) return;
    
    player->setPos(player->pos() + force);
}

void PhysicsEngine::calculateTrajectory(Player* player)
{
}

void PhysicsEngine::updatePhysicsParameters()
{
}

void PhysicsEngine::resolveCollisions(QGraphicsItem* item1, QGraphicsItem* item2)
{
}