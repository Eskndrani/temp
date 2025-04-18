#include "player.h"
#include "powerup.h"
#include "animation.h"
#include <QDebug>
#include <QtMath>
#include <QPainter>

Player::Player(QGraphicsItem *parent)
    : QObject(nullptr),
    QGraphicsItem(parent),
    m_state(PlayerState::SMALL),
    m_currentAnimation(nullptr),
    m_movementState(MovementState::STANDING),
    m_isAlive(true),
    m_isInvincible(false),
    m_isFacingRight(true),
    m_isJumping(false),
    m_isFalling(false),
    m_isRunning(false),
    m_isCrouching(false),
    m_velocity(QPointF(0, 0)),
    m_acceleration(QPointF(0, 0)),
    m_gravity(0.5f),
    m_jumpVelocity(15.0f),
    m_invincibilityTimer(0),
    m_fireballCooldown(0),
    m_canDoubleJump(false),
    m_hasDoubleJumped(false),
    m_canDash(true),
    m_dashCooldown(0)
{
    // Set the object name for debugging
    setObjectName("Player");

    // Set data for collision detection
    setData(0, "player");

    // Initialize animations
    loadAnimations();

    // Set up collision boxes
    setupCollisionBoxes();

    // Initialize with small Mario
    setState(PlayerState::SMALL);

    // Start with idle animation
    updateAnimationState();
}

Player::~Player()
{
    // Clean up animations
    for (Animation* animation : m_animations.values()) {
        delete animation;
    }
    m_animations.clear();
}

QRectF Player::boundingRect() const
{
    // Return the bounding rectangle that contains all collision boxes
    return m_hitBox.united(m_feetBox).united(m_headBox).united(m_leftBox).united(m_rightBox);
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // If there's no current animation, just draw a placeholder
    if (!m_currentAnimation) {
        painter->setPen(Qt::red);
        painter->drawRect(boundingRect());
        return;
    }

    // Draw the current animation frame
    QPixmap frame = m_currentAnimation->getCurrentFrame();

    // Flip the frame if facing left
    if (!m_isFacingRight) {
        frame = frame.transformed(QTransform().scale(-1, 1));
    }

    // Draw the sprite
    painter->drawPixmap(boundingRect().topLeft(), frame);

    // Draw collision boxes in debug mode
#ifdef DEBUG_COLLISION_BOXES
    painter->setPen(Qt::red);
    painter->drawRect(m_hitBox);
    painter->setPen(Qt::blue);
    painter->drawRect(m_feetBox);
    painter->setPen(Qt::green);
    painter->drawRect(m_headBox);
    painter->setPen(Qt::yellow);
    painter->drawRect(m_leftBox);
    painter->setPen(Qt::cyan);
    painter->drawRect(m_rightBox);
#endif
}

void Player::jump()
{
    if (!m_isAlive) return;

    if (m_movementState == MovementState::JUMPING || m_movementState == MovementState::FALLING) {
        // Allow double jump if enabled and not already used
        if (m_canDoubleJump && !m_hasDoubleJumped) {
            m_velocity.setY(-m_jumpVelocity * 0.8f); // Slightly reduced force for double jump
            m_hasDoubleJumped = true;
            updateAnimationState();
            emit jumpStarted();
        }
        return;
    }

    // Regular jump
    setMovementState(MovementState::JUMPING);
    m_velocity.setY(-m_jumpVelocity);
    m_hasDoubleJumped = false; // Reset double jump tracker
    updateAnimationState();
    emit jumpStarted();
}

void Player::wallJump()
{
    if (!m_isAlive || !isTouchingWall()) return;

    // Flip direction and apply upward velocity
    m_isFacingRight = !m_isFacingRight;
    m_velocity.setX(m_isFacingRight ? 10.0f : -10.0f);
    m_velocity.setY(-12.0f); // Special wall jump velocity
    setMovementState(MovementState::JUMPING);
    updateAnimationState();
    emit jumpStarted();
}

void Player::dash()
{
    if (!m_isAlive || !m_canDash || m_dashCooldown > 0) return;

    // Apply horizontal dash velocity
    float dashSpeed = 20.0f;
    m_velocity.setX(m_isFacingRight ? dashSpeed : -dashSpeed);
    m_dashCooldown = 30; // Cooldown frames (half a second at 60fps)
    m_canDash = false;   // Disable further dashing until cooldown resets
    updateAnimationState();
}

void Player::land()
{
    m_hasDoubleJumped = false;  // Reset double jump
    m_canDash = true;          // Reset dash
    setMovementState(MovementState::STANDING);
    updateAnimationState();
    emit landed();
}

void Player::advance(int phase)
{
    if (!phase) return;

    // Update player using delta time of 1/60 second
    float deltaTime = 1.0f / 60.0f;

    if (!m_isAlive && m_state != PlayerState::DYING) return;

    // Handle invincibility timer
    if (m_invincibilityTimer > 0) {
        m_invincibilityTimer--;

        if (m_invincibilityTimer <= 0) {
            setInvincible(false);
        }
    }

    // Handle fireball cooldown
    if (m_fireballCooldown > 0) {
        m_fireballCooldown--;
    }

    // Handle dash cooldown
    if (m_dashCooldown > 0) {
        m_dashCooldown--;
        if (m_dashCooldown <= 0) {
            m_canDash = true;
        }
    }

    // Update velocity based on acceleration
    m_velocity += m_acceleration * deltaTime;

    // Apply gravity
    m_velocity.setY(m_velocity.y() + m_gravity);

    // Apply max velocity constraints
    float m_maxVelocity = 5.0f; // Max horizontal speed
    if (m_velocity.x() > m_maxVelocity) {
        m_velocity.setX(m_maxVelocity);
    } else if (m_velocity.x() < -m_maxVelocity) {
        m_velocity.setX(-m_maxVelocity);
    }

    // Terminal velocity for falling
    if (m_velocity.y() > m_maxVelocity * 2) {
        m_velocity.setY(m_maxVelocity * 2);
    }

    // Update position based on velocity
    setPos(pos() + m_velocity * deltaTime);

    // Update movement state
    updateMovement();

    // Update the animation state based on current player state
    updateAnimationState();

    // Update the current animation frame
    if (m_currentAnimation) {
        m_currentAnimation->nextFrame();
    }
}

bool Player::isTouchingWall() const
{
    // Implement collision detection with walls
    // This is a placeholder; actual implementation would check scene items
    QList<QGraphicsItem*> collisions = scene()->collidingItems(this);

    for (QGraphicsItem* item : collisions) {
        if (item->data(0).toString() == "wall") {
            // Calculate if we're touching with left or right side
            QRectF itemRect = item->sceneBoundingRect();
            if ((m_leftBox.translated(scenePos()).intersects(itemRect) && !m_isFacingRight) ||
                (m_rightBox.translated(scenePos()).intersects(itemRect) && m_isFacingRight)) {
                return true;
            }
        }
    }

    return false;
}

void Player::move(const QString &direction)
{
    if (!m_isAlive) return;
    float moveAcceleration = 0.5f;

    if (direction == "left") {
        m_acceleration.setX(-moveAcceleration);
        m_isFacingRight = false;
        if (m_movementState != MovementState::JUMPING &&
            m_movementState != MovementState::FALLING) {
            setMovementState(MovementState::RUNNING);
        }
    } else if (direction == "right") {
        m_acceleration.setX(moveAcceleration);
        m_isFacingRight = true;
        if (m_movementState != MovementState::JUMPING &&
            m_movementState != MovementState::FALLING) {
            setMovementState(MovementState::RUNNING);
        }
    } else if (direction == "stop") {
        m_acceleration.setX(0);

        if (m_movementState != MovementState::JUMPING &&
            m_movementState != MovementState::FALLING) {
            setMovementState(MovementState::STANDING);
        }
        // Apply friction to slow down
        if (qAbs(m_velocity.x()) > 0.1f) {
            float friction = (m_velocity.x() > 0) ? -0.3f : 0.3f;
            m_velocity.setX(m_velocity.x() + friction);
        } else {
            m_velocity.setX(0);
        }
    } else if (direction == "down" &&
               m_movementState != MovementState::JUMPING &&
               m_movementState != MovementState::FALLING) {
        setMovementState(MovementState::CROUCHING);
    } else if (direction == "up" &&
               m_movementState == MovementState::CROUCHING) {
        setMovementState(MovementState::STANDING);
    }
}

void Player::updateMovement()
{
    // Check if falling
    if (m_velocity.y() > 0) {
        setMovementState(MovementState::FALLING);
    } else if (m_velocity.y() < 0) {
        setMovementState(MovementState::JUMPING);
    } else {
        // Only change to standing if we were previously falling
        if (m_movementState == MovementState::FALLING) {
            // If we're moving horizontally, set to running
            if (qAbs(m_velocity.x()) > 0.1f) {
                setMovementState(MovementState::RUNNING);
            } else {
                setMovementState(MovementState::STANDING);
            }
        }
    }
}

void Player::resetState()
{
    m_state = PlayerState::SMALL;
    m_movementState = MovementState::STANDING;
    m_isAlive = true;
    m_isInvincible = false;
    m_isFacingRight = true;
    m_isJumping = false;
    m_isFalling = false;
    m_isRunning = false;
    m_isCrouching = false;
    m_velocity = QPointF(0, 0);
    m_acceleration = QPointF(0, 0);
    m_invincibilityTimer = 0;
    m_fireballCooldown = 0;
    updateAnimationState();
}

void Player::setState(PlayerState state)
{
    if (m_state == state) return;

    PlayerState oldState = m_state;
    m_state = state;

    // Handle state transitions
    if (oldState == PlayerState::DYING && state != PlayerState::DYING) {
        // Coming back to life
        m_isAlive = true;
    } else if (state == PlayerState::DYING) {
        // Just died
        m_isAlive = false;
        emit died();
    }

    // Adjust collision boxes based on new size
    setupCollisionBoxes();

    // Update animation to match new state
    updateAnimationState();

    emit stateChanged(state);
}

void Player::setMovementState(MovementState state)
{
    if (m_movementState == state) return;

    m_movementState = state;

    // Update movement flags
    m_isJumping = (state == MovementState::JUMPING);
    m_isFalling = (state == MovementState::FALLING);
    m_isRunning = (state == MovementState::RUNNING);
    m_isCrouching = (state == MovementState::CROUCHING);

    // Update animation
    updateAnimationState();
}

void Player::updateAnimationState()
{
    QString animationKey;
    if (!m_isAlive) {
        animationKey = "dying";
    } else {
        switch (m_movementState) {
        case MovementState::JUMPING:
        case MovementState::FALLING:
            animationKey = "jumping";
            break;
        case MovementState::CROUCHING:
            animationKey = "crouching";
            break;
        case MovementState::RUNNING:
            animationKey = "running";
            break;
        case MovementState::STANDING:
        default:
            animationKey = "idle";
            break;
        }
    }
    // Get animation set for current state
    QHash<QString, QString> currentAnimations = m_animationSets.value(m_state);
    // Play the appropriate animation
    if (currentAnimations.contains(animationKey)) {
        setCurrentAnimation(currentAnimations.value(animationKey));
    }
}

void Player::setCurrentAnimation(const QString &animationKey)
{
    if (m_animations.contains(animationKey)) {
        // Stop current animation
        if (m_currentAnimation) {
            m_currentAnimation->stopAnimation();
        }

        // Set and start new animation
        m_currentAnimation = m_animations.value(animationKey);
        m_currentAnimation->startAnimation();
    }
}

void Player::loadAnimations()
{
    // This method would load all animation sprites and create Animation objects
    // For each player state (small, super, fire) and movement state (idle, running, etc.)

    // Example (pseudo-code):
    // m_animations["small_idle"] = new Animation(":/sprites/small_idle.png", frameWidth, frameHeight, frameCount, frameDelay);
    // ... Load all animations...

    // Setup animation mappings
    QHash<QString, QString> smallAnimations;
    smallAnimations["idle"] = "small_idle";
    smallAnimations["running"] = "small_running";
    smallAnimations["jumping"] = "small_jumping";
    smallAnimations["crouching"] = "small_crouching";
    smallAnimations["dying"] = "small_dying";
    m_animationSets[PlayerState::SMALL] = smallAnimations;

    QHash<QString, QString> superAnimations;
    superAnimations["idle"] = "super_idle";
    superAnimations["running"] = "super_running";
    superAnimations["jumping"] = "super_jumping";
    superAnimations["crouching"] = "super_crouching";
    superAnimations["dying"] = "super_dying";
    m_animationSets[PlayerState::SUPER] = superAnimations;

    QHash<QString, QString> fireAnimations;
    fireAnimations["idle"] = "fire_idle";
    fireAnimations["running"] = "fire_running";
    fireAnimations["jumping"] = "fire_jumping";
    fireAnimations["crouching"] = "fire_crouching";
    fireAnimations["dying"] = "fire_dying";
    m_animationSets[PlayerState::FIRE] = fireAnimations;
}

void Player::setupCollisionBoxes()
{
    // Set up collision boxes based on current state
    float width = 32.0f;  // Base width
    float height = (m_state == PlayerState::SMALL) ? 32.0f : 64.0f;  // Height depends on state

    // Main hit box covers the full body
    m_hitBox = QRectF(-width/2, -height, width, height);

    // Feet box for ground collision
    m_feetBox = QRectF(-width/2, -5, width, 5);

    // Head box for ceiling collision
    m_headBox = QRectF(-width/2, -height, width, 5);

    // Side boxes for wall collision
    m_leftBox = QRectF(-width/2, -height + 5, 5, height - 10);
    m_rightBox = QRectF(width/2 - 5, -height + 5, 5, height - 10);
}

void Player::collectPowerUp(PowerUp *powerUp)
{
    // Handle power-up collection
    switch (powerUp->type()) {
    case 0:  // Mushroom
        if (m_state == PlayerState::SMALL) {
            setState(PlayerState::SUPER);
        }
        break;
    case 1:  // Fire Flower
        setState(PlayerState::FIRE);
        break;
    case 2:  // Star
        setInvincible(true);
        m_invincibilityTimer = 600; // 10 seconds at 60fps
        break;
        // Add more power-up types as needed
    }
}

void Player::fireProjectile()
{
    if (m_state != PlayerState::FIRE || m_fireballCooldown > 0) return;

    // Calculate projectile spawn position
    QPointF spawnPos = pos();
    if (m_isFacingRight) {
        spawnPos.setX(spawnPos.x() + 20);
    } else {
        spawnPos.setX(spawnPos.x() - 20);
    }

    // Fire the projectile
    emit projectileFired(spawnPos, m_isFacingRight);

    // Set cooldown
    m_fireballCooldown = 30; // Half second at 60fps
}

void Player::takeDamage()
{
    if (m_isInvincible || !m_isAlive) return;

    emit tookDamage();

    // Downgrade player state
    switch (m_state) {
    case PlayerState::FIRE:
        setState(PlayerState::SUPER);
        break;
    case PlayerState::SUPER:
        setState(PlayerState::SMALL);
        break;
    case PlayerState::SMALL:
        die();
        break;
    default:
        break;
    }

    // Brief invincibility after taking damage
    setInvincible(true);
    m_invincibilityTimer = 120; // 2 seconds at 60fps
}

void Player::die()
{
    if (!m_isAlive) return;

    setState(PlayerState::DYING);
    m_velocity = QPointF(0, -10); // Initial upward velocity for death animation
    updateAnimationState();
}

void Player::setInvincible(bool invincible)
{
    m_isInvincible = invincible;

    // Visual indication of invincibility could go here
    // For example, making the player blink
}

void Player::debugInfo() const
{
    qDebug() << "Player State:" << static_cast<int>(m_state);
    qDebug() << "Movement State:" << static_cast<int>(m_movementState);
    qDebug() << "Position:" << pos();
    qDebug() << "Velocity:" << m_velocity;
    qDebug() << "Acceleration:" << m_acceleration;
    qDebug() << "Is Alive:" << m_isAlive;
    qDebug() << "Is Jumping:" << m_isJumping;
    qDebug() << "Is Falling:" << m_isFalling;
    qDebug() << "Can Double Jump:" << m_canDoubleJump;
    qDebug() << "Has Double Jumped:" << m_hasDoubleJumped;
    qDebug() << "Can Dash:" << m_canDash;
    qDebug() << "Dash Cooldown:" << m_dashCooldown;
}
