QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SuperMario
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    GameEngine/spritesheet.cpp \
    main.cpp \
    mainwindow.cpp \
    GameEngine/myscene.cpp \
    GameEngine/sprite.cpp \
    GameEngine/animation.cpp \
    GameEngine/effect.cpp \
    GameEngine/player.cpp \
    GameEngine/enemy.cpp \
    GameEngine/goomba.cpp \
    GameEngine/turtle.cpp \
    GameEngine/powerup.cpp \
    GameEngine/mushroom.cpp \
    GameEngine/flower.cpp \
    GameEngine/score.cpp \
    GameEngine/timer.cpp \
    GameEngine/soundmanager.cpp \
    GameEngine/physicsengine.cpp \
    GameEngine/inputmanager.cpp \
    LoginModule/loginpage.cpp \
    LoginModule/userprofile.cpp \
    LoginModule/usermanager.cpp \
    GameEngine/spritesheetmanager.cpp

HEADERS += \
    GameEngine/spritesheet.h \
    mainwindow.h \
    GameEngine/myscene.h \
    GameEngine/sprite.h \
    GameEngine/animation.h \
    GameEngine/effect.h \
    GameEngine/player.h \
    GameEngine/enemy.h \
    GameEngine/goomba.h \
    GameEngine/turtle.h \
    GameEngine/powerup.h \
    GameEngine/mushroom.h \
    GameEngine/flower.h \
    GameEngine/score.h \
    GameEngine/timer.h \
    GameEngine/soundmanager.h \
    GameEngine/physicsengine.h \
    GameEngine/inputmanager.h \
    LoginModule/loginpage.h \
    LoginModule/userprofile.h \
    LoginModule/usermanager.h \
    GameEngine/spritesheetmanager.h

FORMS += \
    mainwindow.ui \
    LoginModule/loginpage.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc \
    sounds.qrc
