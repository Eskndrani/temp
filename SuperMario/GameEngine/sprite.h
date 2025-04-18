#ifndef SPRITE_H
#define SPRITE_H
#include <QGraphicsPixmapItem>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QPointF>
#include <QTimer>
#include "animation.h"

class Sprite : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit Sprite(QGraphicsItem *parent = nullptr);
    virtual ~Sprite();

    // Animation handling
    void addAnimation(const QString &name, Animation *animation);
    void playAnimation(const QString &name);
    Animation* getAnimation(const QString &name) const;
    void stopAnimation();
    QString currentAnimationName() const;

    // Spritesheet loading
    void loadSpriteSheet(const QString &path, int frameWidth, int frameCount);
    void extractFramesFromSpriteSheet(const QString &animationName, int startFrame, int endFrame);

    // Transform methods
    void setScale(qreal sx, qreal sy);
    void flip(bool horizontal);
    void flipVertical(bool vertical);
    bool isFlippedHorizontal() const;
    bool isFlippedVertical() const;
    void setRotation(qreal angle);  // Overrides QGraphicsItem::setRotation

    // State getters
    QPixmap getCurrentFrame() const;
    QRectF collisionBox() const;

    // Set custom collision box (relative to sprite position)
    void setCollisionBox(const QRectF &box);

    // Additional methods needed by other classes
    void changeSprite(const QString &path);
    void animate();

public slots:
    void updateFrame(int frameIndex);

signals:
    void animationChanged(const QString &name);
    void animationFinished(const QString &name);
    void animationLooped(const QString &name);

private:
    void applyTransforms();

    QMap<QString, Animation*> m_animations;
    QString m_currentAnimationName;
    QPixmap m_currentSpriteSheet;
    QPixmap m_currentFrame;
    bool m_flipHorizontal;
    bool m_flipVertical;
    qreal m_scaleX;
    qreal m_scaleY;
    QRectF m_customCollisionBox;
    bool m_hasCustomCollisionBox;
};

#endif // SPRITE_H
