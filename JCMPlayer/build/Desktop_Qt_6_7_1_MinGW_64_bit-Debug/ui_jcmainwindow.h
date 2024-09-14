/********************************************************************************
** Form generated from reading UI file 'jcmainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JCMAINWINDOW_H
#define UI_JCMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_JCMainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *OpenVideoFile;
    QLineEdit *url;
    QPushButton *openUrlFile;
    QFrame *my_view;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *JCMainWindow)
    {
        if (JCMainWindow->objectName().isEmpty())
            JCMainWindow->setObjectName("JCMainWindow");
        JCMainWindow->resize(986, 606);
        JCMainWindow->setMinimumSize(QSize(300, 150));
        centralwidget = new QWidget(JCMainWindow);
        centralwidget->setObjectName("centralwidget");
        OpenVideoFile = new QPushButton(centralwidget);
        OpenVideoFile->setObjectName("OpenVideoFile");
        OpenVideoFile->setEnabled(true);
        OpenVideoFile->setGeometry(QRect(0, 20, 131, 31));
        url = new QLineEdit(centralwidget);
        url->setObjectName("url");
        url->setGeometry(QRect(0, 70, 281, 25));
        openUrlFile = new QPushButton(centralwidget);
        openUrlFile->setObjectName("openUrlFile");
        openUrlFile->setGeometry(QRect(180, 20, 93, 28));
        my_view = new QFrame(centralwidget);
        my_view->setObjectName("my_view");
        my_view->setGeometry(QRect(290, 10, 681, 521));
        my_view->setFrameShape(QFrame::StyledPanel);
        my_view->setFrameShadow(QFrame::Raised);
        JCMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(JCMainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 986, 25));
        JCMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(JCMainWindow);
        statusbar->setObjectName("statusbar");
        JCMainWindow->setStatusBar(statusbar);

        retranslateUi(JCMainWindow);

        QMetaObject::connectSlotsByName(JCMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *JCMainWindow)
    {
        JCMainWindow->setWindowTitle(QCoreApplication::translate("JCMainWindow", "JCMainWindow", nullptr));
        OpenVideoFile->setText(QCoreApplication::translate("JCMainWindow", "\346\211\223\345\274\200\350\247\206\351\242\221\346\226\207\344\273\266", nullptr));
        openUrlFile->setText(QCoreApplication::translate("JCMainWindow", "\346\211\223\345\274\200url", nullptr));
    } // retranslateUi

};

namespace Ui {
    class JCMainWindow: public Ui_JCMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JCMAINWINDOW_H
