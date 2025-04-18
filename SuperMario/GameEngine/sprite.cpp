#include "sprite.h"
#include <QTransform>
#include <QDebug>

Sprite::Sprite(QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsPixmapItem(parent),
    m_flipHorizontal(false),
    m_flipVertical(false),
    m_scaleX(1.0),
    m_scaleY(1.0),
    m_hasCustomCollisionBox(false)
{
    // Center the origin point of the sprite
    setTransformOriginPoint(boundingRect().center());
}

Sprite::~Sprite()
{
    // Delete all animations
    for (auto anim : m_animations.values()) {
        delete anim;
    }
    m_animations.clear();
}

void Sprite::addAnimation(const QString &name, Animation *animation)
{
    if (m_animations.contains(name)) {
        delete m_animations[name]; // Delete existing animation
    }

    m_animations[name] = animation;

    // Connect animation signals
    connect(animation, &Animation::frameChanged, this, &Sprite::updateFrame);
    connect(animation, &Animation::animationFinished, [this, name]() {
        emit animationFinished(name);
    });
    connect(animation, &Animation::animationLooped, [this, name]() {
        emit animationLooped(name);
    });

    // If this is the first animation, make it current
    if (m_currentAnimationName.isEmpty()) {
        m_currentAnimationName = name;
        updateFrame(animation->getCurrentFrameIndex());
    }
}

void Sprite::playAnimation(const QString &name)
{
    if (!m_animations.contains(name)) {
        qDebug() << "Animation not found:" << name;
        return;
    }

    // Stop current animation if any
    if (!m_currentAnimationName.isEmpty() && m_animations.contains(m_currentAnimationName)) {
        m_animations[m_currentAnimationName]->stopAnimation();
    }

    // Set and start new animation
    m_currentAnimationName = name;
    m_animations[name]->startAnimation();

    emit animationChanged(name);
}

Animation* Sprite::getAnimation(const QString &name) const
{
    return m_animations.value(name, nullptr);
}

void Sprite::stopAnimation()
{
    if (!m_currentAnimationName.isEmpty() && m_animations.contains(m_currentAnimationName)) {
        m_animations[m_currentAnimationName]->stopAnimation();
    }
}

QString Sprite::currentAnimationName() const
{
    return m_currentAnimationName;
}

void Sprite::loadSpriteSheet(const QString &path, int frameWidth, int frameCount)
{
    m_currentSpriteSheet.load(path);

    if (m_currentSpriteSheet.isNull()) {
        qDebug() << "Failed to load sprite sheet:" << path;
        return;
    }

    // Store the sprite sheet for later frame extraction
    m_currentSpriteSheet = m_currentSpriteSheet;
}

void Sprite::extractFramesFromSpriteSheet(const QString &animationName, int startFrame, int endFrame)
{
    if (m_currentSpriteSheet.isNull()) {
        qDebug() << "No sprite sheet loaded";
        return;
    }

    // Create a new animation if it doesn't exist
    if (!m_animations.contains(animationName)) {
        m_animations[animationName] = new Animation(this);

        // Connect signals
        connect(m_animations[animationName], &Animation::frameChanged, this, &Sprite::updateFrame);
        connect(m_animations[animationName], &Animation::animationFinished, [this, animationName]() {
            emit animationFinished(animationName);
        });
        connect(m_animations[animationName], &Animation::animationLooped, [this, animationName]() {
            emit animationLooped(animationName);
        });
    }

    Animation* animation = m_animations[animationName];

    // Calculate frame dimensions
    int frameHeight = m_currentSpriteSheet.height();
    int frameWidth = m_currentSpriteSheet.width() / (endFrame - startFrame + 1);

    // Extract and add frames
    for (int i = startFrame; i <= endFrame; ++i) {
        QPixmap frame = m_currentSpriteSheet.copy(i * frameWidth, 0, frameWidth, frameHeight);
        animation->addFrame(frame);
    }
}

void Sprite::setScale(qreal sx, qreal sy)
{
    m_scaleX = sx;
    m_scaleY = sy;
    applyTransforms();
}

void Sprite::flip(bool horizontal)
{
    m_flipHorizontal = horizontal;
    applyTransforms();
}

void Sprite::flipVertical(bool vertical)
{
    m_flipVertical = vertical;
    applyTransforms();
}

bool Sprite::isFlippedHorizontal() const
{
    return m_flipHorizontal;
}

bool Sprite::isFlippedVertical() const
{
    return m_flipVertical;
}

void Sprite::setRotation(qreal angle)
{
    QGraphicsPixmapItem::setRotation(angle);
}

QPixmap Sprite::getCurrentFrame() const
{
    if (m_currentAnimationName.isEmpty() || !m_animations.contains(m_currentAnimationName)) {
        return QPixmap();
    }

    return m_animations[m_currentAnimationName]->getCurrentFrame();
}

void Sprite::updateFrame(int frameIndex)
{
    if (m_currentAnimationName.isEmpty() || !m_animations.contains(m_currentAnimationName)) {
        return;
    }

    // Get the current frame from the animation
    m_currentFrame = m_animations[m_currentAnimationName]->getCurrentFrame();

    // Apply transformations and set the pixmap
    applyTransforms();
}

void Sprite::applyTransforms()
{
    if (m_currentFrame.isNull()) {
        return;
    }

    QPixmap transformedFrame = m_currentFrame;

    // Create transform for scaling and flipping
    QTransform transform;
    transform.scale(
        m_flipHorizontal ? -m_scaleX : m_scaleX,
        m_flipVertical ? -m_scaleY : m_scaleY
        );

    // Apply transforms
    transformedFrame = m_currentFrame.transformed(transform);

    // Update the pixmap
    setPixmap(transformedFrame);

    // Center the origin point after transform
    setTransformOriginPoint(boundingRect().center());
}

void Sprite::setCollisionBox(const QRectF &box)
{
    m_customCollisionBox = box;
    m_hasCustomCollisionBox = true;
}

QRectF Sprite::collisionBox() const
{
    if (m_hasCustomCollisionBox) {
        // Return custom collision box adjusted by position
        return QRectF(pos().x() + m_customCollisionBox.x(),
                      pos().y() + m_customCollisionBox.y(),
                      m_customCollisionBox.width(),
                      m_customCollisionBox.height());
    }

    // Default to bounding rect if no custom collision box is set
    return boundingRect().translated(pos());
}

// New methods to fix compilation errors

void Sprite::changeSprite(const QString &path)
{
    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        // Set the pixmap directly
        setPixmap(pixmap);
        m_currentFrame = pixmap;

        // Center the origin point after changing the sprite
        setTransformOriginPoint(boundingRect().center());
    } else {
        qDebug() << "Failed to load sprite:" << path;
    }
}

void Sprite::animate()
{
    // Update current animation if it exists
    if (!m_currentAnimationName.isEmpty() && m_animations.contains(m_currentAnimationName)) {
        m_animations[m_currentAnimationName]->startAnimation();
    }
}
