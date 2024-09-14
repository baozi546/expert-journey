#ifndef JCMAINWINDOW_H
#define JCMAINWINDOW_H

#include <QMainWindow>
#include "jcavcodechandler.h"
#include "videoopengl.h"
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class JCMainWindow;
}
QT_END_NAMESPACE

class JCMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    JCMainWindow(QWidget *parent = nullptr);
    ~JCMainWindow();

    void releseHandler();
    void setHandler();
private slots:

    void on_OpenVideoFile_clicked();
    void on_openUrlFile_clicked();

private:
    Ui::JCMainWindow *ui;
    JCAVCodecHandler *m_pCodecHandler=nullptr;

    QVBoxLayout *layout=nullptr;
};
#endif // JCMAINWINDOW_H
