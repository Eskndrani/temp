#include "score.h"
#include <QDebug>
#include <QFontDatabase>

Score::Score(QGraphicsItem *parent)
    : QGraphicsTextItem(parent),
      m_score(0)
{
    m_font = QFont("Super Mario 256", 12, QFont::Bold);
    setFont(m_font);
    setDefaultTextColor(Qt::white);
    updateDisplay();
}

Score::~Score()
{
}

void Score::addPoints(int points)
{
    m_score += points;
    updateDisplay();
}

void Score::reset()
{
    m_score = 0;
    updateDisplay();
}

int Score::getTotal() const
{
    return m_score;
}

void Score::render()
{
    updateDisplay();
}

void Score::updateDisplay()
{
    setPlainText("SCORE: " + QString::number(m_score));
}

int Score::calculateBonus() const
{
    return 0;
}