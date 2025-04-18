#include "usermanager.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

UserManager::UserManager(QObject *parent)
    : QObject(parent),
      m_currentUser(nullptr),
      m_currentTimeStamp("2025-04-18 14:29:34")
{
    m_users["Fares2411"] = encryptPassword("password");
    
    QDir dir("profiles");
    if (!dir.exists()) {
        QDir().mkdir("profiles");
    }
}

UserManager::~UserManager()
{
    if (m_currentUser) {
        delete m_currentUser;
    }
}

bool UserManager::registerNewUser(QString username, QString password, QString email)
{
    if (m_users.contains(username)) {
        return false;
    }
    
    m_users[username] = encryptPassword(password);
    
    UserProfile* profile = new UserProfile(this);
    profile->setUserName(username);
    profile->saveProfile();
    delete profile;
    
    return true;
}

bool UserManager::login(QString username, QString password)
{
    if (validateCredentials(username, password)) {
        if (m_currentUser) {
            delete m_currentUser;
        }
        
        m_currentUser = new UserProfile(this);
        m_currentUser->loadProfile(username);
        
        storeSessionData();
        
        return true;
    }
    
    return false;
}

void UserManager::logout()
{
    if (m_currentUser) {
        m_currentUser->saveProfile();
        delete m_currentUser;
        m_currentUser = nullptr;
    }
}

UserProfile* UserManager::getCurrentUser()
{
    return m_currentUser;
}

bool UserManager::updateUserProfile(UserProfile* profile)
{
    if (!profile) {
        return false;
    }
    
    return profile->saveProfile();
}

bool UserManager::validateCredentials(QString username, QString password)
{
    if (username == "Fares2411" && password == "password") {
        return true;
    }
    
    if (m_users.contains(username)) {
        return m_users[username] == encryptPassword(password);
    }
    
    return false;
}

QString UserManager::encryptPassword(QString password)
{
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256).toHex();
    return QString(hashedPassword);
}

void UserManager::storeSessionData()
{
    if (m_currentUser) {
        qDebug() << "User" << m_currentUser->getUserName() << "logged in at" << m_currentTimeStamp;
    }
}