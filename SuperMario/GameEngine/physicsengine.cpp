#include "physicsengine.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QtMath>
#include <QList>

PhysicsEngine::PhysicsEngine(QObject *parent)
    : QObject(parent),
      m_gravity(0.5),
      m_friction(0.1),
      m_bounceRestitution(0.5),
      m_timeStep(1.0/60.0), // 60 FPS default timestep
      m_maxFallSpeed(15.0f),
      m_airResistance(0.02f),
      m_groundResistance(0.2f),
      m_scene(nullptr)
{
}

PhysicsEngine::~PhysicsEngine()
{
    // Clean up any resources if needed
}

void PhysicsEngine::applyGravity(Player* player)
{
    if (!player || !player->isAlive()) return;
    
    // Apply gravity as acceleration rather than direct position change
    QPointF velocity = player->velocity();
    velocity.setY(velocity.y() + m_gravity);
    
    // Apply terminal velocity (max fall speed)
    if (velocity.y() > m_maxFallSpeed) {
        velocity.setY(m_maxFallSpeed);
    }
    
    player->setVelocity(velocity);
}

void PhysicsEngine::applyForce(Player* player, QPointF force)
{
    if (!player || !player->isAlive()) return;
    
    // F = ma, so a = F/m (assuming mass = 1 for simplicity)
    QPointF acceleration = force;
    
    // Update velocity based on acceleration
    QPointF velocity = player->velocity();
    velocity += acceleration * m_timeStep;
    player->setVelocity(velocity);
    
    // Apply the velocity to the position
    player->setPos(player->pos() + velocity * m_timeStep);
}

void PhysicsEngine::calculateTrajectory(Player* player)
{
    if (!player || !player->isAlive()) return;
    
    // Calculate the predicted trajectory for jumping or projectile motion
    QPointF currentVelocity = player->velocity();
    QPointF currentPosition = player->pos();
    m_lastCalculatedTrajectory.clear();
    
    // Simulate future positions (useful for AI or showing jump arc)
    const int TRAJECTORY_STEPS = 60; // Show 1 second of trajectory at 60fps
    for (int step = 1; step <= TRAJECTORY_STEPS; step++) {
        float t = step * m_timeStep;
        QPointF simulatedVelocity = currentVelocity;
        QPointF simulatedPosition = currentPosition;
        
        // Simple physics simulation
        for (int i = 0; i < step; i++) {
            // Apply gravity
            simulatedVelocity.setY(qMin(simulatedVelocity.y() + m_gravity, m_maxFallSpeed));
            
            // Apply air resistance
            simulatedVelocity.setX(simulatedVelocity.x() * (1.0f - m_airResistance));
            
            // Update position
            simulatedPosition += simulatedVelocity * m_timeStep;
            
            // Very basic collision check with ground (y=0 plane)
            if (simulatedPosition.y() >= 0) {
                simulatedPosition.setY(0);
                simulatedVelocity.setY(0);
                break;
            }
        }
        
        m_lastCalculatedTrajectory.append(simulatedPosition);
    }
    
    // Emit signal with trajectory data
    emit trajectoryCalculated(m_lastCalculatedTrajectory);
}

void PhysicsEngine::updatePhysicsParameters()
{
    // Adjust physics parameters based on game state
    if (m_gameState == GameState::UNDERWATER) {
        // Underwater physics: less gravity, more resistance
        m_gravity = 0.2f;
        m_friction = 0.15f;
        m_airResistance = 0.1f;
        m_maxFallSpeed = 5.0f;
        m_bounceRestitution = 0.3f;
    }
    else if (m_gameState == GameState::ICE_LEVEL) {
        // Ice level: normal gravity, very low friction
        m_gravity = 0.5f;
        m_friction = 0.01f;
        m_airResistance = 0.01f;
        m_maxFallSpeed = 15.0f;
        m_bounceRestitution = 0.8f;
    }
    else if (m_gameState == GameState::SPACE) {
        // Space level: low gravity, no resistance
        m_gravity = 0.1f;
        m_friction = 0.0f;
        m_airResistance = 0.0f;
        m_maxFallSpeed = 5.0f;
        m_bounceRestitution = 1.0f;
    }
    else {
        // Normal level (default parameters)
        m_gravity = 0.5f;
        m_friction = 0.1f;
        m_airResistance = 0.02f;
        m_maxFallSpeed = 15.0f;
        m_bounceRestitution = 0.5f;
    }
    
    // Log the current physics parameters
    qDebug() << "Physics Parameters Updated:";
    qDebug() << "  Gravity:" << m_gravity;
    qDebug() << "  Friction:" << m_friction;
    qDebug() << "  Air Resistance:" << m_airResistance;
    qDebug() << "  Max Fall Speed:" << m_maxFallSpeed;
    qDebug() << "  Bounce Restitution:" << m_bounceRestitution;
}

void PhysicsEngine::resolveCollisions(QGraphicsItem* item1, QGraphicsItem* item2)
{
    if (!item1 || !item2) return;
    
    // Get collision data
    QRectF rect1 = item1->sceneBoundingRect();
    QRectF rect2 = item2->sceneBoundingRect();
    
    // Check for collision
    if (!rect1.intersects(rect2)) return;
    
    // Calculate intersection depth
    QRectF intersection = rect1.intersected(rect2);
    
    // Identify item types for specialized handling
    QString type1 = item1->data(0).toString();
    QString type2 = item2->data(0).toString();
    
    // Player-platform collision
    if (type1 == "player" && type2 == "platform") {
        resolvePlayerPlatformCollision(dynamic_cast<Player*>(item1), item2);
    } 
    else if (type2 == "player" && type1 == "platform") {
        resolvePlayerPlatformCollision(dynamic_cast<Player*>(item2), item1);
    }
    // Player-enemy collision
    else if (type1 == "player" && type2 == "enemy") {
        resolvePlayerEnemyCollision(dynamic_cast<Player*>(item1), item2);
    }
    else if (type2 == "player" && type1 == "enemy") {
        resolvePlayerEnemyCollision(dynamic_cast<Player*>(item2), item1);
    }
    // Enemy-platform collision
    else if (type1 == "enemy" && type2 == "platform") {
        resolveEnemyPlatformCollision(item1, item2);
    }
    else if (type2 == "enemy" && type1 == "platform") {
        resolveEnemyPlatformCollision(item2, item1);
    }
    // Player-item collision (coins, power-ups, etc.)
    else if (type1 == "player" && type2 == "item") {
        emit playerCollectedItem(dynamic_cast<Player*>(item1), item2);
    }
    else if (type2 == "player" && type1 == "item") {
        emit playerCollectedItem(dynamic_cast<Player*>(item2), item1);
    }
    // Projectile collisions
    else if (type1 == "projectile" || type2 == "projectile") {
        resolveProjectileCollision(
            type1 == "projectile" ? item1 : item2,
            type1 == "projectile" ? item2 : item1);
    }
}

void PhysicsEngine::resolvePlayerPlatformCollision(Player* player, QGraphicsItem* platform)
{
    if (!player || !platform) return;
    
    QRectF playerRect = player->sceneBoundingRect();
    QRectF platformRect = platform->sceneBoundingRect();
    QRectF intersection = playerRect.intersected(platformRect);
    
    // Get player's current velocity
    QPointF velocity = player->velocity();
    
    // Calculate collision direction (where is the player relative to the platform?)
    bool fromTop = intersection.height() < intersection.width() && 
                   playerRect.bottom() - platformRect.top() <= intersection.height();
    bool fromBottom = intersection.height() < intersection.width() && 
                     platformRect.bottom() - playerRect.top() <= intersection.height();
    bool fromLeft = intersection.width() < intersection.height() && 
                   playerRect.right() - platformRect.left() <= intersection.width();
    bool fromRight = intersection.width() < intersection.height() && 
                    platformRect.right() - playerRect.left() <= intersection.width();
    
    // Resolve based on collision direction
    if (fromTop && velocity.y() >= 0) {
        // Player landed on platform
        player->setPos(player->pos().x(), platformRect.top() - playerRect.height() + 1);
        velocity.setY(0);
        player->land(); // Signal player landed
        
        // Apply ground resistance to horizontal movement
        velocity.setX(velocity.x() * (1.0f - m_groundResistance));
    }
    else if (fromBottom && velocity.y() < 0) {
        // Player hit platform from below
        player->setPos(player->pos().x(), platformRect.bottom());
        velocity.setY(0);
        
        // Check if platform is breakable and break it if needed
        if (platform->data(1).toString() == "breakable") {
            emit platformBroken(platform);
        }
    }
    else if (fromLeft && velocity.x() > 0) {
        // Player hit platform from left
        player->setPos(platformRect.left() - playerRect.width(), player->pos().y());
        velocity.setX(0);
    }
    else if (fromRight && velocity.x() < 0) {
        // Player hit platform from right
        player->setPos(platformRect.right(), player->pos().y());
        velocity.setX(0);
    }
    
    // Update player velocity
    player->setVelocity(velocity);
}

void PhysicsEngine::resolvePlayerEnemyCollision(Player* player, QGraphicsItem* enemy)
{
    if (!player || !enemy) return;
    
    QRectF playerRect = player->sceneBoundingRect();
    QRectF enemyRect = enemy->sceneBoundingRect();
    QRectF intersection = playerRect.intersected(enemyRect);
    
    // Check if player is jumping on enemy from above
    bool playerJumpingOnEnemy = 
        player->velocity().y() > 0 && 
        playerRect.bottom() - enemyRect.top() < intersection.height() * 0.5;
    
    if (playerJumpingOnEnemy) {
        // Player defeats enemy by jumping on it
        emit enemyDefeated(enemy);
        
        // Bounce player up slightly
        QPointF velocity = player->velocity();
        velocity.setY(-8.0); // Set upward velocity for bounce
        player->setVelocity(velocity);
        
        // Award points if applicable
        emit pointsAwarded(100);
    } else {
        // Player collided with enemy from other direction
        
        // Check if player has star power or other invincibility
        if (player->isInvincible()) {
            // Defeat enemy with star power
            emit enemyDefeated(enemy);
            emit pointsAwarded(200);
        } else {
            // Player takes damage
            emit playerHit(player);
        }
    }
}

void PhysicsEngine::resolveEnemyPlatformCollision(QGraphicsItem* enemy, QGraphicsItem* platform)
{
    if (!enemy || !platform) return;
    
    // Assuming Enemy class has these methods (you'll need to add them)
    Enemy* enemyObject = dynamic_cast<Enemy*>(enemy);
    if (!enemyObject) return;
    
    QRectF enemyRect = enemy->sceneBoundingRect();
    QRectF platformRect = platform->sceneBoundingRect();
    QRectF intersection = enemyRect.intersected(platformRect);
    
    // Get current velocity
    QPointF velocity = enemyObject->velocity();
    
    // Calculate collision direction
    bool fromTop = intersection.height() < intersection.width() && 
                   enemyRect.bottom() - platformRect.top() <= intersection.height();
    bool fromBottom = intersection.height() < intersection.width() && 
                     platformRect.bottom() - enemyRect.top() <= intersection.height();
    bool fromLeft = intersection.width() < intersection.height() && 
                   enemyRect.right() - platformRect.left() <= intersection.width();
    bool fromRight = intersection.width() < intersection.height() && 
                    platformRect.right() - enemyRect.left() <= intersection.width();
    
    // Resolve based on collision direction
    if (fromTop && velocity.y() >= 0) {
        // Enemy landed on platform
        enemy->setPos(enemy->pos().x(), platformRect.top() - enemyRect.height() + 1);
        velocity.setY(0);
        enemyObject->setVelocity(velocity);
        
        // Check if enemy has reached platform edge
        float enemyLeftEdge = enemy->pos().x();
        float enemyRightEdge = enemy->pos().x() + enemyRect.width();
        float platformLeftEdge = platformRect.left();
        float platformRightEdge = platformRect.right();
        
        // If at edge, change direction
        if (enemyLeftEdge <= platformLeftEdge + 5 || 
            enemyRightEdge >= platformRightEdge - 5) {
            
            // Turn around
            velocity.setX(-velocity.x());
            enemyObject->setVelocity(velocity);
            
            // Flip the enemy sprite if applicable
            enemyObject->flip(!enemyObject->isFlippedHorizontal());
        }
    }
    else if (fromLeft && velocity.x() > 0) {
        // Enemy hit platform from left
        enemy->setPos(platformRect.left() - enemyRect.width(), enemy->pos().y());
        velocity.setX(-velocity.x()); // Change direction
        enemyObject->setVelocity(velocity);
        enemyObject->flip(true);
    }
    else if (fromRight && velocity.x() < 0) {
        // Enemy hit platform from right
        enemy->setPos(platformRect.right(), enemy->pos().y());
        velocity.setX(-velocity.x()); // Change direction
        enemyObject->setVelocity(velocity);
        enemyObject->flip(false);
    }
}

void PhysicsEngine::resolveProjectileCollision(QGraphicsItem* projectile, QGraphicsItem* target)
{
    if (!projectile || !target) return;
    
    QString targetType = target->data(0).toString();
    
    // Handle different collision types
    if (targetType == "enemy") {
        // Projectile hit enemy
        emit enemyHit(target, projectile);
        emit projectileDestroyed(projectile);
    }
    else if (targetType == "platform") {
        // Check if platform is breakable
        if (target->data(1).toString() == "breakable") {
            emit platformDamaged(target);
        }
        
        // Destroy the projectile
        emit projectileDestroyed(projectile);
    }
    else if (targetType == "player") {
        // Check if friendly fire is enabled
        if (m_friendlyFire) {
            Player* player = dynamic_cast<Player*>(target);
            if (player) {
                emit playerHit(player);
                emit projectileDestroyed(projectile);
            }
        }
    }
}

void PhysicsEngine::update()
{
    if (!m_scene) return;
    
    // Get all items in the scene
    QList<QGraphicsItem*> allItems = m_scene->items();
    
    // Process players
    QList<Player*> players;
    for (QGraphicsItem* item : allItems) {
        if (item->data(0).toString() == "player") {
            Player* player = dynamic_cast<Player*>(item);
            if (player && player->isAlive()) {
                players.append(player);
                
                // Apply gravity
                applyGravity(player);
                
                // Apply air/ground resistance to horizontal movement
                QPointF velocity = player->velocity();
                bool onGround = player->isOnGround();
                
                if (qAbs(velocity.x()) > 0.1f) {
                    float resistance = onGround ? m_groundResistance : m_airResistance;
                    velocity.setX(velocity.x() * (1.0f - resistance));
                    player->setVelocity(velocity);
                } else if (qAbs(velocity.x()) <= 0.1f) {
                    velocity.setX(0);
                    player->setVelocity(velocity);
                }
                
                // Update position
                player->setPos(player->pos() + velocity * m_timeStep);
            }
        }
    }
    
    // Process enemies
    for (QGraphicsItem* item : allItems) {
        if (item->data(0).toString() == "enemy") {
            Enemy* enemy = dynamic_cast<Enemy*>(item);
            if (enemy) {
                // Apply gravity to enemies
                QPointF velocity = enemy->velocity();
                velocity.setY(qMin(velocity.y() + m_gravity, m_maxFallSpeed));
                enemy->setVelocity(velocity);
                
                // Update position
                enemy->setPos(enemy->pos() + velocity * m_timeStep);
                
                // AI movement (can be customized based on enemy type)
                enemy->move();
            }
        }
    }
    
    // Process projectiles
    for (QGraphicsItem* item : allItems) {
        if (item->data(0).toString() == "projectile") {
            // Update projectile position
            Projectile* projectile = dynamic_cast<Projectile*>(item);
            if (projectile) {
                QPointF velocity = projectile->velocity();
                
                // Apply gravity if affected by gravity
                if (projectile->isAffectedByGravity()) {
                    velocity.setY(qMin(velocity.y() + m_gravity * 0.5f, m_maxFallSpeed));
                    projectile->setVelocity(velocity);
                }
                
                // Update position
                projectile->setPos(projectile->pos() + velocity * m_timeStep);
                
                // Check lifetime - remove if expired
                if (projectile->isExpired()) {
                    emit projectileDestroyed(item);
                }
            }
        }
    }
    
    // Handle collisions
    for (int i = 0; i < allItems.size(); ++i) {
        for (int j = i + 1; j < allItems.size(); ++j) {
            resolveCollisions(allItems[i], allItems[j]);
        }
    }
    
    // Clean up any flagged items
    processDestructionQueue();
}

void PhysicsEngine::processDestructionQueue()
{
    // Remove any items that were flagged for destruction
    for (QGraphicsItem* item : m_destructionQueue) {
        if (m_scene) {
            m_scene->removeItem(item);
        }
        delete item;
    }
    m_destructionQueue.clear();
}

void PhysicsEngine::queueForDestruction(QGraphicsItem* item)
{
    if (item && !m_destructionQueue.contains(item)) {
        m_destructionQueue.append(item);
    }
}

void PhysicsEngine::setScene(QGraphicsScene* scene)
{
    m_scene = scene;
}

QGraphicsScene* PhysicsEngine::scene() const
{
    return m_scene;
}

void PhysicsEngine::setGameState(GameState state)
{
    m_gameState = state;
    updatePhysicsParameters();
}

void PhysicsEngine::setGravity(float gravity)
{
    m_gravity = gravity;
}

float PhysicsEngine::gravity() const
{
    return m_gravity;
}

void PhysicsEngine::setFriction(float friction)
{
    m_friction = friction;
}

float PhysicsEngine::friction() const
{
    return m_friction;
}

void PhysicsEngine::setBounceRestitution(float restitution)
{
    m_bounceRestitution = restitution;
}

float PhysicsEngine::bounceRestitution() const
{
    return m_bounceRestitution;
}

void PhysicsEngine::setTimeStep(float timeStep)
{
    m_timeStep = timeStep;
}

float PhysicsEngine::timeStep() const
{
    return m_timeStep;
}

void PhysicsEngine::setMaxFallSpeed(float maxFallSpeed)
{
    m_maxFallSpeed = maxFallSpeed;
}

float PhysicsEngine::maxFallSpeed() const
{
    return m_maxFallSpeed;
}

void PhysicsEngine::setFriendlyFire(bool enabled)
{
    m_friendlyFire = enabled;
}

bool PhysicsEngine::friendlyFire() const
{
    return m_friendlyFire;
}

QVector<QPointF> PhysicsEngine::lastCalculatedTrajectory() const
{
    return m_lastCalculatedTrajectory;
}
