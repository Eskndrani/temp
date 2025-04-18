#include "inputmanager.h"
#include <QDebug>

InputManager::InputManager(QObject *parent)
    : QObject(parent)
{
    mapControlsToActions();
}

InputManager::~InputManager()
{
    // No dynamic allocations to clean up
}

void InputManager::processKeyPress(QKeyEvent* event)
{
    if (!event) {
        qWarning() << "Null key event received in processKeyPress";
        return;
    }
    
    int key = event->key();
    m_pressedKeys.insert(key);
    
    // Emit signal for the corresponding action if mapped
    if (m_keyToActionMap.contains(key)) {
        QString action = m_keyToActionMap[key];
        emit actionTriggered(action, true);
        
        // Debug output
        qDebug() << "Action triggered:" << action;
    }
}

void InputManager::processKeyRelease(QKeyEvent* event)
{
    if (!event) {
        qWarning() << "Null key event received in processKeyRelease";
        return;
    }
    
    int key = event->key();
    m_pressedKeys.remove(key);
    
    // Emit signal for the corresponding action if mapped
    if (m_keyToActionMap.contains(key)) {
        QString action = m_keyToActionMap[key];
        emit actionTriggered(action, false);
    }
}

void InputManager::processMouseInput(QMouseEvent* event)
{
    if (!event) {
        qWarning() << "Null mouse event received";
        return;
    }
    
    // Store current mouse position
    m_mousePosition = event->pos();
    
    // Process different mouse events
    switch (event->type()) {
        case QEvent::MouseButtonPress:
            emit mouseActionTriggered("click", true, m_mousePosition);
            break;
        case QEvent::MouseButtonRelease:
            emit mouseActionTriggered("click", false, m_mousePosition);
            break;
        case QEvent::MouseMove:
            emit mouseActionTriggered("move", true, m_mousePosition);
            break;
        default:
            break;
    }
}

bool InputManager::isKeyPressed(int key) const
{
    return m_pressedKeys.contains(key);
}

bool InputManager::isActionActive(const QString& action) const
{
    // Check if any key mapped to this action is pressed
    for (auto it = m_keyToActionMap.begin(); it != m_keyToActionMap.end(); ++it) {
        if (it.value() == action && m_pressedKeys.contains(it.key())) {
            return true;
        }
    }
    return false;
}

QString InputManager::getActionForKey(int key) const
{
    return m_keyToActionMap.value(key, "none");
}

void InputManager::mapControlsToActions()
{
    m_keyToActionMap[Qt::Key_Left] = "moveLeft";
    m_keyToActionMap[Qt::Key_Right] = "moveRight";
    m_keyToActionMap[Qt::Key_Up] = "jump";
    m_keyToActionMap[Qt::Key_Space] = "jump";
    m_keyToActionMap[Qt::Key_F] = "fireball";
    m_keyToActionMap[Qt::Key_Down] = "duck";
    m_keyToActionMap[Qt::Key_P] = "pause";
    m_keyToActionMap[Qt::Key_R] = "reset";
    m_keyToActionMap[Qt::Key_Escape] = "quit";
}

void InputManager::rebindKey(int oldKey, int newKey)
{
    if (m_keyToActionMap.contains(oldKey)) {
        QString action = m_keyToActionMap[oldKey];
        m_keyToActionMap.remove(oldKey);
        m_keyToActionMap[newKey] = action;
        
        qDebug() << "Rebound key" << oldKey << "to" << newKey << "for action" << action;
    }
}

void InputManager::debounceInput()
{
    // Implementation of input debouncing
    // This prevents rapid firing of the same input
    
    // The current implementation uses a simple approach:
    // We clear and then recreate the pressed keys set
    // which effectively debounces all inputs
    
    m_pressedKeys.clear();
    
    // Emit signals to notify that all actions are released
    for (auto it = m_keyToActionMap.begin(); it != m_keyToActionMap.end(); ++it) {
        emit actionTriggered(it.value(), false);
    }
    
    qDebug() << "Input debounced - all keys cleared";
}

void InputManager::clearAllInputs()
{
    m_pressedKeys.clear();
}

QSet<int> InputManager::getPressedKeys() const
{
    return m_pressedKeys;
}

QMap<int, QString> InputManager::getKeyMappings() const
{
    return m_keyToActionMap;
}
