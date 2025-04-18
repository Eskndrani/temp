#include "soundmanager.h"
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QAudioOutput>

SoundManager::SoundManager(QObject *parent)
    : QObject(parent),
    m_backgroundMusic(nullptr),
    m_backgroundAudio(nullptr),
    m_isMuted(false),
    m_volume(100)
{
    m_soundFiles["jump"] = ":/audio/jump.wav";
    m_soundFiles["coin"] = ":/audio/coin.wav";
    m_soundFiles["powerup"] = ":/audio/powerup.wav";
    m_soundFiles["death"] = ":/audio/death.wav";
    m_soundFiles["warp"] = ":/audio/hitwarptube.wav";
    m_soundFiles["sprout"] = ":/audio/sprout.wav";
    m_soundFiles["kick"] = ":/audio/kick.wav";
    m_soundFiles["fireball"] = ":/audio/fireball.wav";
    m_soundFiles["fsprout"] = ":/audio/fsprout.wav";
    m_soundFiles["level_complete"] = ":/audio/levelclear.wav";
    m_soundFiles["shrink"] = ":/audio/shrink.wav";

    // Initialize background music player
    m_backgroundMusic = new QMediaPlayer(this);
    m_backgroundAudio = new QAudioOutput(this);
    m_backgroundMusic->setAudioOutput(m_backgroundAudio);
}

SoundManager::~SoundManager()
{
    if (m_backgroundMusic) {
        m_backgroundMusic->stop();
    }
    // QMediaPlayer and QAudioOutput will be deleted by parent-child relationship
}

void SoundManager::playSound(const QString& soundName)
{
    if (m_isMuted) return;

    if (m_soundFiles.contains(soundName)) {
        QMediaPlayer* player = new QMediaPlayer(this);
        QAudioOutput* audio = new QAudioOutput(this);
        player->setAudioOutput(audio);
        player->setSource(QUrl(m_soundFiles[soundName]));
        audio->setVolume(m_volume / 100.0); // Convert to 0.0-1.0 range
        player->play();

        connect(player, &QMediaPlayer::playbackStateChanged, [=](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState) {
                player->deleteLater();
                audio->deleteLater();
            }
        });
    }
}

void SoundManager::playBackgroundMusic(bool loop)
{
    if (m_isMuted) return;

    m_backgroundMusic->stop();
    m_backgroundMusic->setSource(QUrl(":/audio/level1.mp3"));
    m_backgroundAudio->setVolume(m_volume / 100.0); // Convert to 0.0-1.0 range
    m_backgroundMusic->play();

    if (loop) {
        connect(m_backgroundMusic, &QMediaPlayer::playbackStateChanged, this, [=](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState) {
                m_backgroundMusic->play();
            }
        }, Qt::UniqueConnection); // Use UniqueConnection to avoid multiple connections
    } else {
        // Disconnect any previous connections to ensure it doesn't loop
        disconnect(m_backgroundMusic, &QMediaPlayer::playbackStateChanged, nullptr, nullptr);
    }
}

void SoundManager::stopSound()
{
    // Stop all sounds (in a real implementation, you'd need to track the active sounds)
    // For now, this is left empty as in the original code
    // Consider implementing a container to hold active sound players if needed
}

void SoundManager::stopBackgroundMusic()
{
    if (m_backgroundMusic) {
        m_backgroundMusic->stop();
    }
}

void SoundManager::mute(bool muted)
{
    m_isMuted = muted;

    if (m_backgroundAudio) {
        if (m_isMuted) {
            m_backgroundAudio->setVolume(0);
        } else {
            m_backgroundAudio->setVolume(m_volume / 100.0); // Convert to 0.0-1.0 range
        }
    }
}

void SoundManager::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);

    if (m_backgroundAudio && !m_isMuted) {
        m_backgroundAudio->setVolume(m_volume / 100.0); // Convert to 0.0-1.0 range
    }
}

void SoundManager::loadAudioFile(const QString& filename)
{
    // Implementation to load an audio file from the filesystem
    QFileInfo fileInfo(filename);
    if (fileInfo.exists() && fileInfo.isFile()) {
        QString baseName = fileInfo.baseName();
        m_soundFiles[baseName] = filename;
        qDebug() << "Loaded audio file:" << baseName << "->" << filename;
    } else {
        qDebug() << "Could not load audio file:" << filename;
    }
}

void SoundManager::manageAudioChannels()
{
    // Implementation to manage audio channels (e.g., limit number of concurrent sounds)
    // This could track active players and ensure we don't exceed a certain number
    // For example, keeping a QList<QMediaPlayer*> of active players
    // and removing them when they finish playing

    // For now, this is a placeholder implementation
    qDebug() << "Managing audio channels";
    // In a real implementation, you might want to:
    // 1. Count active sound players
    // 2. If too many are active, stop the oldest ones
    // 3. Prioritize certain sound types
}
