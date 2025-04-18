#include "animation.h"
#include <QDebug>

Animation::Animation(QObject *parent)
    : QObject(parent),
    m_currentFrameIndex(0),
    m_frameRate(60),
    m_duration(0),
    m_looping(false),
    m_running(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Animation::nextFrame);
}

Animation::~Animation()
{
    stopAnimation();
}

void Animation::startAnimation()
{
    if (m_frames.isEmpty()) {
        qDebug() << "Cannot start animation: No frames added";
        return;
    }

    if (!m_running) {
        m_running = true;
        m_timer->start(1000 / m_frameRate); // Convert fps to milliseconds
        emit frameChanged(m_currentFrameIndex);
    }
}

void Animation::stopAnimation()
{
    if (m_running) {
        m_timer->stop();
        m_running = false;
        resetAnimation();
    }
}

void Animation::pauseAnimation()
{
    if (m_running) {
        m_timer->stop();
        m_running = false;
    }
}

void Animation::resetAnimation()
{
    m_currentFrameIndex = 0;
    emit frameChanged(m_currentFrameIndex);
}

void Animation::setFrameRate(int fps)
{
    if (fps <= 0) {
        qDebug() << "Invalid frame rate:" << fps;
        return;
    }

    m_frameRate = fps;

    if (m_running) {
        m_timer->setInterval(1000 / m_frameRate);
    }
}

void Animation::loopAnimation(bool loop)
{
    m_looping = loop;
}

void Animation::setDuration(int milliseconds)
{
    if (milliseconds < 0) {
        qDebug() << "Invalid duration:" << milliseconds;
        return;
    }

    m_duration = milliseconds;

    // If we have a duration and frames, calculate the appropriate frame rate
    if (m_duration > 0 && !m_frames.isEmpty()) {
        int frameRate = (m_frames.size() * 1000) / m_duration;
        setFrameRate(frameRate);
    }
}

void Animation::addFrame(const QPixmap &frame)
{
    m_frames.append(frame);

    // If this is the first frame and we have a frame sequence,
    // automatically calculate a new frame sequence
    if (m_frames.size() == 1 && !m_frameSequence.isEmpty()) {
        calculateFrameSequence();
    }
}

void Animation::addFrames(const QVector<QPixmap> &frames)
{
    m_frames.append(frames);

    // If this is the first set of frames and we have a frame sequence,
    // automatically calculate a new frame sequence
    if (!frames.isEmpty() && !m_frameSequence.isEmpty()) {
        calculateFrameSequence();
    }
}

void Animation::clearFrames()
{
    m_frames.clear();
    m_frameSequence.clear();
    m_currentFrameIndex = 0;
}

void Animation::setFrameSequence(const QVector<int> &sequence)
{
    // Validate frame sequence
    for (int index : sequence) {
        if (index < 0 || (index >= m_frames.size() && !m_frames.isEmpty())) {
            qDebug() << "Invalid frame index in sequence:" << index;
            return;
        }
    }

    m_frameSequence = sequence;
}

void Animation::resetFrameSequence()
{
    m_frameSequence.clear();
    calculateFrameSequence();
}

void Animation::calculateFrameSequence()
{
    m_frameSequence.clear();
    for (int i = 0; i < m_frames.size(); ++i) {
        m_frameSequence.append(i);
    }
}

QPixmap Animation::getCurrentFrame() const
{
    if (m_frames.isEmpty()) {
        return QPixmap();
    }

    int frameIndex = m_currentFrameIndex;
    if (!m_frameSequence.isEmpty()) {
        frameIndex = m_frameSequence.at(m_currentFrameIndex % m_frameSequence.size());
    }

    return m_frames.at(frameIndex % m_frames.size());
}

int Animation::getCurrentFrameIndex() const
{
    if (m_frameSequence.isEmpty()) {
        return m_currentFrameIndex;
    } else {
        return m_frameSequence.at(m_currentFrameIndex % m_frameSequence.size());
    }
}

int Animation::getFrameCount() const
{
    return m_frames.size();
}

bool Animation::isRunning() const
{
    return m_running;
}

bool Animation::isLooping() const
{
    return m_looping;
}

void Animation::nextFrame()
{
    // Increment the frame index
    m_currentFrameIndex++;

    int effectiveSequenceLength = m_frameSequence.isEmpty() ? m_frames.size() : m_frameSequence.size();

    // Check if we've reached the end of the animation
    if (m_currentFrameIndex >= effectiveSequenceLength) {
        if (m_looping) {
            // Loop back to the beginning
            m_currentFrameIndex = 0;
            emit animationLooped();
        } else {
            // Stop the animation
            m_timer->stop();
            m_running = false;
            emit animationFinished();
            return;
        }
    }

    // Emit frame changed signal
    emit frameChanged(getCurrentFrameIndex());
}

// New methods for sprite sheet handling

void Animation::loadSpriteSheet(const QPixmap &spriteSheet, int frameWidth, int frameHeight, 
                               int numFrames, bool clearExistingFrames)
{
    if (spriteSheet.isNull()) {
        qDebug() << "Cannot load sprite sheet: Image is null";
        return;
    }
    
    if (frameWidth <= 0 || frameHeight <= 0) {
        qDebug() << "Invalid frame dimensions:" << frameWidth << "x" << frameHeight;
        return;
    }
    
    if (clearExistingFrames) {
        clearFrames();
    }
    
    sliceSpriteSheet(spriteSheet, frameWidth, frameHeight, numFrames);
}

bool Animation::loadSpriteSheetFromFile(const QString &filePath, int frameWidth, int frameHeight, 
                                       int numFrames, bool clearExistingFrames)
{
    QPixmap spriteSheet(filePath);
    if (spriteSheet.isNull()) {
        qDebug() << "Failed to load sprite sheet from:" << filePath;
        return false;
    }
    
    loadSpriteSheet(spriteSheet, frameWidth, frameHeight, numFrames, clearExistingFrames);
    return true;
}

void Animation::sliceSpriteSheet(const QPixmap &spriteSheet, int frameWidth, int frameHeight, int numFrames)
{
    int sheetWidth = spriteSheet.width();
    int sheetHeight = spriteSheet.height();
    
    int cols = sheetWidth / frameWidth;
    int rows = sheetHeight / frameHeight;
    
    int totalFramesInSheet = cols * rows;
    int framesToExtract = (numFrames > 0 && numFrames <= totalFramesInSheet) ? numFrames : totalFramesInSheet;
    
    m_frames.reserve(m_frames.size() + framesToExtract);
    
    int frameCount = 0;
    for (int row = 0; row < rows && frameCount < framesToExtract; ++row) {
        for (int col = 0; col < cols && frameCount < framesToExtract; ++col) {
            QRect frameRect(col * frameWidth, row * frameHeight, frameWidth, frameHeight);
            QPixmap frame = spriteSheet.copy(frameRect);
            m_frames.append(frame);
            frameCount++;
        }
    }
    
    // If we added frames and have no sequence yet, calculate one
    if (frameCount > 0 && m_frameSequence.isEmpty()) {
        calculateFrameSequence();
    }
    
    qDebug() << "Extracted" << frameCount << "frames from sprite sheet";
}
