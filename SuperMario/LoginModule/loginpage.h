#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include "usermanager.h"

namespace Ui {
class LoginPage;
}

class LoginPage : public QWidget
{
    Q_OBJECT
    
public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();
    
    void display();
    bool acceptCredentials(QString username, QString password);
    void showError(QString message);
    void logout();
    
private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    
private:
    void initUI();
    void clearInputFields();
    
    Ui::LoginPage *ui;
    UserManager *m_userManager;
    QString currentTimeStamp;
};

#endif