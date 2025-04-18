#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QGraphicsItem>
#include <QHash>
#include <QPointF>

class Animation;
class PowerUp;

enum class PlayerState {
    SMALL,
    SUPER,
    FIRE,
    DYING
};

enum class MovementState {
    STANDING,
    RUNNING,
    JUMPING,
    FALLING,
    CROUCHING
};

class Player : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Player(QGraphicsItem *parent = nullptr);
    ~Player();

    // QGraphicsItem required methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void advance(int phase) override;

    // Movement methods
    void move(const QString &direction);
    void jump();
    void land();
    void wallJump();
    void dash();
    void resetState();
    void updateMovement();

    // State control
    void setState(PlayerState state);
    PlayerState state() const { return m_state; }
    void setMovementState(MovementState state);
    MovementState movementState() const { return m_movementState; }

    // Animation control
    void updateAnimationState();
    void setCurrentAnimation(const QString &animationKey);

    // Power-up interactions
    void collectPowerUp(PowerUp *powerUp);
    void fireProjectile();

    // Damage handling
    void takeDamage();
    void die();
    void setInvincible(bool invincible);

    // State queries
    bool isAlive() const { return m_isAlive; }
    bool isInvincible() const { return m_isInvincible; }
    bool isFacingRight() const { return m_isFacingRight; }
    bool isJumping() const { return m_isJumping; }
    bool isFalling() const { return m_isFalling; }
    bool isCrouching() const { return m_isCrouching; }
    bool isTouchingWall() const;

    // Physics getters/setters
    QPointF velocity() const { return m_velocity; }
    void setVelocity(const QPointF &velocity) { m_velocity = velocity; }

    // Debug
    void debugInfo() const;

signals:
    void stateChanged(PlayerState newState);
    void jumpStarted();
    void landed();
    void tookDamage();
    void died();
    void projectileFired(QPointF position, bool facingRight);

private:
    // Private helper methods
    void setupCollisionBoxes();
    void loadAnimations();

    // Core state
    PlayerState m_state;
    Animation* m_currentAnimation;
    MovementState m_movementState;

    // Status flags
    bool m_isAlive;
    bool m_isInvincible;
    bool m_isFacingRight;
    bool m_isJumping;
    bool m_isFalling;
    bool m_isRunning;
    bool m_isCrouching;

    // Physics properties
    QPointF m_velocity;
    QPointF m_acceleration;
    float m_gravity;
    float m_jumpVelocity;

    // Collision boxes
    QRectF m_hitBox;
    QRectF m_feetBox;
    QRectF m_headBox;
    QRectF m_leftBox;
    QRectF m_rightBox;

    // Timers
    int m_invincibilityTimer;
    int m_fireballCooldown;

    // Double jump feature
    bool m_canDoubleJump;
    bool m_hasDoubleJumped;

    // Dash feature
    bool m_canDash;
    int m_dashCooldown;

    // Animation storage
    QHash<QString, Animation*> m_animations;
    QHash<PlayerState, QHash<QString, QString>> m_animationSets;
};

#endif // PLAYER_H
