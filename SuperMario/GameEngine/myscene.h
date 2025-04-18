#ifndef MYSCENE_H
#define MYSCENE_H

#include <QGraphicsScene>
#include <QScrollBar>
#include <QTimer>
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include "sprite.h"
#include "effect.h"
#include "player.h"
#include "enemy.h"
#include "powerup.h"
#include "score.h"
#include "timer.h"
#include "soundmanager.h"
#include "physicsengine.h"
#include "inputmanager.h"

class MyScene : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(qreal jumpFactor READ jumpFactor WRITE setJumpFactor NOTIFY jumpFactorChanged)

public:
    MyScene(QScrollBar* scrollBar, QObject* parent = nullptr);
    ~MyScene();
    
    void timerEvent(QTimerEvent* event) override;
    qreal jumpFactor() const;
    void setJumpFactor(const qreal &jumpFactor);
    
    void startGame();
    void pauseGame();
    void resetGame();
    void gameOver();
    void levelComplete();
    void handleInputEvent(QKeyEvent* event);
    
    void updatePhysics();
    void manageLevelTransition();

signals:
    void jumpFactorChanged(qreal);
    void playSound(QString);
    void spawnMushroom();
    void spawnFlower();
    void stopMusic();
    void gameStarted();
    void gamePaused();
    void gameOverSignal();
    void levelCompleteSignal();

private:
    void initPlayField();
    void movePlayer();
    void jumpPlayer();
    void fallPlayer();
    void updateGameLogic();
    void updateCamera();
    void detectCollisions();
    void checkItemCollisions();
    void checkEnemyCollisions();
    bool handleCollisionWithPlatform();
    void processPowerUpCollection();
    void scoreIncrease(int value);
    void playASound();
    void playHitWarp();
    
    bool gameRunning;
    bool isPaused;
    int gameLevel;
    qreal m_jumpFactor;
    
    QScrollBar* m_scrollBar;
    
    Player* m_player;
    QList<Enemy*> m_enemies;
    QList<PowerUp*> m_powerups;
    QList<QGraphicsItem*> m_platforms;
    QGraphicsPixmapItem* m_background;
    Score* m_score;
    Timer* m_timer;
    SoundManager* m_soundManager;
    PhysicsEngine* m_physicsEngine;
    InputManager* m_inputManager;
    
    int m_timerId;
    
    qreal sceneWidth;
    qreal sceneHeight;
};

#endif