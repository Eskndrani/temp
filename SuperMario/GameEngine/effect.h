#ifndef EFFECT_H
#define EFFECT_H

#include <QObject>
#include <QGraphicsItem>
#include <QString>

class Effect : public QObject, public QGraphicsItem
{
    Q_OBJECT
    
public:
    Effect(QGraphicsItem *parent = nullptr);
    ~Effect();
    
    void playEffect();
    void setIntensity(int intensity);
    void fadeIn();
    void fadeOut();
    void setEffectType(const QString& type);
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
private:
    void initializeEffect();
    void updateEffectParameters();
    
    QString m_effectType;
    int m_intensity;
    bool m_fadeIn;
    bool m_fadeOut;
    int m_duration;
    int m_elapsed;
    bool m_isPlaying;
};

#endif