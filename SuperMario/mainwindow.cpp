#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    backgroundMusic(nullptr),
    audioOutput(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupGame();
    connect(gameScene, &MyScene::gameStarted, this, &MainWindow::startGame);
    connect(gameScene, &MyScene::gamePaused, this, &MainWindow::pauseGame);
    connect(gameScene, &MyScene::gameOverSignal, this, &MainWindow::gameOver);
    connect(gameScene, &MyScene::levelCompleteSignal, this, &MainWindow::levelComplete);
    connect(gameScene, &MyScene::playSound, this, &MainWindow::handleSound);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete gameScene;
    delete soundManager;

    if (backgroundMusic) {
        backgroundMusic->stop();
        delete backgroundMusic;
    }

    delete audioOutput;
}

void MainWindow::setCurrentUser(const QString &username)
{
    currentUser = username;
    ui->statusBar->showMessage("Player: " + username);
}

void MainWindow::startGame()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
        delete backgroundMusic;
    }

    if (audioOutput) {
        delete audioOutput;
    }

    backgroundMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    backgroundMusic->setAudioOutput(audioOutput);
    backgroundMusic->setSource(QUrl("qrc:/audio/level1.mp3"));
    audioOutput->setVolume(0.5);  // 50%
    backgroundMusic->play();

    ui->actionPause->setEnabled(true);
    ui->statusBar->showMessage("Game started - Player: " + currentUser);
}

void MainWindow::pauseGame()
{
    if (backgroundMusic) {
        if (backgroundMusic->playbackState() == QMediaPlayer::PlayingState) {
            backgroundMusic->pause();
        } else {
            backgroundMusic->play();
        }
    }

    bool paused = ui->actionPause->text() == "Resume";
    ui->actionPause->setText(paused ? "Pause" : "Resume");
    ui->statusBar->showMessage(paused ? "Game resumed" : "Game paused");
}

void MainWindow::resetGame()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
    }

    ui->actionPause->setText("Pause");
    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Game reset");
}

void MainWindow::gameOver()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
    }

    QMediaPlayer *gameOverSound = new QMediaPlayer(this);
    QAudioOutput *gameOverAudio = new QAudioOutput(this);
    gameOverSound->setAudioOutput(gameOverAudio);
    gameOverSound->setSource(QUrl("qrc:/audio/death.wav"));
    gameOverAudio->setVolume(0.7);  // 70%
    gameOverSound->play();

    // Use playbackStateChanged instead of stateChanged
    connect(gameOverSound, &QMediaPlayer::playbackStateChanged, [=](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState) {
            gameOverSound->deleteLater();
            gameOverAudio->deleteLater();
        }
    });

    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Game Over");
}

void MainWindow::levelComplete()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
    }

    QMediaPlayer *levelCompleteSound = new QMediaPlayer(this);
    QAudioOutput *levelCompleteAudio = new QAudioOutput(this);
    levelCompleteSound->setAudioOutput(levelCompleteAudio);
    levelCompleteSound->setSource(QUrl("qrc:/audio/levelclear.wav"));
    levelCompleteAudio->setVolume(0.7);  // 70%
    levelCompleteSound->play();

    // Use playbackStateChanged instead of stateChanged
    connect(levelCompleteSound, &QMediaPlayer::playbackStateChanged, [=](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState) {
            levelCompleteSound->deleteLater();
            levelCompleteAudio->deleteLater();
        }
    });

    ui->actionPause->setEnabled(false);
    ui->statusBar->showMessage("Level Complete!");
}

void MainWindow::handleSound(const QString &soundName)
{
    QMediaPlayer *sound = new QMediaPlayer(this);
    QAudioOutput *soundAudio = new QAudioOutput(this);
    sound->setAudioOutput(soundAudio);

    if (soundName == "jump") {
        sound->setSource(QUrl("qrc:/audio/jump.wav"));
    } else if (soundName == "coin") {
        sound->setSource(QUrl("qrc:/audio/coin.wav"));
    } else if (soundName == "powerup") {
        sound->setSource(QUrl("qrc:/audio/powerup.wav"));
    } else if (soundName == "shrink") {
        sound->setSource(QUrl("qrc:/audio/shrink.wav"));
    } else if (soundName == "kick") {
        sound->setSource(QUrl("qrc:/audio/kick.wav"));
    } else if (soundName == "sprout") {
        sound->setSource(QUrl("qrc:/audio/sprout.wav"));
    } else if (soundName == "warp") {
        sound->setSource(QUrl("qrc:/audio/hitwarptube.wav"));
    } else if (soundName == "fireball") {
        sound->setSource(QUrl("qrc:/audio/fireball.wav"));
    } else if (soundName == "fsprout") {
        sound->setSource(QUrl("qrc:/audio/fsprout.wav"));
    } else {
        sound->deleteLater();
        soundAudio->deleteLater();
        return;
    }

    soundAudio->setVolume(0.6);  // 60%
    sound->play();

    // Use playbackStateChanged instead of stateChanged
    connect(sound, &QMediaPlayer::playbackStateChanged, [=](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState) {
            sound->deleteLater();
            soundAudio->deleteLater();
        }
    });
}

void MainWindow::on_actionNew_Game_triggered()
{
    gameScene->resetGame();
    gameScene->startGame();
}

void MainWindow::on_actionPause_triggered()
{
    gameScene->pauseGame();
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
    gameScene->handleInputEvent(event);

    if (event->key() == Qt::Key_Escape) {
        close();
    }

    QMainWindow::keyPressEvent(event);
}
