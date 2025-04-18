#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QPixmap>
#include <QVector>
#include <QTimer>

class Animation : public QObject
{
    Q_OBJECT

public:
    explicit Animation(QObject *parent = nullptr);
    ~Animation();

    // Animation control
    void startAnimation();
    void stopAnimation();
    void pauseAnimation();
    void resetAnimation();
    void setFrameRate(int fps);
    void loopAnimation(bool loop);
    void setDuration(int milliseconds);

    // Frame management
    void addFrame(const QPixmap &frame);
    void addFrames(const QVector<QPixmap> &frames);
    void clearFrames();
    void setFrameSequence(const QVector<int> &sequence);
    void resetFrameSequence();
    void calculateFrameSequence();
    
    // NEW: Sprite sheet methods
    void loadSpriteSheet(const QPixmap &spriteSheet, int frameWidth, int frameHeight, 
                         int numFrames = -1, bool clearExistingFrames = true);
    bool loadSpriteSheetFromFile(const QString &filePath, int frameWidth, int frameHeight, 
                                int numFrames = -1, bool clearExistingFrames = true);

    // Getters
    QPixmap getCurrentFrame() const;
    int getCurrentFrameIndex() const;
    int getFrameCount() const;
    bool isRunning() const;
    bool isLooping() const;

signals:
    void frameChanged(int frameIndex);
    void animationFinished();
    void animationLooped();

private slots:
    void nextFrame();

private:
    QVector<QPixmap> m_frames;
    QVector<int> m_frameSequence;
    int m_currentFrameIndex;
    int m_frameRate;
    int m_duration;
    bool m_looping;
    bool m_running;
    QTimer *m_timer;
    
    // NEW: Helper method to slice a sprite sheet
    void sliceSpriteSheet(const QPixmap &spriteSheet, int frameWidth, int frameHeight, int numFrames);
};

#endif // ANIMATION_H
