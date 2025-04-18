#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QPixmap>

class Animation : public QObject
{
    Q_OBJECT

public:
    explicit Animation(QObject *parent = nullptr);
    ~Animation();

    // Control methods
    void startAnimation();
    void stopAnimation();
    void pauseAnimation();
    void resetAnimation();

    // Configuration methods
    void setFrameRate(int fps);
    void loopAnimation(bool loop);
    void setDuration(int milliseconds);

    // Frame management
    void addFrame(const QPixmap &frame);
    void addFrames(const QVector<QPixmap> &frames);
    void clearFrames();

    // Frame sequence management
    void setFrameSequence(const QVector<int> &sequence);
    void resetFrameSequence();

    // State accessors
    QPixmap getCurrentFrame() const;
    int getCurrentFrameIndex() const;
    int getFrameCount() const;
    bool isRunning() const;
    bool isLooping() const;

public slots:
    void nextFrame();

signals:
    void frameChanged(int frameIndex);
    void animationFinished();
    void animationLooped();

private:
    void calculateFrameSequence();

    QVector<QPixmap> m_frames;
    QVector<int> m_frameSequence;
    QTimer *m_timer;
    int m_currentFrameIndex;
    int m_frameRate;
    int m_duration;
    bool m_looping;
    bool m_running;
};

#endif // ANIMATION_H
