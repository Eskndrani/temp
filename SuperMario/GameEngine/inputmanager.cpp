#include "inputmanager.h"
#include <QDebug>

InputManager::InputManager(QObject *parent)
    : QObject(parent)
{
    mapControlsToActions();
}

InputManager::~InputManager()
{
}

void InputManager::processKeyPress(QKeyEvent* event)
{
    if (event) {
        m_pressedKeys.insert(event->key());
    }
}

void InputManager::processKeyRelease(QKeyEvent* event)
{
    if (event) {
        m_pressedKeys.remove(event->key());
    }
}

void InputManager::processMouseInput(QMouseEvent* event)
{
}

bool InputManager::isKeyPressed(int key) const
{
    return m_pressedKeys.contains(key);
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

void InputManager::debounceInput()
{
}