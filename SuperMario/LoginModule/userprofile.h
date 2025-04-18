#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDir>

class UserProfile : public QObject
{
    Q_OBJECT

public:
    explicit UserProfile(QObject *parent = nullptr);
    ~UserProfile();

    void loadProfile(QString username);
    bool saveProfile();
    QString getUserName() const;
    void setUserName(QString username);
    QVariantMap getPreferences() const;
    void setPreferences(QVariantMap preferences);

private:
    bool validateProfileData() const;

    QString m_username;
    QVariantMap m_preferences;
    QVariantMap m_gameProgress;
    QString m_lastLogin;
};
#endif
