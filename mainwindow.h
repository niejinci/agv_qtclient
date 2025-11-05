#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "client.h"
#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_start_clicked();
    void connect_callback(bool success);

    void on_pushButton_close_clicked();

    void on_pushButton_switch_clicked();

    void on_pushButton_send1_clicked();

    void common_callback(const std::string& reply, const std::string& source="");

    void on_pushButton_uploadfile_clicked();

    void on_pushButton_stop_hb_clicked();

    void on_pushButton_restart_hb_clicked();

    void on_pushButton_agv_positon_clicked();

    void on_pushButton_set_manual_clicked();

    void on_pushButton_set_auto_clicked();

    void on_pushButton_get_model_clicked();

    void on_pushButton_teaching_clicked();

    void on_pushButton_point_cloud_clicked();

    std::vector<std::string> parse_ips(const std::string& input);

    void on_pushButton_clicked();
    void display_execution_queue(const std::string& reply);

    void on_pushButton_get_task_queue_clicked();

private:
    Ui::MainWindow *ui;
    asio::io_context io_context_;
    std::shared_ptr<qclcpp::Client> client_;
    // std::unique_ptr<asio::io_context::work> work_; // 添加一个成员变量来持有 work 对象
};
#endif // MAINWINDOW_H
