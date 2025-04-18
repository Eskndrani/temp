#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QMap>
#include "userprofile.h"

class UserManager : public QObject
{
    Q_OBJECT
    
public:
    explicit UserManager(QObject *parent = nullptr);
    ~UserManager();
    
    bool registerNewUser(QString username, QString password, QString email);
    bool login(QString username, QString password);
    void logout();
    UserProfile* getCurrentUser();
    bool updateUserProfile(UserProfile* profile);
    
private:
    bool validateCredentials(QString username, QString password);
    QString encryptPassword(QString password);
    void storeSessionData();
    
    QMap<QString, QString> m_users;
    UserProfile* m_currentUser;
    QString m_currentTimeStamp;
};

#endif