#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollBar>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QMap>
#include "GameEngine/myscene.h"
#include "GameEngine/soundmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setCurrentUser(const QString &username);
public slots:
    void startGame();
    void pauseGame();
    void resetGame();
    void gameOver();
    void levelComplete();
    void handleSound(const QString &soundName);
private slots:
    void on_actionNew_Game_triggered();
    void on_actionPause_triggered();
    void on_actionExit_triggered();
    void on_actionSettings_triggered();
    void on_actionAbout_triggered();
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;
    MyScene *gameScene;
    QScrollBar *horizontalScrollBar;
    SoundManager *soundManager;
    QMediaPlayer *backgroundMusic; // Keeping for backward compatibility
    QAudioOutput *audioOutput;     // Keeping for backward compatibility
    QString currentUser;
    
    // New sound effect implementation
    QMap<QString, QSoundEffect*> soundEffects;
    void initializeSoundEffects();
    void stopBackgroundMusic();

    void setupGame();
    void setupUI();
};
#endif
