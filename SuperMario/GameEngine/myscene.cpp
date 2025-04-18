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
    
    initPlayField();
}

MyScene::~MyScene()
{
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
    
    emit levelCompleteSignal();
    
    gameLevel++;
}

void MyScene::handleInputEvent(QKeyEvent* event)
{
    if (gameRunning && !isPaused) {
        m_inputManager->processKeyPress(event);
        
        if (event->key() == Qt::Key_Left) {
            m_player->move("left");
            emit playSound("jump");
        } else if (event->key() == Qt::Key_Right) {
            m_player->move("right");
        } else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Up) {
            m_player->jump();
            emit playSound("jump");
       // } else if (event->key() == Qt::Key_F && m_player->hasFirePower()) {
         //   m_player->shootFire();
         //   emit playSound("fireball");
       // }
        }
    }
    
    if (event->key() == Qt::Key_P) {
        pauseGame();
    } else if (event->key() == Qt::Key_R) {
        resetGame();
        startGame();
    }
}

void MyScene::updatePhysics()
{
    m_physicsEngine->applyGravity(m_player);
    m_physicsEngine->calculateTrajectory(m_player);
    
    for (Enemy* enemy : m_enemies) {
        if (enemy->isAlive()) {
            enemy->move();
        }
    }
    
    for (PowerUp* powerup : m_powerups) {
        powerup->move();
    }
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
}

void MyScene::jumpPlayer()
{
}

void MyScene::fallPlayer()
{
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
}

void MyScene::updateCamera()
{
    qreal playerX = m_player->pos().x();
    
    qreal newX = qMax(0.0, playerX - 300);
    
    if (m_scrollBar) {
        m_scrollBar->setValue(newX);
    }
}

void MyScene::detectCollisions()
{
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
            
            if (playerRect.top() > blockRect.bottom() - 10 &&
                playerRect.top() < blockRect.bottom() + 10) {
                
                if (item->data(1).toBool() == false) {
                    item->setData(1, true);
                    
                    QGraphicsPixmapItem* hitBlock = qgraphicsitem_cast<QGraphicsPixmapItem*>(item);
                    if (hitBlock) {
                        hitBlock->setPixmap(QPixmap(":/images/notebox.png"));
                    }
                    
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
        } else if (item->data(0).toString() == "pipe") {
            QRectF playerRect = m_player->sceneBoundingRect();
            QRectF pipeRect = item->sceneBoundingRect();
            
            if (playerRect.bottom() < pipeRect.top() + 10 &&
                playerRect.bottom() > pipeRect.top() - 10) {
                if (m_inputManager->isKeyPressed(Qt::Key_Down)) {
                    playHitWarp();
                }
            }
        } else if (item->data(0).toString() == "castle") {
            levelComplete();
        }
    }
    
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
                
                m_player->setPos(m_player->pos().x(), m_player->pos().y() - 10);
                m_physicsEngine->applyForce(m_player, QPointF(0, -10));
                
                emit playSound("kick");
                
                scoreIncrease(100);
                
                if (!enemy->isAlive()) {
                    defeatedEnemies.append(enemy);
                }
            } else {
                m_player->takeDamage();
                emit playSound("shrink");
                
                if (!m_player->isAlive()) {
                    gameOver();
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
                m_player->setPos(m_player->pos().x(), m_player->pos().y() - overlapTop);
                m_player->setMovementState(MovementState::STANDING);
                onPlatform = true;
            } else if (minOverlap == overlapBottom && playerRect.center().y() > platformRect.center().y()) {
                m_player->setPos(m_player->pos().x(), m_player->pos().y() + overlapBottom);
                m_player->setMovementState(MovementState::FALLING);
                
                if (platform->data(0).toString() == "questionBlock") {
                    checkItemCollisions();
                }
            } else if (minOverlap == overlapLeft && playerRect.center().x() < platformRect.center().x()) {
                m_player->setPos(m_player->pos().x() - overlapLeft, m_player->pos().y());
            } else if (minOverlap == overlapRight && playerRect.center().x() > platformRect.center().x()) {
                m_player->setPos(m_player->pos().x() + overlapRight, m_player->pos().y());
            }
        }
    }
    
    return onPlatform;
}

void MyScene::processPowerUpCollection()
{
}

void MyScene::scoreIncrease(int value)
{
    m_score->addPoints(value);
}

void MyScene::playASound()
{
}

void MyScene::playHitWarp()
{
    emit playSound("warp");
}
