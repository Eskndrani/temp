#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMap>
#include <QString>
#include <QFileInfo>

class SoundManager : public QObject
{
    Q_OBJECT
public:
    explicit SoundManager(QObject *parent = nullptr);
    ~SoundManager();

    void playSound(const QString& soundName);
    void playBackgroundMusic(bool loop = true);
    void stopSound();
    void stopBackgroundMusic();
    void mute(bool muted);
    void setVolume(int volume); // 0-100 range
    void loadAudioFile(const QString& filename);
    void manageAudioChannels();

private:
    QMediaPlayer* m_backgroundMusic;
    QAudioOutput* m_backgroundAudio;
    QMap<QString, QString> m_soundFiles;
    bool m_isMuted;
    int m_volume; // 0-100 range
};

#endif // SOUNDMANAGER_H
