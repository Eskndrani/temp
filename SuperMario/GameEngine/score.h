#ifndef SCORE_H
#define SCORE_H

#include <QGraphicsTextItem>
#include <QFont>

class Score : public QGraphicsTextItem
{
public:
    Score(QGraphicsItem *parent = nullptr);
    ~Score();
    
    void addPoints(int points);
    void reset();
    int getTotal() const;
    void render();
    
private:
    void updateDisplay();
    int calculateBonus() const;
    
    int m_score;
    QFont m_font;
};

#endif