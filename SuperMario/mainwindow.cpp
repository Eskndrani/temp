#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QSoundEffect>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gameScene(nullptr),
    horizontalScrollBar(nullptr),
    soundManager(nullptr),
    backgroundMusic(nullptr),
    audioOutput(nullptr),
    currentUser("Fares2411")  // Initialize with your username
{
    ui->setupUi(this);
    setupUI();
    setupGame();
    
    // Move these connections after setupGame to ensure gameScene is initialized
    if (gameScene) {
        connect(gameScene, &MyScene::gameStarted, this, &MainWindow::startGame);
        connect(gameScene, &MyScene::gamePaused, this, &MainWindow::pauseGame);
        connect(gameScene, &MyScene::gameOverSignal, this, &MainWindow::gameOver);
        connect(gameScene, &MyScene::levelCompleteSignal, this, &MainWindow::levelComplete);
        connect(gameScene, &MyScene::playSound, this, &MainWindow::handleSound);
    }
    
    // Initialize our sound effects pool
    initializeSoundEffects();
}

MainWindow::~MainWindow()
{
    delete ui;
    
    // Clear all sound effects from our map
    qDeleteAll(soundEffects);
    soundEffects.clear();
    
    // These are created in setupGame, so only delete if setupGame was called
    if (gameScene) {
        delete gameScene;
        gameScene = nullptr;
    }
    
    if (soundManager) {
        delete soundManager;
        soundManager = nullptr;
    }
    
    // horizontalScrollBar is parented to graphicsView, so it will be deleted by Qt's parent-child system
}

void MainWindow::initializeSoundEffects()
{
    // Create a map of sound effects for quick access
    QStringList soundNames = {
        "jump", "coin", "powerup", "shrink", "kick", 
        "sprout", "warp", "fireball", "fsprout", 
        "death", "levelclear", "level1"
    };
    
    for (const QString &name : soundNames) {
        QSoundEffect *effect = new QSoundEffect(this);
        effect->setSource(QUrl(QString("qrc:/audio/%1.wav").arg(name)));
        effect->setVolume(0.6f); // Default volume at 60%
        soundEffects[name] = effect;
    }
    
    // Set special volumes for certain effects
    if (soundEffects.contains("level1")) {
        soundEffects["level1"]->setVolume(0.5f); // Background music at 50%
    }
    if (soundEffects.contains("death")) {
        soundEffects["death"]->setVolume(0.7f); // Death sound at 70%
    }
    if (soundEffects.contains("levelclear")) {
        soundEffects["levelclear"]->setVolume(0.7f); // Level clear at 70%
    }
}

void MainWindow::setCurrentUser(const QString &username)
{
    currentUser = username;
    ui->statusBar->showMessage("Player: " + username);
}

void MainWindow::startGame()
{
    // Stop any currently playing background music
    stopBackgroundMusic();
    
    // Start the level music
    if (soundEffects.contains("level1")) {
        soundEffects["level1"]->setLoopCount(QSoundEffect::Infinite);
        soundEffects["level1"]->play();
    }

    ui->actionPause->setEnabled(true);
    ui->statusBar->showMessage("Game started - Player: " + currentUser);
}

void MainWindow::pauseGame()
{
    // Toggle background music pause/play
    if (soundEffects.contains("level1")) {
        if (soundEffects["level1"]->isPlaying()) {
            soundEffects["level1"]->stop();
        } else {
            soundEffects["level1"]->play();
        }
    }

    bool paused = ui->actionPause->text() == "Resume";
    ui->actionPause->setText(paused ? "Pause" : "Resume");
    ui->statusBar->showMessage(paused ? "Game resumed" : "Game paused");
}

void MainWindow::resetGame()
{
    stopBackgroundMusic();
    ui->actionPause->setText("Pause");
    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Game reset");
}

void MainWindow::stopBackgroundMusic()
{
    // Stop any background music that might be playing
    if (soundEffects.contains("level1") && soundEffects["level1"]->isPlaying()) {
        soundEffects["level1"]->stop();
    }
}

void MainWindow::gameOver()
{
    stopBackgroundMusic();
    
    // Play game over sound
    if (soundEffects.contains("death")) {
        soundEffects["death"]->play();
    }

    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Game Over");
}

void MainWindow::levelComplete()
{
    stopBackgroundMusic();
    
    // Play level complete sound
    if (soundEffects.contains("levelclear")) {
        soundEffects["levelclear"]->play();
    }

    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Level Complete!");
}

void MainWindow::handleSound(const QString &soundName)
{
    // Play the requested sound effect if it exists in our map
    if (soundEffects.contains(soundName)) {
        // Stop the sound if it's currently playing to restart it
        if (soundEffects[soundName]->isPlaying()) {
            soundEffects[soundName]->stop();
        }
        soundEffects[soundName]->play();
    }
}

void MainWindow::on_actionNew_Game_triggered()
{
    if (gameScene) {
        gameScene->resetGame();
        gameScene->startGame();
    }
}

void MainWindow::on_actionPause_triggered()
{
    if (gameScene) {
        gameScene->pauseGame();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionSettings_triggered()
{
    QMessageBox::information(this, "Settings", "Settings dialog will be implemented here.");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About Super Mario",
                       "Super Mario Game\n"
                       "Version 1.0\n"
                       "Created: 2025-04-18");
}

void MainWindow::setupGame()
{
    horizontalScrollBar = new QScrollBar(Qt::Horizontal);
    horizontalScrollBar->setRange(0, 5000);
    horizontalScrollBar->setPageStep(800);

    gameScene = new MyScene(horizontalScrollBar);

    ui->graphicsView->setScene(gameScene);
    ui->graphicsView->setHorizontalScrollBar(horizontalScrollBar);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    soundManager = new SoundManager();
    // We're creating a SoundManager but using our own sound implementation for now.
}

void MainWindow::setupUI()
{
    setWindowTitle("Super Mario Game");
    setWindowIcon(QIcon(":/images/icon.ico"));
    setFixedSize(800, 650);
    ui->graphicsView->setFixedSize(800, 600);
    ui->actionPause->setEnabled(false);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (gameScene) {
        gameScene->handleInputEvent(event);
    }

    if (event->key() == Qt::Key_Escape) {
        close();
    }

    QMainWindow::keyPressEvent(event);
}
