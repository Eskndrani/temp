#include "loginpage.h"
#include "ui_loginpage.h"
#include "../mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

LoginPage::LoginPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginPage),
    currentTimeStamp("2025-04-18 14:29:34")
{
    ui->setupUi(this);
    
    m_userManager = new UserManager(this);
    
    initUI();
}

LoginPage::~LoginPage()
{
    delete ui;
    delete m_userManager;
}

void LoginPage::display()
{
    show();
}

bool LoginPage::acceptCredentials(QString username, QString password)
{
    return m_userManager->login(username, password);
}

void LoginPage::showError(QString message)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet("color: red;");
}

void LoginPage::logout()
{
    clearInputFields();
    show();
}

void LoginPage::on_loginButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        showError("Please enter both username and password.");
        return;
    }
    
    if (m_userManager->login(username, password)) {
        qDebug() << "User" << username << "logged in at" << currentTimeStamp;
        
        MainWindow *mainWindow = new MainWindow();
        mainWindow->setCurrentUser(username);
        mainWindow->show();
        
        this->hide();
    } else {
        showError("Invalid username or password!");
    }
}

void LoginPage::on_registerButton_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString email = "user@example.com";
    
    if (username.isEmpty() || password.isEmpty()) {
        showError("Please enter both username and password.");
        return;
    }
    
    if (m_userManager->registerNewUser(username, password, email)) {
        QMessageBox::information(this, "Registration Successful", 
                                "User registered successfully. You can now log in.");
        clearInputFields();
    } else {
        showError("Username already exists!");
    }
}

void LoginPage::initUI()
{
    setWindowTitle("Super Mario - Login");
    setFixedSize(400, 300);
    
    ui->logoLabel->setPixmap(QPixmap(":/images/loginlogo.png").scaled(200, 100, Qt::KeepAspectRatio));
    
    ui->statusLabel->clear();
    
    QFont marioFont("Super Mario 256", 10);
    ui->usernameLabel->setFont(marioFont);
    ui->passwordLabel->setFont(marioFont);
    ui->loginButton->setFont(marioFont);
    ui->registerButton->setFont(marioFont);
    
    ui->usernameLineEdit->setPlaceholderText("Default: Fares2411");
    ui->passwordLineEdit->setPlaceholderText("Default: password");
}

void LoginPage::clearInputFields()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->statusLabel->clear();
}