#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <QObject>
#include <QPointF>
#include <QVector>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include "player.h"
#include "enemy.h"
#include "projectile.h"

// Game state enum to configure physics for different environments
enum class GameState {
    NORMAL,
    UNDERWATER,
    ICE_LEVEL,
    SPACE
};

class PhysicsEngine : public QObject
{
    Q_OBJECT
public:
    explicit PhysicsEngine(QObject *parent = nullptr);
    ~PhysicsEngine();
    
    // Core physics methods
    void applyGravity(Player* player);
    void applyForce(Player* player, QPointF force);
    void calculateTrajectory(Player* player);
    void updatePhysicsParameters();
    void update();
    
    // Collision resolution methods
    void resolveCollisions(QGraphicsItem* item1, QGraphicsItem* item2);
    void resolvePlayerPlatformCollision(Player* player, QGraphicsItem* platform);
    void resolvePlayerEnemyCollision(Player* player, QGraphicsItem* enemy);
    void resolveEnemyPlatformCollision(QGraphicsItem* enemy, QGraphicsItem* platform);
    void resolveProjectileCollision(QGraphicsItem* projectile, QGraphicsItem* target);
    
    // Scene management
    void setScene(QGraphicsScene* scene);
    QGraphicsScene* scene() const;
    void processDestructionQueue();
    void queueForDestruction(QGraphicsItem* item);
    
    // State management
    void setGameState(GameState state);
    
    // Getters and setters for physics parameters
    void setGravity(float gravity);
    float gravity() const;
    
    void setFriction(float friction);
    float friction() const;
    
    void setBounceRestitution(float restitution);
    float bounceRestitution() const;
    
    void setTimeStep(float timeStep);
    float timeStep() const;
    
    void setMaxFallSpeed(float maxFallSpeed);
    float maxFallSpeed() const;
    
    void setFriendlyFire(bool enabled);
    bool friendlyFire() const;
    
    QVector<QPointF> lastCalculatedTrajectory() const;

signals:
    void trajectoryCalculated(const QVector<QPointF>& trajectory);
    void enemyDefeated(QGraphicsItem* enemy);
    void enemyHit(QGraphicsItem* enemy, QGraphicsItem* projectile);
    void playerHit(Player* player);
    void playerCollectedItem(Player* player, QGraphicsItem* item);
    void projectileDestroyed(QGraphicsItem* projectile);
    void platformBroken(QGraphicsItem* platform);
    void platformDamaged(QGraphicsItem* platform);
    void pointsAwarded(int points);
    
private:
    // Physics parameters
    float m_gravity;
    float m_friction;
    float m_bounceRestitution;
    float m_timeStep;
    float m_maxFallSpeed;
    float m_airResistance;
    float m_groundResistance;
    
    // Game state
    GameState m_gameState = GameState::NORMAL;
    bool m_friendlyFire = false;
    
    // Scene reference
    QGraphicsScene* m_scene;
    
    // Destruction queue for safe item removal
    QList<QGraphicsItem*> m_destructionQueue;
    
    // Trajectory data
    QVector<QPointF> m_lastCalculatedTrajectory;
};

#endif // PHYSICSENGINE_H
