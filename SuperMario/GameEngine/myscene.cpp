#include "myscene.h"
#include "goomba.h"
#include "turtle.h"
#include "mushroom.h"
#include "flower.h"
#include <QGraphicsRectItem>
#include <QDebug>

MyScene::MyScene(QScrollBar* scrollBar, QObject* parent)
    : QGraphicsScene(parent),
      gameRunning(false),
      isPaused(false),
      gameLevel(1),
      m_jumpFactor(1.0),
      m_scrollBar(scrollBar),
      m_timerId(0),
      sceneWidth(5000),
      sceneHeight(600)
{
    setSceneRect(0, 0, sceneWidth, sceneHeight);
    
    m_player = new Player();
    m_score = new Score();
    m_timer = new Timer();
    m_soundManager = new SoundManager();
    m_physicsEngine = new PhysicsEngine();
    m_inputManager = new InputManager();
    
    m_background = new QGraphicsPixmapItem(QPixmap(":/images/sky.png"));
    m_background->setZValue(-100);
    
    connect(this, &MyScene::playSound, m_soundManager, &SoundManager::playSound);
    
    // Connect player signals
    connect(m_player, &Player::projectileFired, this, &MyScene::handlePlayerFireball);
    
    initPlayField();
}

MyScene::~MyScene()
{
    // Clean up resources
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    
    // Remove items from scene before deleting to avoid double-delete
    removeItem(m_player);
    removeItem(m_score);
    removeItem(m_timer);
    removeItem(m_background);
    
    delete m_player;
    delete m_score;
    delete m_timer;
    delete m_soundManager;
    delete m_physicsEngine;
    delete m_inputManager;
    
    qDeleteAll(m_enemies);
    m_enemies.clear();
    
    qDeleteAll(m_powerups);
    m_powerups.clear();
    
    // Clear platforms list without deleting items (they're owned by the scene)
    m_platforms.clear();
}

void MyScene::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId && gameRunning && !isPaused) {
        updateGameLogic();
        updatePhysics();
        detectCollisions();
        updateCamera();
    }
}

qreal MyScene::jumpFactor() const
{
    return m_jumpFactor;
}

void MyScene::setJumpFactor(const qreal &jumpFactor)
{
    if (m_jumpFactor != jumpFactor) {
        m_jumpFactor = jumpFactor;
        emit jumpFactorChanged(jumpFactor);
    }
}

void MyScene::startGame()
{
    if (!gameRunning) {
        if (m_timerId == 0) {
            m_timerId = startTimer(16);
        }
        
        m_player->resetState();
        m_score->reset();
        m_timer->start();
        
        gameRunning = true;
        isPaused = false;
        
        emit gameStarted();
    }
}

void MyScene::pauseGame()
{
    if (gameRunning) {
        isPaused = !isPaused;
        
        if (isPaused) {
            m_timer->pause();
        } else {
            m_timer->start();
        }
        
        emit gamePaused();
    }
}

void MyScene::resetGame()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    
    clear();
    
    qDeleteAll(m_enemies);
    m_enemies.clear();
    
    qDeleteAll(m_powerups);
    m_powerups.clear();
    
    // Clear platforms list without deleting items (already handled by clear())
    m_platforms.clear();
    
    m_player->resetState();
    
    m_score->reset();
    m_timer->reset();
    
    gameRunning = false;
    isPaused = false;
    gameLevel = 1;
    
    initPlayField();
}

void MyScene::gameOver()
{
    if (gameRunning) {
        gameRunning = false;
        
        if (m_timerId != 0) {
            killTimer(m_timerId);
            m_timerId = 0;
        }
        
        QGraphicsPixmapItem* gameOverText = new QGraphicsPixmapItem(QPixmap(":/images/gameover.png"));
        gameOverText->setPos(400 - gameOverText->boundingRect().width()/2, 
                           300 - gameOverText->boundingRect().height()/2);
        gameOverText->setZValue(100);
        addItem(gameOverText);
        
        emit playSound("gameover");
        emit gameOverSignal();
    }
}

void MyScene::levelComplete()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    
    QGraphicsPixmapItem* levelCompleteText = new QGraphicsPixmapItem(QPixmap(":/images/flag.png"));
    levelCompleteText->setPos(m_player->pos().x() + 100, 300);
    levelCompleteText->setZValue(50);
    addItem(levelCompleteText);
    
    emit playSound("levelcomplete");
    emit levelCompleteSignal();
    
    gameLevel++;
}

void MyScene::handleInputEvent(QKeyEvent* event)
{
    if (gameRunning && !isPaused) {
        m_inputManager->processKeyPress(event);
        
        if (event->key() == Qt::Key_Left) {
            m_player->move("left");
        } else if (event->key() == Qt::Key_Right) {
            m_player->move("right");
        } else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Up) {
            m_player->jump();
            emit playSound("jump");
        } else if (event->key() == Qt::Key_F) {
            // Fixed the commented out code
            if (m_player->hasFirePower()) {
                m_player->fireProjectile(); // Changed to match the proper method name
                emit playSound("fireball");
            }
        }
    }
    
    if (event->key() == Qt::Key_P) {
        pauseGame();
    } else if (event->key() == Qt::Key_R) {
        resetGame();
        startGame();
    }
}

void MyScene::handleKeyReleaseEvent(QKeyEvent* event)
{
    if (gameRunning && !isPaused) {
        m_inputManager->processKeyRelease(event);
        
        if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
            m_player->move("stop");
        }
    }
}

void MyScene::updatePhysics()
{
    // Set the physics engine's scene reference if not already set
    if (m_physicsEngine->scene() != this) {
        m_physicsEngine->setScene(this);
    }
    
    // Let the physics engine handle all physics updates
    m_physicsEngine->update();
}

void MyScene::manageLevelTransition()
{
    resetGame();
    startGame();
}

void MyScene::initPlayField()
{
    addItem(m_background);
    
    addItem(m_player);
    m_player->setPos(100, sceneHeight - 200);
    
    for (int x = 0; x < sceneWidth; x += 32) {
        QGraphicsPixmapItem* ground = new QGraphicsPixmapItem(QPixmap(":/images/ground.png"));
        ground->setPos(x, sceneHeight - 32);
        addItem(ground);
        m_platforms.append(ground);
        ground->setData(0, "platform");
    }
    
    QVector<QPoint> brickPositions = {
        {200, 400}, {232, 400}, {264, 400},
        {400, 350}, {432, 350}, {464, 350},
        {600, 300}, {632, 300}, {664, 300},
        {800, 400}, {832, 400},
        {1000, 350}, {1032, 350}, {1064, 350},
        {1200, 300}, {1232, 300}
    };
    
    for (const QPoint& pos : brickPositions) {
        QGraphicsPixmapItem* brick = new QGraphicsPixmapItem(QPixmap(":/images/brick3.png"));
        brick->setPos(pos);
        addItem(brick);
        m_platforms.append(brick);
        brick->setData(0, "platform");
        brick->setData(1, false); // Not hit yet
    }
    
    QVector<QPoint> questionPositions = {
        {300, 350}, {500, 300}, {700, 250}, 
        {900, 350}, {1100, 300}
    };
    
    for (const QPoint& pos : questionPositions) {
        QGraphicsPixmapItem* question = new QGraphicsPixmapItem(QPixmap(":/images/questbox.png"));
        question->setPos(pos);
        addItem(question);
        m_platforms.append(question);
        question->setData(0, "questionBlock");
        question->setData(1, false); // Not hit yet
    }
    
    QVector<QPoint> pipePositions = {
        {400, sceneHeight - 64}, {800, sceneHeight - 64}, 
        {1200, sceneHeight - 64}, {1600, sceneHeight - 64}
    };
    
    for (const QPoint& pos : pipePositions) {
        QGraphicsPixmapItem* pipe = new QGraphicsPixmapItem(QPixmap(":/images/Warp.png"));
        pipe->setPos(pos);
        addItem(pipe);
        m_platforms.append(pipe);
        pipe->setData(0, "pipe");
    }
    
    for (int i = 0; i < 8; i++) {
        int x = 350 + i * 350;
        int y = sceneHeight - 64;
        
        Goomba* goomba = new Goomba();
        goomba->setPos(x, y);
        addItem(goomba);
        m_enemies.append(goomba);
    }
    
    for (int i = 0; i < 4; i++) {
        int x = 600 + i * 500;
        int y = sceneHeight - 64;
        
        Turtle* turtle = new Turtle();
        turtle->setPos(x, y);
        addItem(turtle);
        m_enemies.append(turtle);
    }
    
    for (int i = 0; i < 20; i++) {
        int x = 250 + i * 200;
        int y = sceneHeight - 100 - (rand() % 200);
        
        QGraphicsPixmapItem* coin = new QGraphicsPixmapItem(QPixmap(":/images/coin.png"));
        coin->setPos(x, y);
        addItem(coin);
        coin->setData(0, "coin");
    }
    
    QGraphicsPixmapItem* castle = new QGraphicsPixmapItem(QPixmap(":/images/castle.png"));
    castle->setPos(sceneWidth - 200, sceneHeight - 200);
    addItem(castle);
    castle->setData(0, "castle");
    
    addItem(m_score);
    m_score->setPos(10, 10);
    
    addItem(m_timer);
    m_timer->setPos(700, 10);
}

void MyScene::movePlayer()
{
    // This method is now handled by the physics engine's update method
    // The player's physics are processed in updatePhysics()
}

void MyScene::jumpPlayer()
{
    // Player jump is now triggered directly from handleInputEvent
    // and processed by physics engine
    if (m_player->isOnGround()) {
        m_player->jump();
        emit playSound("jump");
    }
}

void MyScene::fallPlayer()
{
    // Falling is now handled automatically by the physics engine
    // when the player is not supported by a platform
    m_player->setMovementState(MovementState::FALLING);
}

void MyScene::updateGameLogic()
{
    m_timer->tick();
    
    if (m_timer->isTimeUp()) {
        gameOver();
    }
    
    if (m_player->pos().y() > sceneHeight) {
        m_player->die();
        gameOver();
    }
    
    if (m_player->pos().x() > sceneWidth - 200) {
        levelComplete();
    }
    
    // Process any special input states (like holding down on pipes)
    if (m_inputManager->isKeyPressed(Qt::Key_Down)) {
        checkForWarpPipe();
    }
}

void MyScene::updateCamera()
{
    qreal playerX = m_player->pos().x();
    
    // Keep player in the visible portion of the scene
    qreal newX = qMax(0.0, playerX - 300);
    
    // Smooth camera movement
    qreal currentX = m_scrollBar ? m_scrollBar->value() : 0;
    qreal smoothFactor = 0.1; // Adjust for smoother or more responsive camera
    qreal targetX = currentX + (newX - currentX) * smoothFactor;
    
    if (m_scrollBar) {
        m_scrollBar->setValue(targetX);
    }
}

void MyScene::detectCollisions()
{
    // Using the physics engine for collision detection and response
    QList<QGraphicsItem*> allItems = items();
    
    // Let physics engine handle all collisions
    for (int i = 0; i < allItems.size(); ++i) {
        for (int j = i + 1; j < allItems.size(); ++j) {
            m_physicsEngine->resolveCollisions(allItems[i], allItems[j]);
        }
    }
    
    // Additional game-specific collision handling
    handleCollisionWithPlatform();
    checkItemCollisions();
    checkEnemyCollisions();
}

void MyScene::checkItemCollisions()
{
    QList<QGraphicsItem*> collidingItems = m_player->collidingItems();
    
    for (QGraphicsItem* item : collidingItems) {
        if (item->data(0).toString() == "coin") {
            emit playSound("coin");
            scoreIncrease(100);
            
            removeItem(item);
            delete item;
        } else if (item->data(0).toString() == "questionBlock") {
            QRectF playerRect = m_player->sceneBoundingRect();
            QRectF blockRect = item->sceneBoundingRect();
            
            // Only hit from below
            if (playerRect.top() > blockRect.bottom() - 10 &&
                playerRect.top() < blockRect.bottom() + 10) {
                
                if (item->data(1).toBool() == false) {
                    item->setData(1, true);
                    
                    QGraphicsPixmapItem* hitBlock = qgraphicsitem_cast<QGraphicsPixmapItem*>(item);
                    if (hitBlock) {
                        hitBlock->setPixmap(QPixmap(":/images/notebox.png"));
                    }
                    
                    // Create power-up
                    if (rand() % 2 == 0) {
                        Mushroom* mushroom = new Mushroom();
                        mushroom->setPos(item->pos().x(), item->pos().y() - 32);
                        addItem(mushroom);
                        m_powerups.append(mushroom);
                        emit playSound("sprout");
                    } else {
                        Flower* flower = new Flower();
                        flower->setPos(item->pos().x(), item->pos().y() - 32);
                        addItem(flower);
                        m_powerups.append(flower);
                        emit playSound("fsprout");
                    }
                }
            }
        } else if (item->data(0).toString() == "castle") {
            levelComplete();
        }
    }
    
    processPowerUpCollection();
}

void MyScene::checkEnemyCollisions()
{
    QList<Enemy*> defeatedEnemies;
    
    for (Enemy* enemy : m_enemies) {
        if (!enemy->isAlive()) continue;
        
        if (m_player->collidesWithItem(enemy)) {
            QRectF playerRect = m_player->sceneBoundingRect();
            QRectF enemyRect = enemy->sceneBoundingRect();
            
            bool playerOnTop = playerRect.bottom() < enemyRect.center().y() &&
                              m_player->movementState() == MovementState::FALLING;
            
            if (playerOnTop) {
                enemy->takeDamage();
                
                // Bounce the player up after jumping on enemy
                m_player->setPos(m_player->pos().x(), m_player->pos().y() - 10);
                QPointF jumpVelocity = m_player->velocity();
                jumpVelocity.setY(-10);
                m_player->setVelocity(jumpVelocity);
                
                emit playSound("kick");
                
                scoreIncrease(100);
                
                if (!enemy->isAlive()) {
                    defeatedEnemies.append(enemy);
                }
            } else if (!m_player->isInvincible()) {
                m_player->takeDamage();
                emit playSound("shrink");
                
                // Apply brief invincibility after taking damage
                m_player->setInvincible(true);
                QTimer::singleShot(1500, [this]() {
                    if (m_player) {
                        m_player->setInvincible(false);
                    }
                });
                
                if (!m_player->isAlive()) {
                    gameOver();
                }
            }
        }
        
        // Also check if fireballs hit enemies
        if (m_player->hasFirePower() && !m_projectiles.isEmpty()) {
            for (QGraphicsItem* fireball : m_projectiles) {
                if (fireball->collidesWithItem(enemy)) {
                    enemy->takeDamage();
                    emit playSound("kick");
                    
                    scoreIncrease(200);
                    
                    if (!enemy->isAlive()) {
                        defeatedEnemies.append(enemy);
                    }
                    
                    // Remove the fireball
                    m_projectiles.removeOne(fireball);
                    removeItem(fireball);
                    delete fireball;
                    break;
                }
            }
        }
    }
    
    for (Enemy* enemy : defeatedEnemies) {
        m_enemies.removeOne(enemy);
        removeItem(enemy);
        delete enemy;
    }
}

bool MyScene::handleCollisionWithPlatform()
{
    bool onPlatform = false;
    
    QList<QGraphicsItem*> collidingItems = m_player->collidingItems();
    
    for (QGraphicsItem* platform : collidingItems) {
        if (platform->data(0).toString() == "platform" || 
            platform->data(0).toString() == "questionBlock" ||
            platform->data(0).toString() == "pipe") {
            
            QRectF playerRect = m_player->sceneBoundingRect();
            QRectF platformRect = platform->sceneBoundingRect();
            
            qreal overlapTop = playerRect.bottom() - platformRect.top();
            qreal overlapBottom = platformRect.bottom() - playerRect.top();
            qreal overlapLeft = playerRect.right() - platformRect.left();
            qreal overlapRight = platformRect.right() - playerRect.left();
            
            qreal minOverlap = qMin(qMin(overlapTop, overlapBottom), qMin(overlapLeft, overlapRight));
            
            if (minOverlap == overlapTop && playerRect.center().y() < platformRect.center().y()) {
                // Landing on a platform
                m_player->setPos(m_player->pos().x(), m_player->pos().y() - overlapTop);
                
                // Only change to standing if the player was falling
                if (m_player->movementState() == MovementState::FALLING) {
                    m_player->setMovementState(MovementState::STANDING);
                    m_player->land();
                }
                
                // Update velocity
                QPointF velocity = m_player->velocity();
                velocity.setY(0);
                m_player->setVelocity(velocity);
                
                onPlatform = true;
            } else if (minOverlap == overlapBottom && playerRect.center().y() > platformRect.center().y()) {
                // Hitting a platform from below
                m_player->setPos(m_player->pos().x(), m_player->pos().y() + overlapBottom);
                
                QPointF velocity = m_player->velocity();
                if (velocity.y() < 0) {
                    velocity.setY(0);
                    m_player->setVelocity(velocity);
                }
                
                m_player->setMovementState(MovementState::FALLING);
                
                if (platform->data(0).toString() == "questionBlock") {
                    checkItemCollisions();
                }
            } else if (minOverlap == overlapLeft && playerRect.center().x() < platformRect.center().x()) {
                // Collision from the left
                m_player->setPos(m_player->pos().x() - overlapLeft, m_player->pos().y());
                
                QPointF velocity = m_player->velocity();
                if (velocity.x() > 0) {
                    velocity.setX(0);
                    m_player->setVelocity(velocity);
                }
            } else if (minOverlap == overlapRight && playerRect.center().x() > platformRect.center().x()) {
                // Collision from the right
                m_player->setPos(m_player->pos().x() + overlapRight, m_player->pos().y());
                
                QPointF velocity = m_player->velocity();
                if (velocity.x() < 0) {
                    velocity.setX(0);
                    m_player->setVelocity(velocity);
                }
            }
        }
    }
    
    // Update player's ground state
    m_player->setOnGround(onPlatform);
    return onPlatform;
}

void MyScene::processPowerUpCollection()
{
    QList<PowerUp*> collectedPowerups;
    
    for (PowerUp* powerup : m_powerups) {
        if (m_player->collidesWithItem(powerup)) {
            powerup->applyToPlayer(m_player);
            
            emit playSound("powerup");
            
            collectedPowerups.append(powerup);
            
            scoreIncrease(1000);
        }
    }
    
    for (PowerUp* powerup : collectedPowerups) {
        m_powerups.removeOne(powerup);
        removeItem(powerup);
        delete powerup;
    }
}

void MyScene::scoreIncrease(int value)
{
    m_score->addPoints(value);
}

void MyScene::playASound(const QString &soundName)
{
    emit playSound(soundName);
}

void MyScene::playHitWarp()
{
    emit playSound("warp");
}

void MyScene::checkForWarpPipe()
{
    QList<QGraphicsItem*> collidingItems = m_player->collidingItems();
    
    for (QGraphicsItem* item : collidingItems) {
        if (item->data(0).toString() == "pipe") {
            QRectF playerRect = m_player->sceneBoundingRect();
            QRectF pipeRect = item->sceneBoundingRect();
            
            // Check if player is on top of the pipe
            if (playerRect.bottom() > pipeRect.top() && 
                playerRect.bottom() < pipeRect.top() + 10 &&
                playerRect.center().x() > pipeRect.left() + 10 &&
                playerRect.center().x() < pipeRect.right() - 10) {
                    
                playHitWarp();
                
                // Animate player going down the pipe
                QPropertyAnimation* anim = new QPropertyAnimation(m_player, "y");
                anim->setDuration(1000);
                anim->setStartValue(m_player->pos().y());
                anim->setEndValue(m_player->pos().y() + 100);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
                
                // Change level or trigger secret area after animation
                QTimer::singleShot(1200, this, &MyScene::manageLevelTransition);
            }
        }
    }
}

void MyScene::handlePlayerFireball(QPointF position, bool facingRight)
{
    if (!m_player->hasFirePower()) return;
    
    // Create fireball object
    QGraphicsPixmapItem* fireball = new QGraphicsPixmapItem(QPixmap(":/images/fireball.png"));
    fireball->setPos(position);
    fireball->setData(0, "projectile"); // Set type for collision detection
    
    // Add to scene and to tracking list
    addItem(fireball);
    m_projectiles.append(fireball);
    
    // Apply velocity using QPropertyAnimation
    QPropertyAnimation* anim = new QPropertyAnimation(fireball, "x");
    anim->setDuration(2000); // 2 seconds flight time
    anim->setStartValue(position.x());
    anim->setEndValue(position.x() + (facingRight ? 500 : -500)); // Travel distance
    
    // Gravity effect
    QPropertyAnimation* yAnim = new QPropertyAnimation(fireball, "y");
    yAnim->setDuration(2000);
    yAnim->setStartValue(position.y());
    yAnim->setKeyValueAt(0.5, position.y() - 50); // Arc upward
    yAnim->setEndValue(position.y() + 100); // End lower
    
    // Group animations
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    group->addAnimation(anim);
    group->addAnimation(yAnim);
    
    // Clean up when done
    connect(group, &QParallelAnimationGroup::finished, [this, fireball]() {
        if (m_projectiles.contains(fireball)) {
            m_projectiles.removeOne(fireball);
            removeItem(fireball);
            delete fireball;
        }
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
