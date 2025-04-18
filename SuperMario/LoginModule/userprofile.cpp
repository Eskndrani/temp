#include "userprofile.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

UserProfile::UserProfile(QObject *parent)
    : QObject(parent)
{
}

UserProfile::~UserProfile()
{
}

void UserProfile::loadProfile(QString username)
{
    m_username = username;
    
    QFile file("profiles/" + username + ".json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        
        m_preferences = obj["preferences"].toObject().toVariantMap();
        m_gameProgress = obj["gameProgress"].toObject().toVariantMap();
        m_lastLogin = obj["lastLogin"].toString();
    } else {
        m_preferences.clear();
        m_gameProgress.clear();
        m_lastLogin = "2025-04-18 14:29:34";
    }
}

bool UserProfile::saveProfile()
{
    if (!validateProfileData()) {
        return false;
    }
    
    QDir profileDir("profiles");
    if (!profileDir.exists()) {
        QDir().mkdir("profiles");
    }
    
    QFile file("profiles/" + m_username + ".json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["username"] = m_username;
        obj["preferences"] = QJsonObject::fromVariantMap(m_preferences);
        obj["gameProgress"] = QJsonObject::fromVariantMap(m_gameProgress);
        obj["lastLogin"] = "2025-04-18 14:29:34";
        
        QJsonDocument doc(obj);
        file.write(doc.toJson());
        file.close();
        return true;
    }
    
    return false;
}

QString UserProfile::getUserName() const
{
    return m_username;
}

void UserProfile::setUserName(QString username)
{
    m_username = username;
}

QVariantMap UserProfile::getPreferences() const
{
    return m_preferences;
}

void UserProfile::setPreferences(QVariantMap preferences)
{
    m_preferences = preferences;
}

bool UserProfile::validateProfileData() const
{
    return !m_username.isEmpty();
}