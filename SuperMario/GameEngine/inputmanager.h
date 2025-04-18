#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QObject>
#include <QKeyEvent>
#include <QSet>

class InputManager : public QObject
{
    Q_OBJECT
    
public:
    InputManager(QObject *parent = nullptr);
    ~InputManager();
    
    void processKeyPress(QKeyEvent* event);
    void processKeyRelease(QKeyEvent* event);
    void processMouseInput(QMouseEvent* event);
    bool isKeyPressed(int key) const;
    
private:
    void mapControlsToActions();
    void debounceInput();
    
    QSet<int> m_pressedKeys;
    QMap<int, QString> m_keyToActionMap;
};

#endif