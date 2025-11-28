#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <thread>
#include <iostream>
#include <QWidget>
#include <memory>
#include "message.h"
#include <QAbstractItemView>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QCompleter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client_(qclcpp::Client::create(io_context_))
    // , work_(std::make_unique<asio::io_context::work>(io_context_)) // 初始化 work 对象
{
    ui->setupUi(this);
    ui->comboBox->setEditable(true); // 设为可编辑

    // 创建一个 QCompleter
    QCompleter *completer = new QCompleter(ui->comboBox->model(), this);

    // 设置过滤模式，`Qt::MatchContains` 表示只要包含输入文本就算匹配
    completer->setFilterMode(Qt::MatchContains);

    // 设置大小写不敏感
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    // 将 completer 应用到 comboBox
    ui->comboBox->setCompleter(completer);

    ui->comboBox->addItem("RELOCATION");
    ui->comboBox->addItem("CONFIRM_RELOCATION");
    ui->comboBox->addItem("TRANSLATION");
    ui->comboBox->addItem("ROTATION");
    ui->comboBox->addItem("LIFTING");
    ui->comboBox->addItem("SET_SPEED");
    ui->comboBox->addItem("SET_MOTOR_CONTROL");
    ui->comboBox->addItem("SET_PALLET_ROTATION");
    ui->comboBox->addItem("SET_HINT");
    ui->comboBox->addItem("PATH_NAVIGATION");
    ui->comboBox->addItem("REMOTE_CONTROL");
    ui->comboBox->addItem("PULL_MAP");
    ui->comboBox->addItem("PUSH_MAP");
    ui->comboBox->addItem("QUERY_TASK");
    ui->comboBox->addItem("START_TASK");
    ui->comboBox->addItem("CANCEL_TASK");
    ui->comboBox->addItem("PAUSE_TASK");
    ui->comboBox->addItem("RESUME_TASK");
    ui->comboBox->addItem("HEART_BEAT");
    ui->comboBox->addItem("GET_AGV_POSITION");
    ui->comboBox->addItem("CANCEL_GET_AGV_POSITION");
    ui->comboBox->addItem("GET_POINT_CLOUD");
    ui->comboBox->addItem("CANCEL_GET_POINT_CLOUD");
    ui->comboBox->addItem("SET_OPERATING_MODE");
    ui->comboBox->addItem("GET_OPERATING_MODE");
    ui->comboBox->addItem("GET_MAP_LIST");
    ui->comboBox->addItem("CHECK_CONNECTIVITY");
    ui->comboBox->addItem("GET_LOG_LIST");
    ui->comboBox->addItem("GET_LOG_FILE");
    ui->comboBox->addItem("REBOOT_OR_POWEROFF");
    ui->comboBox->addItem("SET_DATE_TIME");
    ui->comboBox->addItem("GET_DATE_TIME");
    ui->comboBox->addItem("TERMINAL_COMMAND");
    ui->comboBox->addItem("BUILD_MAPPING");
    ui->comboBox->addItem("OTA_UPGRADE");
    ui->comboBox->addItem("LOCALIZATION_QUALITY");
    ui->comboBox->addItem("GET_VELOCITY");
    ui->comboBox->addItem("GET_MCU2PC");
    ui->comboBox->addItem("GET_RUN_TASK");
    ui->comboBox->addItem("SET_LOG_LEVEL");
    ui->comboBox->addItem("GET_CLIENTS");
    ui->comboBox->addItem("EMERGENCY_STOP");
    ui->comboBox->addItem("GET_SYSINFO");
    ui->comboBox->addItem("GET_SYSINFO_PERIODIC");
    ui->comboBox->addItem("CANCEL_GET_SYSINFO");
    ui->comboBox->addItem("PALLET_ROTATION");
    ui->comboBox->addItem("GET_MODEL_FILE");
    ui->comboBox->addItem("GET_CAMERA_POINT_CLOUD");
    ui->comboBox->addItem("GET_CAMERA_POINT_CLOUD_SINGLE");
    ui->comboBox->addItem("CANCEL_GET_CAMERA_POINT_CLOUD");
    ui->comboBox->addItem("UPLOAD_FILE");
    ui->comboBox->addItem("GET_CAMERA_VIDEO_LIST");
    ui->comboBox->addItem("GET_CAMERA_VIDEO");
    ui->comboBox->addItem("GET_ERRORS");
    ui->comboBox->addItem("STOP_CHARGING");
    ui->comboBox->addItem("SET_DO");
    ui->comboBox->addItem("SET_DI");
    ui->comboBox->addItem("CLEAR_ERRORS");
    ui->comboBox->addItem("GET_PROCESSES_INFO");
    ui->comboBox->addItem("CHECK_FD_KEEP_ALIVE");
    ui->comboBox->addItem("GET_PLC_DIGITAL_IO");
    ui->comboBox->addItem("GET_WIFI_LIST");
    ui->comboBox->addItem("SET_WIFI_CONFIG");
    ui->comboBox->addItem("GET_NETWORK_INTERFACE");
    ui->comboBox->addItem("GET_EXECUTION_QUEUE");
    ui->comboBox->addItem("GET_QR_CAMERA_DATA");
    ui->comboBox->addItem("CANCEL_GET_QR_CAMERA_DATA");
    ui->comboBox->addItem("GET_SCAN2POINTCLOUD");
    ui->comboBox->addItem("CANCEL_GET_SCAN2POINTCLOUD");
    ui->comboBox->addItem("GET_OBST_POLYGON");
    ui->comboBox->addItem("CANCEL_GET_OBST_POLYGON");
    ui->comboBox->addItem("GET_OBST_PCL");
    ui->comboBox->addItem("CANCEL_GET_OBST_PCL");
    ui->comboBox->addItem("GET_MODEL_POLYGON");
    ui->comboBox->addItem("CANCEL_GET_MODEL_POLYGON");
    ui->comboBox->addItem("ENABLE_ROBOT");
    ui->comboBox->addItem("SET_COORDINATE_SYSTEM");
    ui->comboBox->addItem("SET_TOOL");
    ui->comboBox->addItem("JOG_SINGLE_AXIS");
    ui->comboBox->addItem("ONE_CLICK_HOMING");
    ui->comboBox->addItem("GET_TEACHIN_FILE_LIST");
    ui->comboBox->addItem("GET_TEACHIN_FILE");
    ui->comboBox->addItem("PUSH_TEACHIN_POINTS");
    ui->comboBox->addItem("GET_TEACHIN_POINTS");
    ui->comboBox->addItem("GET_ROBOT_STATE");
    ui->comboBox->addItem("CANCEL_GET_ROBOT_STATE");
    ui->comboBox->addItem("GET_CHASSIS_INFO");
    ui->comboBox->addItem("DELETE_TEACHIN_FILES");
    ui->comboBox->addItem("GET_BROKER_CONNECTION");
    ui->comboBox->addItem("SET_RCS_ONLINE");
    ui->comboBox->addItem("SOFT_RESET");
    ui->comboBox->addItem("GET_RACK_NUMBER");
    QAbstractItemView *view = ui->comboBox->view();
    // view->setMinimumWidth(175);
    // view->setMaximumWidth(175);

    std::thread t([this]() {
        // 创建一个 work 对象，用于保持 io_context 运行
        asio::io_context::work work(this->io_context_);
        this->io_context_.run();
        });
    t.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connect_callback(bool success)
{
    if (success) {
        ThreadInformationMessageBox::show("connect", "success");
        //QMessageBox::information(this, "connect", "success");
        std::cerr << "connect success\n";
    } else {
        std::cerr << "connect failed\n";
        ThreadInformationMessageBox::show("connect", "fail");
//        QMessageBox::information(this, "connect", "fail");
    }
}

//读取文本框输入的ip和port，connect 到服务器
void MainWindow::on_pushButton_start_clicked()
{
   // client_->disconnect(qclcpp::Client::default_disconnect_callback);

    // 读取 lineEdit_ip 和 lineEdit_port 的输入并赋值给两个 QString 变量
    QString ip = ui->lineEdit_ip->text();
    QString portStr = ui->lineEdit_port->text();

    // 现在你可以使用 ip 和 port 进行网络连接等操作了
    qDebug()<< "ip="<< ip << ", port=" << portStr << "\n";
    if (ip.isEmpty() || portStr.isEmpty()) {
        QMessageBox::warning(this, "connect", "ip or port is empty");
        return;
    }
    client_->connect(ip.toStdString(), portStr.toStdString(), [this](bool success) {
        this->connect_callback(success);
    });
 //   qDebug() << "connect to server\n";
}

void MainWindow::on_pushButton_close_clicked()
{
    //断开跟服务器的连接
    client_->disconnect(qclcpp::Client::default_disconnect_callback);
    // std::cerr <<"disconnect from server\n";
}

void MainWindow::on_pushButton_switch_clicked()
{
    //读取文本框输入，connect 到服务器
    client_->disconnect(qclcpp::Client::default_disconnect_callback);
    on_pushButton_start_clicked();
}

void MainWindow::common_callback(const std::string& reply, const std::string& source)
{
    // std::cerr << "[" << __func__ << ":" << __LINE__ << "] " << reply << "\n";
    //plainTextEdit_response
    // ui->plainTextEdit_response->setPlainText(QString::fromStdString(reply));

    if (source == "upload_file") {
        QMetaObject::invokeMethod(ui->plainTextEdit_uploadfile, "setPlainText", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromUtf8(reply.c_str())));
    } else {
        QMetaObject::invokeMethod(ui->plainTextEdit_response, "setPlainText", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(reply)));
    }

}

std::vector<std::string> MainWindow::parse_ips(const std::string& input)
{
    std::vector<std::string> ips;
    std::istringstream stream(input);
    std::string ip;

    // 使用getline函数按行读取，'\n'作为分隔符
    while (std::getline(stream, ip, '\n')) {
        if (!ip.empty()) { // 忽略空字符串
            ips.push_back(ip);
        }
    }

    return ips;
}

void MainWindow::on_pushButton_send1_clicked()
{
    //读取文本框输入，发送到服务器，把响应数据写入到另一个文本框
    QString strRequestName = ui->comboBox->currentText();
    QString strText = ui->plainTextEdit_send1->toPlainText();
    //QMessageBox::information(this, "server", strText);
    qDebug() << "request name=" << strRequestName;
    qDebug() << "strText     =" << strText;
    std::string strRequestNameStr = strRequestName.toStdString();
    if ("PULL_MAP" == strRequestName && strText.isEmpty()) {
        QMessageBox::warning(this, "send", "file_name is empty");
        return;
    } else if ("PULL_MAP" == strRequestName) {
        bool ret = client_->pull_map(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        if (!ret) {
            std::cout << "pull map failed\n";
        }
        return;
    } else if ("PUSH_MAP" == strRequestName) {
        bool ret = client_->push_map(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        if (!ret) {
            std::cout << "push map failed\n";
        }
        return;
    } else if ("GET_AGV_POSITION" == strRequestName) {
        client_->get_agv_position([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_GET_AGV_POSITION" == strRequestName) {
        client_->cancel_get_agv_position();
        return;
    } else if ("GET_POINT_CLOUD" == strRequestName) {   //获取点云数据
        client_->get_point_cloud([this](const std::string& reply) {
            // this->common_callback(reply);
            std::cerr << std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000'000 << "\n";
        });
        return;
    } else if ("CANCEL_GET_POINT_CLOUD" == strRequestName) {    //取消获取点云数据
        client_->cancel_get_point_cloud();
        return;
    } else if ("CHECK_CONNECTIVITY" == strRequestName) {    //检测网络联通性
        // 解析 strText, 构造 std::vector<std::string>
        //strText     = "192.168.192.124\n192.168.1.101\n192.168.10.101"
        std::vector<std::string> ip_list = parse_ips(strText.toStdString());
        client_->check_connectivity(ip_list, [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_LOG_LIST" == strRequestName) {    //获取日志文件列表
        client_->get_log_list( [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_LOG_FILE" == strRequestName) {    //获取日志文件
        client_->get_log_file(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("REBOOT_OR_POWEROFF" == strRequestName) {    //重启or关机设备
        client_->reboot_or_poweroff(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_DATE_TIME" == strRequestName) {    //设置系统时间
        client_->set_datetime(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_DATE_TIME" == strRequestName) {    //获取系统时间
        client_->get_datetime([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("TERMINAL_COMMAND" == strRequestName) {    //获取系统时间
        client_->terminal_command(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("BUILD_MAPPING" == strRequestName) {    //建图
        client_->build_map([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("OTA_UPGRADE" == strRequestName) {    //升级
        client_->ota_upgrade(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_VELOCITY" == strRequestName) {    //获取速度
        client_->get_velocity([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_MCU2PC" == strRequestName) {    //获取电池和电机状态
        client_->get_mcu2pc([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("PAUSE_TASK" == strRequestName) {    //暂定任务
        client_->pause_task([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("RESUME_TASK" == strRequestName) {    //恢复任务
        client_->resume_task([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_TASK" == strRequestName) {    //取消任务
        client_->cancel_task(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("START_TASK" == strRequestName) {    //启动任务
        client_->start_task(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("ROTATION" == strRequestName) {    //小车旋转
        client_->rotation(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("TRANSLATION" == strRequestName) {    //平动
        client_->translation(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("RELOCATION" == strRequestName) {    //重定位
        client_->relocation(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }   else if ("GET_RUN_TASK" == strRequestName) {    //获取当前任务
        client_->get_run_task([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("GET_MAP_LIST" == strRequestName) {    //获取地图列表
        client_->get_map_list([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("REMOTE_CONTROL" == strRequestName) {    //遥控
        client_->remote_control(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("LIFTING" == strRequestName) {    //顶升
        client_->lifting(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }   else if ("SET_LOG_LEVEL" == strRequestName) {    //设置日志级别
        client_->set_log_level(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_CLIENTS" == strRequestName) {    //获取客户端信息
        client_->get_clients([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("EMERGENCY_STOP" == strRequestName) {    //急停
        client_->emergency_stop(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_SYSINFO" == strRequestName) {    //获取系统信息
        client_->get_sysinfo([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("GET_SYSINFO_PERIODIC" == strRequestName) {    //获取系统信息(定期)
        client_->get_sysinfo_periodic([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("CANCEL_GET_SYSINFO" == strRequestName) {    //取消获取系统信息(定期)
        client_->cancel_get_sysinfo();
        return;
    } else if ("PALLET_ROTATION" == strRequestName) {    //托盘旋转
        client_->pallet_rotation(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_MODEL_FILE" == strRequestName) {    //获取模型文件
        client_->get_model_file(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_CAMERA_POINT_CLOUD" == strRequestName) {   //获取3d摄像头点云数据
        client_->get_3dcamera_pointcloud([this](const std::string& reply) {
            //this->common_callback(reply);
            std::cerr << "get 3d camera pc, reply.size()=" << reply.size() << "\n";
        });
        return;
    }  else if ("CANCEL_GET_CAMERA_POINT_CLOUD" == strRequestName) {   //取消获取3d摄像头点云数据
        client_->cancel_get_3dcamera_pointcloud();
        return;
    }  else if ("GET_CAMERA_POINT_CLOUD_SINGLE" == strRequestName) {   //获取3d摄像头点云数据
        client_->get_3dcamera_pointcloud_single(strText.toStdString(), [this](const std::string& reply) {
            // std::string msg = "[single] get 3d camera pc, reply.size()=" + std::to_string(reply.size());
            // this->common_callback(msg);
            std::cerr << "[single] get 3d camera pc, reply.size()=" << reply.size() << "\n";
            if (reply.size() < 1000) {
                std::cerr << reply << "\n";
            }
        });
        return;
    }  else if ("UPLOAD_FILE" == strRequestName) {   //上传文件
        client_->upload_file(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply, "upload_file");
        });
        return;
    } else if ("GET_CAMERA_VIDEO_LIST" == strRequestName) {   //获取3d摄像头视频文件列表
        client_->get_camera_video_list([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_CAMERA_VIDEO" == strRequestName) {   //获取3d摄像头视频文件
        client_->get_camera_video(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_ERRORS" == strRequestName) {   //获取错误信息
        client_->get_errors([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("STOP_CHARGING" == strRequestName) {   // 停止充电
        client_->stop_charging([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_DO" == strRequestName) {   // 设置输出
        client_->set_do(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_DI" == strRequestName) {   // 设置输入
        client_->set_di(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CLEAR_ERRORS" == strRequestName) {   // 清除错误信息 api (直到底层再次上报错误信息)
        client_->clear_errors([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CLEAR_ERRORS" == strRequestName) {   // 清除错误信息 api (直到底层再次上报错误信息)
        client_->clear_errors([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_PROCESSES_INFO" == strRequestName) {   // 获取进程信息
        client_->get_processes_info(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CHECK_FD_KEEP_ALIVE" == strRequestName) {   // 获取指定 socket fd 的 keep_alive 配置
        client_->check_fd_keep_alive(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_PLC_DIGITAL_IO" == strRequestName) {   // 获取 plc dido
        client_->get_plc_digital_io(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_WIFI_LIST" == strRequestName) {   // 获取 wifi 列表
        client_->get_wifi_list([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_WIFI_CONFIG" == strRequestName) {   // 设置 wifi 配置
        client_->set_wifi_config(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_NETWORK_INTERFACE" == strRequestName) {   // 获取 网卡列表
        client_->get_network_interface([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("GET_EXECUTION_QUEUE" == strRequestName) {   // 获取 执行队列
        client_->get_execution_queue([this](const std::string& reply) {
            this->common_callback(reply);
            this->display_execution_queue(reply);
        });
        return;
    } else if ("GET_QR_CAMERA_DATA" == strRequestName) {   // 获取 相机扫码数据
        client_->get_qr_camera_data([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }  else if ("CANCEL_GET_QR_CAMERA_DATA" == strRequestName) {   // 取消获取 扫码相机数据
        client_->cancel_get_qr_camera_data();
        return;
    } else if ("GET_SCAN2POINTCLOUD" == strRequestName) {   // 获取 避障点云
        client_->get_scan2pointcloud([this](const std::string& reply) {
            // this->common_callback(reply);
            std::cerr << std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000'000 << "\n";
        });
        return;
    } else if ("CANCEL_GET_SCAN2POINTCLOUD" == strRequestName) {   // 取消获取 避障点云
        client_->cancel_get_scan2pointcloud();
        return;
    } else if ("GET_OBST_POLYGON" == strRequestName) {   // 获取 避障轮廓
        client_->get_obst_polygon_once([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_GET_OBST_POLYGON" == strRequestName) {   // 取消获取 避障轮廓
        client_->cancel_get_obst_polygon();
        return;
    } else if ("GET_OBST_PCL" == strRequestName) {   // 获取 避障障碍物
        client_->get_obst_pcl_once([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_GET_OBST_PCL" == strRequestName) {   // 取消获取 避障障碍物
        client_->cancel_get_obst_pcl();
        return;
    } else if ("GET_MODEL_POLYGON" == strRequestName) {   // 获取 模型轮廓
        client_->get_model_polygon([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_GET_MODEL_POLYGON" == strRequestName) {   // 取消获取 模型轮廓
        client_->cancel_get_model_polygon();
        return;
    } else if ("ENABLE_ROBOT" == strRequestName) {   // 开启/关闭使能
        client_->enable_robot(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_COORDINATE_SYSTEM" == strRequestName) {   // 切换坐标系
        client_->set_coordinate_system(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_TOOL" == strRequestName) {   // 切换工具
        client_->set_tool(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("JOG_SINGLE_AXIS" == strRequestName) {   // 单轴点动
        client_->jog_single_axis(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("ONE_CLICK_HOMING" == strRequestName) {   // 一键回原点
        client_->one_click_homing(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_TEACHIN_FILE_LIST" == strRequestName) {   // 获取示教文件列表
        client_->get_teachin_file_list([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_TEACHIN_FILE" == strRequestName) {   // 获取示教文件
        client_->get_teachin_file(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("PUSH_TEACHIN_POINTS" == strRequestName) {   // 上传示教点
        client_->push_teachin_points(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_TEACHIN_POINTS" == strRequestName) {   // 获取示教点位数据
        std::string filename = strText.toUtf8().constData(); // 转换为 UTF-8 编码的 std::string
        client_->get_teachin_points(filename, [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_ROBOT_STATE" == strRequestName) {   // 获取机器人状态
        client_->get_robot_state([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("CANCEL_GET_ROBOT_STATE" == strRequestName) {   // 取消获取机器人状态
        client_->cancel_get_robot_state();
        return;
    } else if ("GET_CHASSIS_INFO" == strRequestName) {   // 获取底盘信息
        client_->get_chassis_info([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("DELETE_TEACHIN_FILES" == strRequestName) {   // 删除示教文件
        client_->delete_teachin_files(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_BROKER_CONNECTION" == strRequestName) {   // 获取 broker 连接状态
        client_->get_broker_connection([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SET_RCS_ONLINE" == strRequestName) {   // 设置 RCS 在线/离线
        client_->set_rcs_online(strText.toStdString(), [this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("SOFT_RESET" == strRequestName) {   // 软复位
        client_->soft_reset([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    } else if ("GET_RACK_NUMBER" == strRequestName) {   // 获取货架编号
        client_->get_rack_number([this](const std::string& reply) {
            this->common_callback(reply);
        });
        return;
    }
}


void MainWindow::display_execution_queue(const std::string &reply)
{
    QMetaObject::invokeMethod(this, [this, reply]() {
        ui->listWidget_executionQueue->clear();
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(reply).toUtf8());
        if (!doc.isObject()) {
            return;
        }
        QJsonObject rootObj = doc.object();
        if (rootObj["code"].toInt() != 0) {
            return;
        }

        QJsonObject dataObj = rootObj["data"].toObject();
        int currentIndex = dataObj["current_execution_index"].toInt(-1);
        QJsonArray queueArray = dataObj["execution_queue"].toArray();

        for (int i = 0; i < queueArray.size(); ++i) {
            QJsonObject itemObj = queueArray[i].toObject();
            int type = itemObj["type"].toInt();
            QString name = itemObj["name"].toString();
            QString itemTypeStr = (type == 0) ? "任务" : "动作";
            QString actionStr = itemObj["action"].toString();
            QString displayText = QString("[%1] %2 (%3)").arg(itemTypeStr).arg(name).arg(actionStr);

            QListWidgetItem *item = new QListWidgetItem(displayText);
            if (i == currentIndex) {
                item->setBackground(Qt::yellow); // 高亮当前执行项
                item->setText("--> " + displayText);
            }
            ui->listWidget_executionQueue->addItem(item);
        }
    }, Qt::QueuedConnection);
}

void start_agv(const std::string& reply)
{
    std::cerr << "[" << __func__ << ":" << __LINE__ << "] " << reply << "\n";
}

void stop_agv(const std::string& reply)
{
    std::cerr << "[" << __func__ << ":" << __LINE__ << "] " << reply << "\n";
}

void MainWindow::on_pushButton_uploadfile_clicked()
{
    //获取打开的文件显示到消息框
    QString file_name = QFileDialog::getOpenFileName(this,"upload b file", "d://");
    QMessageBox::information(this, "...", file_name);
    //std::string file_name_str = file_name.toStdString();
    std::string file_name_str = file_name.toLocal8Bit().constData();    //解决 QString 包含中文时转换为 std::string 乱码的问题

    std::cerr << "file_name_str=" << file_name_str << "\n";
    QString echo_string = "准备上传文件: " + file_name;
    ui->plainTextEdit_uploadfile->setPlainText(echo_string);
    client_->upload_file(file_name_str, [this](const std::string& reply) {
        this->common_callback(reply, "upload_file");
    });
}

void MainWindow::on_pushButton_stop_hb_clicked()
{
    client_->stop_heart_beat();
    std::cerr <<"stop heart_beat\n";
}

void MainWindow::on_pushButton_restart_hb_clicked()
{
        client_->restart_heart_beat();
        // std::cerr <<"restart heart_beat\n";
}

void MainWindow::on_pushButton_agv_positon_clicked()
{
    client_->cancel_get_agv_position();
    // std::cerr <<"cancel_get_agv_position\n";
}

void MainWindow::on_pushButton_set_manual_clicked()
{
    client_->set_operating_mode(qclcpp::Client::manual_mode, [this](const std::string& reply) {
        this->common_callback(reply);
    });
    // std::cerr << "call set_operating_mode: manual\n";
}

void MainWindow::on_pushButton_set_auto_clicked()
{
    client_->set_operating_mode(qclcpp::Client::automatic_mode, [this](const std::string& reply) {
        this->common_callback(reply);
    });
    // std::cerr << "call set_operating_mode: auto\n";
}

void MainWindow::on_pushButton_get_model_clicked()
{
    client_->get_operating_mode([this](const std::string& reply) {
        this->common_callback(reply);
    });
}

void MainWindow::on_pushButton_teaching_clicked()
{
    client_->set_operating_mode(qclcpp::Client::teaching_mode, [this](const std::string& reply) {
        this->common_callback(reply);
    });
}

void MainWindow::on_pushButton_point_cloud_clicked()
{
    client_->cancel_get_point_cloud();
}

void MainWindow::on_pushButton_clicked()
{
    client_->get_processes_info(R"({ "processes": [ "agv_server_pubsub_node" ] })", [this](const std::string& reply) {
        this->common_callback(reply);
    });
}

void MainWindow::on_pushButton_get_task_queue_clicked()
{
    client_->get_execution_queue([this](const std::string& reply) {
        this->common_callback(reply);
        this->display_execution_queue(reply);
    });
}
