#include "mainwindow.h"
#include "LoginModule/loginpage.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/font/SuperMario256.ttf");
    QFontDatabase::addApplicationFont(":/font/CoinCount2.ttf");
    LoginPage loginPage;
    loginPage.show();
    return a.exec();
}