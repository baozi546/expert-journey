#include "jcmainwindow.h"
#include "ui_jcmainwindow.h"

#include <QFileDialog>
#include <QDir>


JCMainWindow::JCMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JCMainWindow)
{
    ui->setupUi(this);
    layout = new QVBoxLayout(ui->my_view);
   // layout->addWidget(m_v);
}

JCMainWindow::~JCMainWindow()
{
    releseHandler();
    delete ui;
}

void JCMainWindow::releseHandler()
{
    if(m_pCodecHandler!=nullptr)
    {
        delete m_pCodecHandler;
        m_pCodecHandler=nullptr;
    }

}

void JCMainWindow::setHandler()
{
    m_pCodecHandler = new JCAVCodecHandler(this);
}

void JCMainWindow::on_OpenVideoFile_clicked()
{
    QString path = QFileDialog::getOpenFileName();

    if(path.isEmpty()) return;

    releseHandler();
    setHandler();

    m_pCodecHandler->SetVideoFilePath(path);
   int ret= m_pCodecHandler->InitVideoCodec();
    if(ret!=0)
   {
       qDebug()<<"error handler init";
       return;
   }
    m_pCodecHandler->startMedioProcessThread();
}

void JCMainWindow::on_openUrlFile_clicked()
{
    QString path = ui->url->text();

    if(path.isEmpty()) return;

    releseHandler();
    setHandler();

    m_pCodecHandler->SetVideoFilePath(path);
    int ret= m_pCodecHandler->InitVideoCodec();
    if(ret!=0)
    {
        qDebug()<<"error handler init";
        return;
    }
    m_pCodecHandler->startMedioProcessThread();
}

