#include "message.h"
#include <QApplication>
#include <QTimer>

ThreadInformationMessageBox::ThreadInformationMessageBox(const QString &strTitle, const QString &strMessage)
	: m_strTitle(strTitle), 
	  m_strMessage(strMessage)
{
}
 
void ThreadInformationMessageBox::show(const QString &strTitle, const QString &strMessage)
{
    QEventLoop eventLoop;
    auto messageBox = new ThreadInformationMessageBox(strTitle, strMessage);
    connect(messageBox, SIGNAL(destroyed()), &eventLoop, SLOT(quit()));
    messageBox->readyShow();
    eventLoop.exec();
}
 
void ThreadInformationMessageBox::readyShow(void)
{
    this->moveToThread(qApp->thread());
    QTimer::singleShot(0, this, SLOT(onShow()));
}

void ThreadInformationMessageBox::onShow(void)
{
    QMessageBox::information(NULL, m_strTitle, m_strMessage);
    this->deleteLater();
}