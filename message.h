#include <QMessageBox>
#include <QEventLoop>
 
class ThreadInformationMessageBox : public QObject
{
    Q_OBJECT
 
private:
    const QString m_strTitle;
    const QString m_strMessage;
 
public:
    ThreadInformationMessageBox(const QString &strTitle, const QString &strMessage);
 
    static void show(const QString &strTitle, const QString &strMessage);
 
private:
    void readyShow(void);
 
private slots:
    void onShow(void);
};