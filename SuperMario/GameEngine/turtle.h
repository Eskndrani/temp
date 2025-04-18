#ifndef TURTLE_H
#define TURTLE_H

#include "enemy.h"
#include <QTimer>

class Turtle : public Enemy
{
    Q_OBJECT

public:
    explicit Turtle(QGraphicsItem *parent = nullptr);
    ~Turtle() override;

    // Override base class methods
    void move() override;
    void takeDamage() override;
    void updateMovementPattern() override;

    // Turtle-specific methods
    void enterShell();
    void spinShell();
    void withdrawIntoShell();

private:
    bool m_inShell;
    bool m_isSpinning;
    int m_shellTimer;
};

#endif // TURTLE_H
