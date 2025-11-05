#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "client.h"
#include <asio/io_context.hpp>
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
class ClientManager {
public:
    // 获取单例实例
    static ClientManager& instance() {
        static ClientManager instance_;
        return instance_;
    }

    // 获取 qclcpp::Client 实例
    std::shared_ptr<qclcpp::Client> getClient() {
        return client_;
    }

    // 禁用拷贝构造和赋值操作
    ClientManager(const ClientManager&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;

private:
    ClientManager() : client_(qclcpp::Client::create(io_context_)) {
        std::thread([this]() {
            asio::io_context::work work(io_context_);
            io_context_.run();
        }).detach();
    }

    ~ClientManager() {
        io_context_.stop();
    }

    asio::io_context io_context_;
    std::shared_ptr<qclcpp::Client> client_;
};

#endif // CLIENT_MANAGER_H
