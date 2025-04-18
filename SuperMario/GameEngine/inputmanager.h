#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSet>
#include <QMap>
#include <QPoint>

class InputManager : public QObject
{
    Q_OBJECT
public:
    explicit InputManager(QObject *parent = nullptr);
    ~InputManager();
    
    void processKeyPress(QKeyEvent* event);
    void processKeyRelease(QKeyEvent* event);
    void processMouseInput(QMouseEvent* event);
    
    bool isKeyPressed(int key) const;
    bool isActionActive(const QString& action) const;
    QString getActionForKey(int key) const;
    
    void rebindKey(int oldKey, int newKey);
    void debounceInput();
    void clearAllInputs();
    
    QSet<int> getPressedKeys() const;
    QMap<int, QString> getKeyMappings() const;
    
signals:
    void actionTriggered(const QString& action, bool pressed);
    void mouseActionTriggered(const QString& action, bool pressed, const QPoint& position);
    
private:
    void mapControlsToActions();
    
    QSet<int> m_pressedKeys;
    QMap<int, QString> m_keyToActionMap;
    QPoint m_mousePosition;
};

#endif // INPUTMANAGER_H
