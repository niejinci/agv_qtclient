[toc]
***

# 名词解释

**qclcpp**: qt client library for c++

qclcpp 提供 c++ API 给qt客户端使用

# 客户端服务端通讯协议定义

客户端和服务端之间使用tcp/ip协议通信，应用层请求数据格式参考 **博纳特socket通讯协议**, 采用标准的 TLV(Type, Length, Value) 格式。

应用层数据格式如下：

| 字段名称   | 类型      | 长度(byte) | 说明                                                                |
|-----------|-----------|------------|--------------------------------------------------------------------|
| SyncByte  | uint16_t  | 2          | 0x4E66 固定包头                                                     |
| Command   | uint16_t  | 2          | 对应gui操作的命令字段，用于区分不同的操作，例如，0xfefe, 表示心跳请求   |
| Length    | uint16_t  | 2          | 包含 Uuid, Data 的长度                                              |
| Uuid      | string    | 36         | 例如: 96480a00-e856-41e1-9d95-932d11a02828，用于区分本次操作         |
| Data      | string    | {Length}-36| 操作实际发送的数据，json 格式                                        |

## 服务端端口

所有操作的端口都是: **9034**

## 应用层请求数据示例

以下是发送心跳请求时候的应用层数据：
```text
4e66fefe003444206cd2-f0a5-4afa-a9ed-548af7f12d54{"data": "Ping"}
```
1. **第 1-4 个字节：同步码（4e66）**
   - 内容：4e66
   - 说明：用于标识数据包的起始。该字段为 `uint16_t` 类型，格式化为 4 个十六进制字符，占用 4 个字节。

2. **第 5-8 个字节：命令字（fefe）**
   - 内容：fefe
   - 说明：表示数据包的命令类型。该字段为 `uint16_t` 类型，格式化为 4 个十六进制字符，占用 4 个字节。

3. **第 9-12 个字节：数据长度（0034）**
   - 内容：0034
   - 说明：表示后续数据部分的长度（以字节为单位）。此字段为 `uint16_t` 类型，格式化为 4 个十六进制字符。例子中 0034 表示 52 字节。

4. **第 13-48 个字节：UUID（44206cd2-f0a5-4afa-a9ed-548af7f12d54）**
   - 内容：44206cd2-f0a5-4afa-a9ed-548af7f12d54
   - 说明：唯一标识本次操作的 UUID，占用 36 个字节。

5. **第 49-64 个字节：数据体（{"data": "Ping"}）**
   - 内容：{"data": "Ping"}
   - 说明：实际发送的数据内容，本例为心跳接口的数据，长度为 16 字节。

# API

* 所有 api (除了 connect, disconnect, switch_server) 的返回类型都是 bool，如果是 false 表示请求失败(**此时还未发起异步操作**)。如果是 true 表示发起异步操作成功，实际返回值的处理在回调函数中进行。
<br>

* 回调函数的入参是一个 json 格式的字符串，在回调函数体中，将其解析为json 对象，解析后的对象具有三个字段: code, message, data(如果code非0，说明请求失败了，此时无需关注data)，例如：

    ```json
    {
        "code": 0,
        "data": {
            "filename": "./log/agv_server_pubsub/agvcomm.log",
        },
        "message": "success"
    }
    ```
    ```json
    {
        "code": 9,
        "message": "file_path is empty"
    }
    ```
    请求参数字段说明:

    | 字段名称      | 类型         | 含义                                               |
    |---------------|-------------|----------------------------------------------------|
    | code       | int            | 状态码，0表示成功，非0表示失败                       |
    | message    | string         | code 非0时，表示对应的错误信息                       |
    | data       | json object    | 响应数据，**code 非0时，无需关注该字段**              |


## connect

**函数签名**
```cpp
void connect(const std::string& host, const std::string& port, std::function<void(bool)> callback)
```

**描述**
connect 函数用于异步连接到指定的服务器地址和端口。该函数不阻塞调用线程，而是立即返回，并在连接尝试完成后通过回调函数通知结果。

**参数**
* **host**(const std::string&)
    * 服务器ip地址

* **port**(const std::string&)
    * 服务器监听的端口号，以字符串形式表示，例如: "9034"

* **callback**(std::function<void(bool)>)
    * 连接操作完成后的回调函数。当连接尝试结束时（无论成功还是失败），这个回调会被调用。
    * 回调函数接受一个布尔类型的参数：
        * true: 表示连接成功。
        * false: 表示连接失败。

**返回值**
无返回值 (void)。所有关于连接操作的结果信息都将通过提供的回调函数进行传递。

**示例代码**
```cpp
//connect_demo.cpp
#include "client.h"
#include <iostream>
#include <asio.hpp>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " host port\n";
        return 1;
    }

    asio::io_context io;
    std::shared_ptr<qclcpp::Client> client = qclcpp::Client::create(io);
    client->connect(argv[1], argv[2], [](bool success) {
        if (success) {
            std::cout << "connect success\n";
        } else {
            std::cout << "connect fail\n";
        }
    });
    io.run();

    std::cout << "main end\n";
    return 0;
}
```

## disconnect

**函数签名**
```cpp
void disconnect(std::function<void(std::string& error)> callback = default_disconnect_callback);
```

**描述**
断开跟服务端的连接。

**参数**
* **callback**(std::function<void(std::string& error)>)
    * 断开连接的回调函数，有一个参数，表示操作的结果，回调函数有默认值，其实现如下：
        ```cpp
        void Client::default_disconnect_callback(std::string& error)
        {
            std::cout << error << std::endl;
        }
        ```

**返回值**
无

**示例代码**
```cpp
//disconnect_demo.cpp

//...
    asio::io_context io;
    std::shared_ptr<qclcpp::Client> client = qclcpp::Client::create(io);
    client->disconnect();
//...
```

## switch_server

**函数签名**
```cpp
void switch_server(const std::string& new_host, const std::string& new_port, SwitchServerCallback callback = default_switch_callback);
```

**描述**
切换连接到新的服务器地址。

**参数**
* **new_host**(const std::string&)
    新服务器的IP地址或域名。
* **new_port**(const std::string&)
    新服务器的端口号。
* **callback**(SwitchServerCallback, 可选)
    切换服务器操作完成后的回调，默认使用default_switch_callback。其源代码为:
    ```cpp
    void default_switch_callback(bool success) {
        if (success) {
            std::cout << "Successfully switched to the new server." << std::endl;
        } else {
            std::cout << "Failed to switch to the new server." << std::endl;
        }
    }
    ```

**返回值**
无返回值 (void)。

**示例代码**
```cpp
//disconnect_demo.cpp

//...
    asio::io_context io;
    std::shared_ptr<qclcpp::Client> client = qclcpp::Client::create(io);
    client->switch_server("120.24.252.217", "9890");
//...
```

## upload_file

**函数签名**
```cpp
bool upload_file(const std::string& args, ResponseHandler handler);
```

**描述**
上传文件到服务端。
默认保存在目录: /home/docker_share/, 模型文件保存在目录: /home/byd/config/model/，上传模型文件后，会重启docker 。
识别文件保存在目录: /home/byd/config/video/

**参数**
* **file_path**(const std::string&)
    json格式的字符串，包含文件路径和文件类型，例如：
    ```json
    {
        "filepath": "D:/robotics/agv_model_para.json",
        "type": "model"
    }
    ```
    请求参数字段说明:

    | 字段名称      | 类型   | 含义                                                |
    |---------------|--------|-----------------------------------------------------|
    | filepath       | string | 要上传的文件路径                                    |
    | type           | string | 文件类型【可选字段】，目前有： model-模型文件，video-识别文件，如果没有该字段，上传的文件默认放到 /home/docker_share/ 目录下     |
* **handler**(ResponseHandler)
    当文件上传完成或发生错误时调用的回调函数。
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "message": "file upload completed"
    }
    ```
    上传失败：
    ```json
    {
        "code": -1,
        "data": {
            "receivedFileSize": 0,
            "totalFileSize": 42638,
            "transmissionPercentage": "0.000%"
        },
        "message": "can't open or create file:/home/byd/config/model/agv_model_para.json"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如:
        * 传入的回调函数为空；
        * 没有请求名称对应的命令字配置；
        * 未连接到服务器;
        * 传入的文件路径为空；
        * 文件内容为空；
        * 另一个文件正在上传；

**示例代码**
```cpp
bool ret = client_->upload_file(args, [this](const std::string& reply) {
    this->common_callback(reply, "upload_file");
});
if (!ret) {
    //错误处理
}
```

## push_map

**函数签名**
```cpp
bool push_map(const std::string& file_path, ResponseHandler handler);
```

**描述**
推送地图文件给小车。推送的地图位于 "/home/byd/data/map/gui/" 目录下。

**参数**
* **file_path**(const std::string&)
    待推送文件的本地路径（注意是绝对路径）。例如: D:/robotics/2d-1.smap
* **handler**(ResponseHandler)
    当推送文件完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "message": "success",
        "data": {
            "current_push_size": 1153,
            "total_size": 1153
        }
    }
    ```
    推送失败的示例：
    ```json
    {
        "code": 1,
        "message": "D:/robotics/2d-1.smap is pushing, please wait"
    }

    //or
    {
        "code": 9,
        "message": "file_path is empty"
    }
    ```

**返回值**

* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如，file_path 为空，未提供 handler, 未连接服务器，当前正在推送文件

**示例代码**
```cpp
bool ret = client_->push_map(strText.toStdString(), [this](const std::string& reply) {
    this->common_callback(reply);
});
if (!ret) {
    std::cout << "push map failed\n";
}
```

## pull_map

**函数签名**
```cpp
bool pull_map(const std::string& map_name, ResponseHandler handler);
```

**描述**
从小车拉取地图文件

**参数**
* **map_name**(const std::string&)
    待拉取文件的名称。例如: pc/2d-1/2d-1.smap (获取地图文件名称的典型方法是先调用 get_map_list 获取服务器上的地图文件列表，然后选中一个文件，就获取了文件名称，注意文件名称的格式: 来源/文件名称/文件名称.smap)
    如果要拉取 unkown 分类中的文件，直接传递 "文件名称.smap"

* **handler**(ResponseHandler)
    当拉取文件完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "filename": "./map/pc/2d-1/2d-1.smap"
        },
        "message": "success"
    }
    ```
    拉取失败的示例:
    ```json
    {
        "code": 4,
        "message": "Failed to open file."
    }
    ```

**返回值**
* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如， map_name 为空，未提供 handler, 未连接服务器，没有请求名称对应的命令字，当前正在拉取文件

**示例代码**
```cpp
bool ret = client_->pull_map(strText.toStdString(), [this](const std::string& reply) {
    this->common_callback(reply);
});
if (!ret) {
    std::cout << "pull map failed\n";
}
```

## get_agv_position

**函数签名**
```cpp
bool get_agv_position(ResponseHandler handler);
```

**描述**
获取小车位置。
调用该函数后，client 库中会启动一个定时器，定期获取小车位置。如果要停止定时器，调用 cancel_get_agv_position()。重新获取点云数据，再次调用 get_agv_position 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车位置信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "deviation_range": 0,
            "localization_score": 11.73,
            "map_description": "",
            "map_id": "2d-1",
            "position_initialized": true,
            "theta": -0.5989412450106449,
            "x": 3.9941876561129983,
            "y": 0.525087608040933
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_agv_position(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_agv_position

**函数签名**
```cpp
void cancel_get_agv_position();
```

**描述**
取消获取小车位置信息。

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_agv_position();
```

## get_point_cloud

**函数签名**
```cpp
bool get_point_cloud(ResponseHandler handler);
```

**描述**
获取小车点云数据
调用该函数后，client 库中会启动一个定时器，定期获取小车点云。如果要停止定时器，调用 cancel_get_point_cloud()，重新获取点云数据，再次调用 get_point_cloud 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车点云信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
                    {
                        "x": 1.1,       //  空间坐标
                        "y": 2.2,       //  空间坐标
                        //"z": 0.0,       //  空间坐标，去掉，未用到
                        "i": 2.2        // 反射强度 (Reflectivity Intensity), 通常由激光雷达（LiDAR）提供。LiDAR 发射激光脉冲，当脉冲击中物体表面后会反射回传感器。intensity 值表示返回信号的强度
                    },
                    //...
            ],
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_point_cloud(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_point_cloud

**函数签名**
```cpp
void cancel_get_point_cloud();
```

**描述**
取消获取小车位置信息。

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_point_cloud();
```

## set_operating_mode

**函数签名**
```cpp
bool set_operating_mode(int mode, ResponseHandler handler);
```

**描述**
设置小车操作模式。**服务器端发布主题后，就会响应成功。实际设置的结果要通过 get_operating_mode() api 获取。**

**参数**
* **mode**(int)
    设置小车操作模式，目前定义了以下几种操作模式：
    ```cpp
    // 操作模式常量
    static constexpr int automatic_mode     = 0;
    static constexpr int semiautomatic_mode = 1;
    static constexpr int manual_mode        = 2;
    static constexpr int service_mode       = 3;
    static constexpr int teaching_mode      = 4;
    ```

* **handler**(ResponseHandler)
    当设置小车操作模式完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "mode": 2   //回显请求参数
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->set_control_mode(qclcpp::Client::manual_mode, handler);
if (!ret) {
    //错误处理
}
```

## get_operating_mode

**函数签名**
```cpp
bool get_operating_mode(ResponseHandler handler);
```

**描述**
获取小车当前的操作模式。

**参数**
* **handler**(ResponseHandler)
    获取小车操作模式完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "driving": false,
            "e_stop": 0,
            "operating_mode": 0
        },
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                             |
    | operating_mode | int      | 见前面的操作模式常量定义                              |
    | driving        | bool     | true: AGV 正在行驶和/或旋转。false: AGV 不行驶，也不旋转  |
    | e_stop         | int      | 安全停车状态{AUTOACK,MANUAL,REMOTE,NONE}: <br>AUTOACK: 可以自动恢复的安全停车触发，如遇障 <br> MANUAL: 需要手动恢复的安全停车触发，如碰撞条 <br> REMOTE: 可以远程消除的安全停车触发  <br> NONE: 正常状态 |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_control_mode(handler);
if (!ret) {
    //错误处理
}
```

## get_map_list

**函数签名**
```cpp
bool get_map_list(ResponseHandler handler);
```

**描述**
获取小车的地图文件列表。

**参数**
* **handler**(ResponseHandler)
    获取小车地图文件列表完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "pc": [         //来自qt上位机的地图文件
                "m.smap",
                "n.smap"
            ],
            "rcs": [    //来自rcs的地图文件
                "y.smap",
                "x.smap"
            ],
            "unkown": [
                "byd_agv.smap"
            ]
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器


**示例代码**
```cpp
bool ret = client_->get_map_list(handler);
if (!ret) {
    //错误处理
}
```

## check_connectivity

**函数签名**
```cpp
void check_connectivity(const std::vector<std::string>& ip_list, ResponseHandler handler);
```

**描述**
检测列表中的ip是否可以连接

**参数**
* **ip_list**(std::vector<std::string>)
    包含ip地址的数组

* **handler**(ResponseHandler)
    检测ip列表连接完成或超时时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    [
        {
            "connected": true,
            "ip": "192.168.192.124"
        },
        {
            "connected": false,
            "ip": "192.168.192.255"
        },
        {
            "connected": false,
            "ip": "192.168.192.123"
        }
    ]
    ```
    响应字段说明：
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | array         | json 数组 |                                                   |
    | array[0]      | json 对象 |                                                   |
    | connected     | bool     | true: 能连接服务器，false: 不能连接服务器            |
    | ip            | string   | 待检测的 ip                                        |

**返回值**
无

**示例代码**
```cpp
std::vector<std::string> ip_list{"192.168.192.124", "192.168.192.125", "192.168.192.126"};
client_->check_connectivity(ip_list, handler);
```

## get_log_list

**函数签名**
```cpp
bool get_log_list(ResponseHandler handler);
```

**描述**
获取日志文件列表

**参数**
* **handler**(ResponseHandler)
    获取列表连接完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "agv_server_pubsub_node": [     //agv_server_pubsub_node 是日志目录下的模块名称，正常应该是一个模块一个子目录
                "agvcomm.log",
                "agvcomm.1.log"
            ],
            "unkown_module": [
                "vda.log"
            ]
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器


**示例代码**
```cpp
bool ret = client_->get_log_list(handler);
if (!ret) {
    //错误处理
}
```

## get_log_file

**函数签名**
```cpp
bool get_log_file(const std::string& file_name, ResponseHandler handler);
```

**描述**
获取日志文件。

**参数**
* **file_name**(std::string)
    要获取的日志文件名称，例如: agv_server_pubsub_node/agvcomm.log (获取文件名称的典型方法是先调用 get_log_list 获取服务器上的日志文件列表，然后选中一个文件，就获取了文件名称，file_name格式为：模块名称/xxx.log)

* **handler**(ResponseHandler)
    获取日志文件完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "filename": "./log/agv_server_pubsub/agvcomm.log"           //拉取的日志存放在程序当前 log 目录下(跟地图的 map 目录平级)
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如， file_name 为空，未提供 handler, 未连接服务器，当前正在获取文件


**示例代码**
```cpp
//入参 file_name 格式: 模块/日志文件, 例如： agv_server_pubsub_node/agvcomm.log
//如果是模块是 unkown_module, 直接传文件名，例如, vda.log
bool ret = client_->get_log_file("agv_server_pubsub_node/agvcomm.log", handler);
if (!ret) {
    //出错处理
}
```

## get_datetime

**函数签名**
```cpp
bool get_datetime(ResponseHandler handler);
```

**描述**
获取系统时间。

**参数**
* **handler**(ResponseHandler)
    获取系统时间完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "datetime": "2025-03-31 19:01:22"
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_datetime(handler);
if (!ret) {
    //错误处理
}
```

## set_datetime

**函数签名**
```cpp
bool set_datetime(const std::string& datetime, ResponseHandler handler);
```

**描述**
设置系统时间。

**参数**
* **datetime**(std::string)
    要设置的系统时间，格式为 yyyy-mm-dd HH:MM:ss ，例如: 2025-03-17 08:42:22
* **handler**(ResponseHandler)
    设置系统时间完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": "{\"datetime\": \"2025-03-17 10:00:11\"}",  //回显请求参数
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->set_datetime("2025-03-31 15:33:29", handler);
if (!ret) {
    //错误处理
}
```

## reboot_or_poweroff

**函数签名**
```cpp
bool reboot_or_poweroff(const std::string& args, ResponseHandler handler);
```

**描述**
重启或关闭系统。

**参数**
* **args**(std::string)
    json格式的字符串，例如:
    ```json
    {
        "command": "reboot" //or "poweroff"
    }
    ```
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": "{\"datetime\": \"2025-03-17 10:00:11\"}",  //回显请求参数
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->reboot_or_poweroff(R"({"command": "reboot")", handler);
if (!ret) {
    //错误处理
}
```

## terminal_command

**函数签名**
```cpp
bool terminal_command(const std::string& args, ResponseHandler handler);
```

**描述**
终端命令：在 gui 执行终端命令，就像ssh到远程服务器执行命令一样。

**参数**
* **args**(std::string)
    请求参数，json 格式字符串，例如：
    ```json
    {
        "command": "ifconfig"
    }

    //or

    {
        "command": "date",
        "tty": "3a57de45-433f-4db6-b34c-41ed4c22f02b",
        "next_chunk": 2
    }
    ```
    请求参数字段说明:

    | 字段名称      | 类型   | 含义                                                |
    |---------------|--------|-----------------------------------------------------|
    | command       | string | 要执行的终端命令，例如: ifconfig, ps -ef            |
    | tty           | string | 虚拟终端号，从响应获取，首次请求时，无需传递        |
    | next_chunk    | int    | 要获取的下一个输出块，首次请求时，无需传递该字段    |


* **handler**(ResponseHandler)
    设置系统时间完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "current_chunk": 1,
            "total_chunks": 2,
            "tty": "3a57de45-433f-4db6-b34c-41ed4c22f02b",
            "cmd_output": "eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500\n        inet 172.17.0.2  netmask 255.255.0.0  broadcast 172.17.255.255\n        ether 02:42:ac:11:00:02  txqueuelen 0  (Ethernet)\n        RX packets 2263  bytes 164695 (164.6 KB)\n        RX errors 0  dropped 0  overruns 0  frame 0\n        TX packets 24617  bytes 7474080 (7.4 MB)\n        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0\n\nlo: flags=73<"
        },
        "message": "success"
    }

    {
        "code": 0,
        "data": {
            "current_chunk": 2,
            "total_chunks": 2,
            "tty": "3a57de45-433f-4db6-b34c-41ed4c22f02b",
            "cmd_output": "UP,LOOPBACK,RUNNING>  mtu 65536\n        inet 127.0.0.1  netmask 255.0.0.0\n        inet6 ::1  prefixlen 128  scopeid 0x10<host>\n        loop  txqueuelen 1000  (Local Loopback)\n        RX packets 22981  bytes 7051624 (7.0 MB)\n        RX errors 0  dropped 0  overruns 0  frame 0\n        TX packets 22981  bytes 7051624 (7.0 MB)\n        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0\n\n"
        },
        "message": "success"
    }
    ```
    有些命令执行后没有输出结果，例如: cd ..
    ```json
    {
        "code": 0,
        "data": {
            "current_chunk": 0,
            "total_chunks": 0,
            "tty": "39232155-38d2-46f0-a202-83495a88deb2"
        },
        "message": "success"
    }
    ```

    响应字段说明：
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string | 失败信息，例如参数为空                              |
    | data           | json 对象 | 终端命令执行的输出结果，注意字符串中可能包含了换行符(\n), 前端显示的时候，要换行  |
    | current_chunk  | int    | 终端命令执行的输出结果可能很长，超出一屏，服务端分块返回，表示当前的块号，编号从1开始计数 |
    | total_chunks   | int    | 终端命令执行的输出结果总共有多少块                  |
    | tty            | string | 虚拟终端号                                          |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器；参数格式不对

**示例代码**
```cpp
//在终端执行 ifconig 命令
bool ret = client_->terminal_command(R"({ "command": "ifconfig" })", handler);
if (!ret) {
    //错误处理
}

//如果返回的结果显示超过一块，客户端应该继续请求下一块(例如，按下 Enter 键时，触发请求下一块)
ret = client_->terminal_command(R"({ "command": "ifconfig", "tty": "3a57de45-433f-4db6-b34c-41ed4c22f02b", "next_chunk": 2 })", handler);
if (!ret) {
    //错误处理
}

//如果中途退出终端命令页面(中途定义: 有执行命令，但是在获取完命令的输出结果之前就退出了，请求响应中的 current_chunk < total_chunks)
//发送 exit 命令，告诉服务端清空终端命令的输出结果
ret = client_->terminal_command(R"({ "command": "exit", "tty": "3a57de45-433f-4db6-b34c-41ed4c22f02b", "next_chunk": 2 })", handler);
if (!ret) {
    //错误处理
}
```

## build_map

**函数签名**
```cpp
bool build_map(ResponseHandler handler);
```

**描述**
构建地图(2d)。

**参数**
* **handler**(ResponseHandler)
    设置系统时间完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": "",
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器；

**示例代码**
```cpp
bool ret = client_->build_map(handler);
if (!ret) {
    //错误处理
}
```

## get_localization_quality 【已废弃】

**函数签名**
```cpp
bool get_localization_quality(ResponseHandler handler);
```

**描述**
获取定位质量数据

**参数**
* **handler**(ResponseHandler)
    获取定位质量完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "localization_score": 0.16,
            "theta": 0,
            "x": 0,
            "y": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器；

**示例代码**
```cpp
bool ret = client_->get_localization_quality(handler);
if (!ret) {
    //错误处理
}
```

## get_velocity

**函数签名**
```cpp
bool get_velocity(ResponseHandler handler);
```

**描述**
获取速度(线速度和角速度)

**参数**
* **handler**(ResponseHandler)
    获取速度完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "angular_velocity": 0,
            "linear_velocity": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_velocity(handler);
if (!ret) {
    //失败处理
}
```

## get_mcu2pc

**函数签名**
```cpp
bool get_mcu2pc(ResponseHandler handler);
```

**描述**
获取电池和电机状态信息

**参数**
* **handler**(ResponseHandler)
    获取电池和电机状态信息完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "battery": {
                "battery_charge": 96,       // 当前电量剩余百分比
                "battery_voltage": 53,      // 当前电压
                "charging": false           // 手动充电时一直都是 false, 通过充电桩充电时，才会是 true
            },
            "chassis_io": {
                "gpio_input_1": 1,
                "gpio_input_2": 0,
                "gpio_input_3": 0,
                "gpio_output_1": 0,
                "gpio_output_2": 0,
                "gpio_output_3": 0,
                "logic_state_1": 2,
                "logic_state_2": 0
            },
            "left_servo": {
                "driver_state": 256,
                "encoder_data": 32666,
                "error_code": 0,
                "time_gap": 0
            },
            "lift_servo": {
                "driver_state": 5175,
                "encoder_data": -1,
                "error_code": 0,
                "time_gap": 0
            },
            "right_servo": {
                "driver_state": 256,
                "encoder_data": 32669,
                "error_code": 0,
                "time_gap": 0
            },
            "rotation_servo": {
                "driver_state": 5175,
                "encoder_data": 0,
                "error_code": 0,
                "time_gap": 0
            },
            "dis": [
                1,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
            ],
            "dos": [
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
            ]
        },
        "message": "success"
    }
    ```
    响应字段说明：
    | 字段名称         | 类型   | 含义                                                |
    |------------------|--------|----------------------------------------------------|
    | battery          | 对象   | 电池信息                                            |
    | chassis_iosage   | 对象   | 底盘io                                              |
    | left_servo       | 对象   | 左电机                                              |
    | right_servo      | 对象   | 右电机                                              |
    | lift_servo       | 对象   | 顶升电机                                            |
    | rotation_servo   | 对象   | 旋转电机                                            |
    | dis              | 数组   | gpio di 数组: di0-di23                             |
    | dos              | 数组   | gpio do 数组: do0-do23                              |

    电机字段说明：
    | 字段名称          | 类型   | 含义                                                |
    |------------------|--------|-----------------------------------------------------|
    | driver_state     | 整型   | 状态字                                               |
    | encoder_data     | 整型   | 转速                                                 |
    | error_code       | 整型   | 错误码                                               |
    | time_gap         | 整型   | 未用到                                               |


**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->get_mcu2pc(handler);
if (!ret) {
    //失败处理
}
```

## ota_upgrade

**函数签名**
```cpp
bool ota_upgrade(const std::string& args, ResponseHandler handler);
```

**描述**
ota升级

**参数**
* **args**(std::string)
    升级参数，json格式字符串:
    ```json
    {
        "command": "upgrade",   //升级命令
        "version": "v2.0.9"     //版本号
    }
    ```
    请求参数字段说明:
    | 字段名称      | 类型         | 含义                                                |
    |----------------------------|-------------|-----------------------------------------------------|
    | command     | string        | 升级时传 upgrade,  取消升级时传 disupgrade     |
    | version    | string        | 升级时传递的版本信息, 格式为: vx.y.z 例如: v2.1.2 ,取消升级时可以不传    |

* **handler**(ResponseHandler)
    ota升级完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "msg": "正在从阿里云下载镜像到本地",
            "percentage": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->ota_upgrade(R"({"command": "upgrade", "version": "v2.0.9" })", handler);
if (!ret) {
    //错误处理
}
```
gui 触发升级后， ota_upgrade 会启动定时器，定时请求升级完成进度，升级完成后会停止定时器。


## start_task

**函数签名**
```cpp
bool start_task(const std::string& args, ResponseHandler handler)
```

**描述**
启动任务

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
    "max_speed": 0.3,       // 最大速度，单位 m
    "map_id": "2dd",
    "loop": true,
    "tasks": [
        {
            "cmd": "100a",
            "list": [
                {
                    "start_point": {
                        "a": 0,
                        "n": "LM4",
                        "x": 0,
                        "y": 0,
                        "avoid": {
                            "forward": 0.5,
                            "back": 0.5,
                            "left": 0.2,
                            "right": 0.2
                        }
                    },
                    "control_points": [
                        {
                            "x": 1.1,
                            "y": 2.174,
                            "weight": 1
                        },
                        {
                            "x": 1.2,
                            "y": -0.566,
                            "weight": 1
                        }
                    ],
                    "end_point": {
                        "a": 0,
                        "n": "LM5",
                        "x": 1.1,
                        "y": 0,
                        "avoid": {
                            "forward": 0.5,
                            "back": 0.5,
                            "left": 0.2,
                            "right": 0.2
                        }
                    },
                    "orient": 3.1415926,
                    "orient_type": "GLOBAL",
                    "load_orient": "",   // 默认空字符串
                    "line_speed": 0.3,    // 边的速度，单位 m, 不能超过 max_speed
                    "avoid": {
                        "forward": 0.5,
                        "back": 0.5,
                        "left": 0.2,
                        "right": 0.2
                    }
                }
            ]
        },
        {
            "action": "pick",
            "cmd": "1005",
            "height": 0.05

        },
        {
            "action": "start_charging",
            "cmd": "100e",
            "charging_time": 240,
            "ip": "192.168.192.124",
            "port": 12345
        },
        {
            "action": "set_do",
            "cmd": "f013",
            "index": 0,
            "value": 1
        },
        {
            "action": "wait_di",
            "cmd": "f014",
            "index": 2,
            "value": 0
        },
        {
            "action": "station_code_rectify",
            "cmd": "f015",
            "type": 1
        }
    ]
    }
    ```
    参数字段说明：

    | 字段名称     | 类型       | 含义                                                                                                        |
    |--------------|------------|-------------------------------------------------------------------------------------------------------------|
    | max_speed    | double     | 【**可选**】最大速度，单位m                                                                                             |
    | map_id       | string     | 地图编号                                                                                                      |
    | loop         | bool       | true: 表示循环任务， false: 表示非循环任务                                                                 |
    | tasks        | json 数组  |                                                                                                             |
    | tasks[0]     | json 对象  | 一个任务                                                                                                      |
    | cmd          | string     | 命令字段, 路径导航：100a，顶升: 1005, 100e: 充电, f013: 设置输出，f014: 等待输入                                   |
    | list         | json 数组  | 边信息数组                                                                                                    |
    | list[0]      | json 对象  | 一条边                                                                                                        |
    | start_point  | json 对象  | 一个节点对象                                                                                                  |
    | n            | string     | 节点名称                                                                                                      |
    | a            | double     | 弧度 (-pi, pi]                                                                                                         |
    | x            | double     | x坐标，单位米                                                                                                 |
    | y            | double     | y坐标，单位米                                                                                                 |
    | end_point    | json 对象  | 同 start_point                                                                                                |
    | load_orient  | string     | load_orientation - 货物朝向 <br> Agv在当前路线上时的货物朝向,仅用于潜伏agv; <br> 当agv进入当前edge时agv应判断如果货物朝向与此字段不符则应在agv到达下一节点前旋转货物到指定朝向,未下发则不需要旋转;字段为枚举类型,取值如下: <br> All: 货物角度与地面保持不变(即车转货不转) <br> **Horizontal**: 货物长边与agv车体长边平行,如有货物贴有底码则根据底码,无底码则使用顶升板的方向计算; <br> **Vertical**: 货物长边与agv车体长边垂直,如有货物贴有底码则根据底码,无底码则使用顶升板的方向计算; <br> Front: 仅货物有底码时可用,货物底码方向与车头方向相同; <br> Left: 仅货物有底码时可用,货物底码方向相对车方向逆时针旋转90°; <br> Back: 仅货物有底码时可用,货物底码方向相对车头方向旋转180°; <br> Right: 货物有底码时可用,货物底码方向相对车头方向顺时针旋转90° |
    | orient       | double     | AGV在路径段上的朝向。与orientationType共同定义AGV的形式方式和形式角度，单位弧度                                   |
    | orient_type  | string   | - orientationType 定义了路径段的定位形式，分别为地图全局坐标系(GLOBAL)和路径段的切线(TANGENTIAL) <br> - orientationType = TANGENTIAL, 此时orientation 只有两个取值，为0时表示与前进，Pi时则表示为后退 <br> - orientationType = GLOBAL, 此时，Orientation 表示车在地图下的角度，对于差速车而言，设备只可以以路径方向前进/后退，因此 Orientation 只能为路径段角度或路径段角度减Pi(文档中说的是加pi, 跟奉工对齐是减 pi)，取值范围 [-pi, pi], 路径段角度计算方式为 atan2(y2 - y1, x2 - x1)|
    |line_speed    | double   |小车在该边上的行驶速度，如果大于 max_speed 就取 max_speed 的值                                                      |

    顶升动作节点字段说明
    | 字段名称     | 类型       | 含义                            |
    |--------------|------------|-------------------------------|
    | cmd    | string     | 取值为 "1005"                        |
    | action       | string     | 取值为 "pick" or "drop"       |
    | height       | double     | 顶升高度，单位 m                |

    充电动作节点字段说明 **(一个节点有了充电动作就不能有其他动作)**
    | 字段名称     | 类型       | 含义                            |
    |--------------|------------|-------------------------------|
    | cmd    | string     | 取值为 "100e"                        |
    | action       | string     | **【可选字段】**，取值 "start_charging"       |
    | charging_time | int  | 充电时间，单位分钟                                                                       |
    | ip            | string     | 【**可选**】充电桩IP，非无线桩可以忽略                    |
    | port          | int  | 【**可选**】充电桩端口，非无线桩可以忽略，不下发采用默认端口         |

    设置gpio dido动作节点字段说明
    | 字段名称     | 类型       | 含义                            |
    |--------------|------------|-------------------------------|
    | cmd    | string     | 取值为 "f013" or "f014"                       |
    | action       | string     | **【可选字段】** 取值为 "setDO" or "waitDI"       |
    | index         | int     | io口的下标，取值范围 [0, 23]                  |
    | value         | int     | 0 or 1                                 |

    站点二维码纠正动作节点字段说明
    | 字段名称     | 类型       | 含义                            |
    |--------------|------------|-------------------------------|
    | cmd    | string     | 取值为 "f015"                        |
    | action       | string     | **【可选字段】** 取值为 "station_code_rectify"       |
    | type       | int     |1 - DependencyWareType - 依赖载具类型 <br> 2 - QrCodeChangeDirectionRectify - 站台内原地旋转纠偏 <br>3 - QrCodeExitStationRectify - 出站纠偏  |

    设置贝塞尔曲线节点字段说明
    | 字段名称          | 类型       | 含义                                     |
    |-------------------|------------|----------------------------------------|
    | control_points    | json 数组  | **【可选字段】** 包含控制节点的数组       |
    | control_points[0] | json 对象  | 控制节点，结构如下：                     |
    | x                 | double     | 世界坐标系中描述的X 坐标                 |
    | y                 | double     | 世界坐标系中描述的Y 坐标                 |
    | weight            | double     | 权重，未定义默认为1.0                    |

    避障字段说明
    | 字段名称          | 类型       | 含义                                     |
    |-------------------|------------|----------------------------------------|
    | avoid    | json 对象  | **【可选字段】** 避障对象，小车避障距离 = 货架尺寸 + avoid 对象中对应方向的距离<br>**节点中的 avoid 控制 旋转避障，边中的 avoid 控制直行避障** |
    | forward   | double  | 前方距离，单位m, 默认 0.5                     |
    | back      | double     | 后方距离，单位m, 默认 0.5                  |
    | left      | double     | 左侧距离，单位m, 默认 0.2               |
    | right     | double     | 右侧距离，单位m, 默认 0.2                   |


* **handler**(ResponseHandler)
    启动任务完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //...   请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
bool
    true: 发起异步操作成功，实际结果通过传递给回调函数的参数判断
    false: 启动任务失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->start_task(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## translation

**函数签名**
```cpp
bool translation(const std::string& args, ResponseHandler handler)
```

**描述**
平动

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "action": "up",
        "linear_velocity": 0.5,
        "angular_velocity": 30,
        "distance": 10
    }
    ```
    请求参数字段说明: (按照仙工对应功能定义的，跟实际的后端接口需要的参数并不一样)
    | 字段名称          | 类型      | 含义                                                         |
    |-------------------|-----------|--------------------------------------------------------------|
    | action            | string    | 实际未用到                                                   |
    | linear_velocity   | double    | x方向 移动速度，对应 vx                                      |
    | angular_velocity  | double    | vy                                                           |
    | distance          | double    | 距离，对应 dist                                              |
    | mode              | int       | 前端未传该字段，服务端默认传的 1， 0:定位模式, 1:里程计       |

* **handler**(ResponseHandler)
    平动完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //... 请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->translation(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## lifting

**函数签名**
```cpp
bool lifting(const std::string& args, ResponseHandler handler)
```

**描述**
顶升

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "action": "pick",  //pick or drop
        "height": 0.15     //单位: m
    }
    ```
* **handler**(ResponseHandler)
    顶升完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //... 请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->lifting(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## remote_control

**函数签名**
```cpp
bool remote_control(const std::string& args, ResponseHandler handler)
```

**描述**
遥控

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "action": "up",
        "linear_velocity": 0.5,
        "angular_velocity": 30,
        "distance": 10
    }
    ```
    请求参数字段说明: (按照仙工对应功能定义的，跟实际的后端接口需要的参数并不一样)
    | 字段名称          | 类型      | 含义                                                         |
    |-------------------|-----------|--------------------------------------------------------------|
    | action            | string    | 实际未用到                                                   |
    | linear_velocity   | double    | 移动线速度，对应 linear_x                                  |
    | angular_velocity  | double    | 移动角速度，对应 angular_z                                 |

* **handler**(ResponseHandler)
    遥控完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //... 请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->remote_control(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## rotation

**函数签名**
```cpp
bool rotation(const std::string& args, ResponseHandler handler)
```

**描述**
小车旋转

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "direction": "left",
        "linear_velocity": 0.2,
        "angular_velocity": 45,
        "distance": 0
    }
    ```
    请求参数字段说明:(按照仙工对应功能定义的，跟实际的后端接口需要的参数并不一样)
    | 字段名称          | 类型      | 含义                                                         |
    |-------------------|-----------|--------------------------------------------------------------|
    | direction         | string    | 后端未用到                                                   |
    | linear_velocity   | double    | 旋转速度，后端对应 vw                                        |
    | angular_velocity  | double    | 旋转角度，后端对应 angle                                     |
    | distance          | double    | 后端未用到                                                   |

* **handler**(ResponseHandler)
    旋转完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //... 请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->rotation(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## cancel_task

**函数签名**
```cpp
bool cancel_task(const std::string& args, ResponseHandler handler)
```

**描述**
取消任务

**参数**
* **args**(std::string)
    json格式的字符串
    ```json
    {
        "stop_right": true    //true: 表示立即停车 false: 表示到下一个节点停车
    }
    ```
* **handler**(ResponseHandler)
    取消任务完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "angular_velocity": 0,
            "linear_velocity": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->cancel_task(args, handler);
if (!ret) {
    //失败处理
}
```

## resume_task

**函数签名**
```cpp
bool resume_task(ResponseHandler handler)
```

**描述**
恢复任务

**参数**
* **handler**(ResponseHandler)
    恢复任务完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "angular_velocity": 0,
            "linear_velocity": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->resume_task(handler);
if (!ret) {
    //失败处理
}
```

## pause_task

**函数签名**
```cpp
bool pause_task(ResponseHandler handler)
```

**描述**
暂停任务

**参数**
* **handler**(ResponseHandler)
    暂停任务完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "angular_velocity": 0,
            "linear_velocity": 0
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->pause_task(handler);
if (!ret) {
    //失败处理
}
```

## get_run_task

**函数签名**
```cpp
bool get_run_task(ResponseHandler handler)
```

**描述**
获取当前运行的任务信息

**参数**
* **handler**(ResponseHandler)
    获取当前运行任务完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "action_states": [
                {
                    "action_id": "4:B4wcAZ5hT-6khPB:238126",
                    "action_status": "WAITING",
                    "action_type": "pick"
                },
                {
                    "action_id": "4:B4wcAZ5hT-6khPB:238126",
                    "action_status": "WAITING",
                    "action_type": "drop"
                }
            ],
            "drop": [
                "LM2"
            ],
            "order_id": "3f685511-3dc1-453f-b624-77090e61c796",
            "pick": [
                "LM1"
            ]
        },
        "message": "success"
    }
    ```
    响应字段说明：
    | 字段名称       | 类型   | 含义                                                 |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败               |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 包含当前任务的信息                                 |
    | order_id       | string    | 当前执行的任务的id, 如果没有任务在执行，其值为空     |
    | drop           | json 数组 | 放货所在节点的名称                                 |
    | pick           | json 数组 | 取货所在节点的名称                                 |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->get_run_task(handler);
if (!ret) {
    //失败处理
}
```

## relocation

**函数签名**
```cpp
bool relocation(const std::string& args, ResponseHandler handler)
```

**描述**
重定位

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "x": 1.0,
        "y": 2.0,
        "direction": 2.1,    //弧度
        "length": 10,
        "map_id": "2d-1"    //期望重定位的地图id
    }
    ```
    请求参数字段说明: (按照仙工对应功能定义的，跟实际的后端接口需要的参数并不一样)
    | 字段名称       | 类型      | 含义                                                                 |
    |----------------|-----------|---------------------------------------------------------------------|
    | x              | double    | x 坐标，单位 m                                                             |
    | y              | double    | y 坐标，单位 m                                                             |
    | direction      | double    | 角度, 对应后端 theta, 范围 [-pi, pi]                                 |
    | map_id         | string    | 对应后端 mapId, 地图名称，如果跟小车当前使用的地图相同，执行重定位；反之执行地图切换      |

* **handler**(ResponseHandler)
    重定位完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //... 请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->relocation(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## set_log_level

**函数签名**
```cpp
bool set_log_level(const std::string& args, ResponseHandler handler);
```

**描述**
设置调试日志。

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "enable_log_point_cloud": true,
        "enable_log_agv_position": true,
        "log_level": "debug"
    }
    ```
    请求参数字段说明:
    | 字段名称      | 类型         | 含义                                                |
    |----------------------------|-------------|-----------------------------------------------------|
    | enable_log_point_cloud     | bool        | true: 表示启用获取点云数据日志，反之不记录点云日志      |
    | enable_log_agv_position    | bool        | true: 表示启用获取位置数据日志，反之不记录位置日志      |
    | log_level                  | string      | 日志级别(从高到低)，其取值为: trace, debug, info, warn, error, critical, off(关闭日志) ，默认日志级别是 info，也就是说默认情况下只会记录 info 级别及以下的日志。   |

    **如果请求参数传递的字符串为空: {}, 此时返回的是服务端当前的日志配置。**

* **handler**(ResponseHandler)
    设置完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "enable_log_agv_position": false,   //服务端当前记录位置日志的开关
            "enable_log_point_cloud": false,    //服务端当前记录点云日志的开关
            "log_level": "info"                 //服务端当前的日志级别
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->set_log_level(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## emergency_stop

**函数签名**
```cpp
bool Client::emergency_stop(const std::string& args, ResponseHandler handler)
```

**描述**
急停(软急停：不支持抱死急停，给一个0速度)

**参数**
* **args**
    json 格式字符串：
    ```json
    {
        "status": true  //true: 急停， false: 从急停恢复
    }
    ```
* **handler**(ResponseHandler)
    软急停完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": "",
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器；

**示例代码**
```cpp
bool ret = client_->emergency_stop(handler);
if (!ret) {
    //错误处理
}
```


## get_sysinfo

**函数签名**
```cpp
bool get_sysinfo(ResponseHandler handler)
```

**描述**
获取系统信息，包括软件版本信息和资源信息。

**参数**
* **handler**(ResponseHandler)
    获取系统信息完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "process_infos": [
                {
                    "cpu": 0.1,
                    "cswch": 79124,
                    "fds": 21,
                    "mem": 0.3,
                    "model_ver": "",
                    "name": "app",
                    "nvcswch": 2292,
                    "pid": 86,
                    "pss": 25.85,
                    "rss": 43.73,
                    "state": "S",
                    "sy": 22.69,
                    "threads": 21,
                    "us": 98.71,
                    "vss": 1440.72
                },
                {
                    "cpu": 0,
                    "cswch": 28334,
                    "fds": 19,
                    "mem": 0.5,
                    "model_ver": "",
                    "name": "amcl",
                    "nvcswch": 1117,
                    "pid": 3334,
                    "pss": 53.72,
                    "rss": 73.39,
                    "state": "S",
                    "sy": 2.3,
                    "threads": 12,
                    "us": 8.61,
                    "vss": 769.93
                },
                {
                    "cpu": 0.1,
                    "cswch": 9763,
                    "fds": 21,
                    "mem": 0.4,
                    "model_ver": "",
                    "name": "agv_task_manage",
                    "nvcswch": 429,
                    "pid": 3854,
                    "pss": 45.62,
                    "rss": 63.54,
                    "state": "S",
                    "sy": 0.66,
                    "threads": 14,
                    "us": 2.26,
                    "vss": 763.41
                },
                {
                    "cpu": 0,
                    "cswch": 118360,
                    "fds": 32,
                    "mem": 0.2,
                    "model_ver": "",
                    "name": "hal_process",
                    "nvcswch": 246,
                    "pid": 138,
                    "pss": 13.42,
                    "rss": 29.89,
                    "state": "S",
                    "sy": 29.2,
                    "threads": 19,
                    "us": 36.68,
                    "vss": 1211.48
                },
                {
                    "cpu": 0,
                    "cswch": 608,
                    "fds": 21,
                    "mem": 0.2,
                    "model_ver": "",
                    "name": "status_display",
                    "nvcswch": 33,
                    "pid": 169,
                    "pss": 11.81,
                    "rss": 28.92,
                    "state": "S",
                    "sy": 1.85,
                    "threads": 11,
                    "us": 1.72,
                    "vss": 636.34
                },
                {
                    "cpu": 0,
                    "cswch": 1447,
                    "fds": 20,
                    "mem": 0.2,
                    "model_ver": "",
                    "name": "ltme_node",
                    "nvcswch": 160,
                    "pid": 273,
                    "pss": 11.17,
                    "rss": 26.93,
                    "state": "S",
                    "sy": 1.75,
                    "threads": 11,
                    "us": 1.62,
                    "vss": 704.83
                }
            ],
            "resources": {
                "cpu": 4.9,
                "idle": 95.11,
                "mem": 14.6,
                "mem_unused": 13348.29,
                "mem_used": 2281.88,
                "swapd": 0.25,
                "temp": 36.95
            },
            "version": {
                "firmware_ver": "v2.1.6",
                "map_ver": "",
                "model_ver": "",
                "robot_id": "",
                "robot_name": "",
                "robot_type": "BYD_AGV",
                "sensor_ver": ""
            }
        },
        "message": "success"
    }
    ```
    响应字段说明：

    | 字段名称          | 类型          | 含义                                                                                       |
    |-------------------|---------------|--------------------------------------------------------------------------------------------|
    | code              | int           | 状态码，0 表示请求处理成功，非 0 表示失败                                                  |
    | message           | string        | 失败信息，例如参数为空                                                                    |
    | data              | json 对象     | 包含当前任务的信息                                                                        |
    | **resources**     | json 对象     | 资源对象                                                                                 |
    | cpu               | double        | CPU 使用率，系统总值，范围 0-100%                                                        |
    | idl               | double        | CPU 空闲态比例，范围 0-100%                                                              |
    | mem               | double        | 内存使用比例，范围 0-100%                                                                |
    | temp              | double        | CPU 温度，如果有多核，取均值，单位：摄氏度                                               |
    | swapd             | double        | 系统已用交换空间大小，单位：MB                                                           |
    | mem_used          | uint32        | 已使用物理内存大小，单位：MB                                                             |
    | mem_unused        | uint32        | 未使用物理内存大小，单位：MB                                                             |
    | **version**       | json 对象     | 版本对象                                                                                 |
    | robot_id          | string        | 机器人编号                                                                               |
    | robot_name        | string        | 机器人名称                                                                               |
    | robot_type        | string        | 机器人类型                                                                               |
    | firmware_ver      | string        | 固件版本                                                                                 |
    | sensor_ver        | string        | 传感器版本，哪个传感器？                                                                 |
    | map_ver           | string        | 地图版本                                                                                 |
    | model_ver         | string        | 模型版本                                                                                 |
    | **process_infos** | json 数组     | 进程列表，包含白名单进程的各种属性                                                       |
    | pid               | uint32        | 进程 id                                                                                 |
    | name              | string        | 进程名称                                                                                 |
    | cpu               | double        | 百分比，当前进程的 CPU 使用占比，范围可能超过 100%（多核系统 CPU 使用率可能超过 100%，与 TOP 命令结果保持一致） |
    | mem               | double        | 百分比，当前进程的物理内存使用占比，范围 0-100%                                          |
    | rss               | double        | 单位 MB，常驻内存集（Resident Set Size），表示进程占用的物理内存大小                      |
    | vss               | double        | 单位 MB，虚拟内存大小（Virtual Memory Size），表示进程的已分配虚拟地址空间大小            |
    | pss               | double        | 单位 MB，比例内存集（Proportional Set Size），考虑共享内存的比例后占用的内存大小          |
    | threads           | uint16        | 当前进程的线程数                                                                         |
    | fds               | uint16        | 当前进程打开的文件描述符数量                                                             |
    | cswch             | uint32        | 自愿上下文切换次数（主动让出 CPU）                                                       |
    | nvcswch           | uint32        | 非自愿上下文切换次数（被系统强制切换）                                                   |
    | model_ver         | string        | 软件模型版本号                                                                           |
    | state             | string        | 当前进程的状态（例如运行中、暂停等）                                                     |
    | us                | double        | 用户态 CPU 时间占比（百分比）                                                            |
    | sy                | double        | 内核态 CPU 时间占比（百分比）                                                            |



**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->get_sysinfo(handler);
if (!ret) {
    //失败处理
}
```

## pallet_rotation

**函数签名**
```cpp
bool pallet_rotation(const std::string& args, ResponseHandler handler);
```

**描述**
托盘旋转

**参数**
* **args**(std::string)
    json 格式的字符串，例如：
    ```json
    {
        "angle": 1,    //旋转的角度，单位是弧度, 范围 [-2PI, 2PI]
        "mode": 0       // 0:增量式, 1:机器人坐标系（绝对） 默认值是 1
    }
    ```
    请求参数字段说明:
    | 字段名称 | 类型    | 含义                                            |
    |----------|---------|-------------------------------------------------|
    | angle    | double  | 旋转角度                                        |
    | mode     | double  | 0: 增量式, 1: 绝对，默认绝对模式                 |

* **handler**(ResponseHandler)
    托盘旋转操作完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            //...   请求数据原样返回
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 启动任务失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool = client_->pallet_rotation(request_argument, handler);
if (!ret) {
    //失败处理
}
```

## get_model_file

**函数签名**
```cpp
bool get_model_file(const std::string& file_name, ResponseHandler handler);
```

**描述**
获取模型文件。

**参数**
* **file_name**(std::string)
    要获取的模型文件名称，例如: agv_model_para.json, 如果要获取的模型文件名称就是 agv_model_para.json, file_name 可以设置为空字符串。

* **handler**(ResponseHandler)
    获取模型文件完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "filename": "./model/agv_model_para.json"       //拉取的模型文件存放在程序当前 model 目录下(跟地图的 map 目录平级)
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如，未提供 handler, 未连接服务器，当前正在获取文件


**示例代码**
```cpp
//入参 file_name="", 默认获取 agv_model_para.json
bool ret = client_->get_model_file("", handler);
if (!ret) {
    //出错处理
}
```

## get_3dcamera_pointcloud

**函数签名**
```cpp
bool Client::get_3dcamera_pointcloud(ResponseHandler handler)
```

**描述**
获取小车3d摄像头点云数据
调用该函数后，client 库中会启动一个定时器，定期获取小车点云。如果要停止定时器，调用 cancel_get_3dcamera_pointcloud()，重新获取点云数据，再次调用get_3dcamera_pointcloud 就行。
**注意**: 在其中一台小车测试，请求达到服务端，到客户端收到完整响应，一次请求的数据量大约20万个点，压缩后大小 600KB, 耗时 400ms左右，服务端从收到请求到发送完成耗时 60-70ms, 网络传输耗时 200-300ms, 客户端显示耗时 10ms 左右。所以，客户端更新点云的频率大概 3hz

**参数**
* **handler**(ResponseHandler)
    当获取小车3d点云信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string& reply)>
    ```
    其参数为操作的结果。【注意】，该api的返回跟其他api有一点不同，如果请求失败，返回 json 格式的字符串，例如:
    ```json
    {
        "code": 23,
        "message": "convert PointCloud2 failed"
    }
    ```
    如果请求成功，返回的是二进制的字节序列(为了减少数据量；to json 对象，json 对象 to string 跟序列化和压缩解压缩相比，耗时更多)，此时，客户端点的典型处理：
    ```cpp
    json obj = json::parse(repley);
    if parse failed
        //说明请求成功了，进行以下处理：
        //1. 反序列化
        CompressionBounds bounds;
        std::vector<CompressedPoint3D> compressed_points;
        deserialize_from_byte_array(reply, bounds, compressed_points);

        //2. 解压缩
        std::vector<Point3D> decompressed_points = PointCloudCompressor::decompress_point_cloud(compressed_points, bounds);

        //3. 显示点云
        //show(decompressed_points)

        //压缩，解压缩定义在工具类头文件 point_cloud_compression.h
    else
        //说明请求失败了，obj["message"] 指示了出错的原因。
    ```


**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_3dcamera_pointcloud(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_3dcamera_pointcloud

**函数签名**
```cpp
void cancel_get_3dcamera_pointcloud()
```

**描述**
取消获取3d摄像头点云信息。

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_3dcamera_pointcloud();
```

## get_camera_video_list

**函数签名**
```cpp
bool get_camera_video_list(ResponseHandler handler);
```

**描述**
获取3d摄像头视频文件列表

**参数**
* **handler**(ResponseHandler)
    获取列表连接完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            "recording_0baef1f4-c006-4ca4-a634-54c33ae54244_20250506-135843-262.avi",
            "xxx.zip",
            "recording_bc658d04-f290-4da4-8461-6fb21b716e98_20250506-090521-357.avi",
            "recording_d38c5632-8cba-46e6-89e7-12c4d849ff61_20250506-090040-271.avi",
            "recording_ccc376c7-0bf3-4bc7-9bcb-6e033f27e38f_20250506-112418-849.avi",
            "recording_ed76ce4c-f89f-4e72-a7b0-a2fee042aa16_20250506-110405-819.avi",
            "recording_de74e40f-ed46-4e45-a9ac-8f74f61939bc_20250506-091026-015.avi"
        ],
        "data_size": 7,
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器


**示例代码**
```cpp
bool ret = client_->get_camera_video_list(handler);
if (!ret) {
    //错误处理
}
```

## get_camera_video

**函数签名**
```cpp
bool get_camera_video(const std::string& args, ResponseHandler handler);
```

**描述**
获取3d摄像头视频文件

**参数**
* **args**(std::string)
    json 格式的字符串，例如:
    ```json
    {
        "host": "192.168.192.124",
        "port": "22",
        "username": "登录小车的用户名称",
        "password": "登录小车的用户名称对应的密码",
        "remote_file": "3d-camera-6991.json"
    }
    ```
    请求参数字段说明:
    | 字段名称      | 类型         | 含义                                                                                                 |
    |---------------|--------------|------------------------------------------------------------------------------------------------------|
    | host          | string       | agv 小车的 ip ，必传字段                                                                              |
    | port          | string       | agv 小车端口，可选字段，如果不传，默认是 22                                                              |
    | username      | string       | 登录小车的用户名称 ，必传字段                                                                         |
    | password      | string       | 登录小车的用户名称对应的密码 ，必传字段                                                                |
    | remote_file   | string       | 要获取的视频文件名称，例如: recording_0baef1f4-c006-4ca4-a634-54c33ae54244_20250506-135843-262.avi (获取文件名称的典型方法是先调用 get_camera_video_list 获取服务器上的视频文件列表，然后选中一个文件，就获取了文件名称。必传字段   |

* **handler**(ResponseHandler)
    获取视频文件完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "filename": "./video/xxx.zip"                   // 拉取的视频存放在程序当前 video 目录下(跟地图的 map 目录平级)
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如， file_name 为空，未提供 handler, 未连接服务器，当前正在获取文件


**示例代码**
```cpp
bool ret = client_->get_camera_video(args, handler);
if (!ret) {
    //出错处理
}
```

## get_errors

**函数签名**
```cpp
bool get_errors(ResponseHandler handler);
```

**描述**
获取错误信息

**参数**
* **handler**(ResponseHandler)
    获取错误信息完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            {
                "error_description": "Bad Communication to Motor",
                "error_hint": "",
                "error_level": "FATAL",
                "error_type": "55100002"
            },
            {
                "error_description": "[33500003][WARNING] Serial port:/dev/ttyUSB0 is not open!",
                "error_hint": "",
                "error_level": "WARNING",
                "error_type": "33500003"
            },
            {
                "error_description": "无法获取到定位",
                "error_hint": "",
                "error_level": "FATAL",
                "error_type": "77300002"
            },
            {
                "error_description": "电池电量低，请及时充电",
                "error_hint": "",
                "error_level": "WARNING",
                "error_type": "33300001"
            },
            {
                "error_description": "定位匹配分数低!",
                "error_hint": "",
                "error_level": "WARNING",
                "error_type": "33300002"
            },
            {
                "error_description": "[33500006][WARNING] statusCode:8193 is not in module:4",
                "error_hint": "",
                "error_level": "WARNING",
                "error_type": "33500006"
            }
        ],
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称            | 类型   | 含义                                                |
    |--------------------|--------|-----------------------------------------------------|
    | code               | int      | 状态码，0表示请求处理成功，非0表示失败               |
    | message            | string   | 失败信息，例如参数为空                              |
    | data               | json 数组| 响应数据                                           |
    | error_type         | string   | 错误类型编码（在数据库中作为错误类型唯一编码）        |
    | error_level        | string   | 错误级别枚举{WARNING,FATAL}<br>WARNING:不影响AGV执行任务的错误。<br>FATAL:如果不消除错误，AGV完全无法执行任务。  |
    | error_hint         | string   | 错误提示，没有时为空                               |
    | error_description  | string   | 错误描述，可省略(如果因货物检测报错，上报任务下发的编码和实际检测到的二维码，例：任务编码：xxx,检测货物编码：xxy) |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_errors(handler);
if (!ret) {
    //失败处理
}
```

## set_do

**函数签名**
```cpp
bool set_do(const std::string& args, ResponseHandler handler);
```

**描述**
设置io模块输出

**参数**
* **args**(std::string)
    json 格式的字符串，例如:
    ```json
    {
        "index": 2,
        "value": 1
    }
    ```
    请求参数字段说明:
    | 字段名称      | 类型         | 含义                                                                                                 |
    |---------------|--------------|------------------------------------------------------------------------------------------------------|
    | index          | int       | io 字段编号，取值范围 [0, 23]                                                                          |
    | value          | int       | 值，0 or 1                                                                                            |

* **handler**(ResponseHandler)
    设置io模块输出完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "filename": "./video/xxx.zip"                   // 拉取的视频存放在程序当前 video 目录下(跟地图的 map 目录平级)
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 表示发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 表示发起异步操作失败，例如， file_name 为空，未提供 handler, 未连接服务器，当前正在获取文件


**示例代码**
```cpp
bool ret = client_->set_do(args, handler);
if (!ret) {
    //出错处理
}
```

## get_sysinfo_periodic

**函数签名**
```cpp
bool get_sysinfo_periodic(ResponseHandler handler);
```

**描述**
**定期**获取小车系统信息。
调用该函数后，client 库中会启动一个定时器，定期获取小车系统信息。如果要停止定时器，调用 cancel_get_sysinfo()。重新获取系统数据，再次调用 get_sysinfo_periodic 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小系统信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    **响应字段说明见api: get_sysinfo()**

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_sysinfo_periodic(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_sysinfo

**函数签名**
```cpp
void cancel_get_sysinfo();
```

**描述**
取消获取小车系统信息。

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_sysinfo();
```

## clear_errors

**函数签名**
```cpp
bool clear_errors(ResponseHandler handler);
```

**描述**
清除错误信息。(直到底层再次发送 errors, 否则 get_errors() 返回的错误为空)

**参数**
* **handler**(ResponseHandler)
    清除错误信息完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": "",
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->clear_errors(handler);
if (!ret) {
    //失败处理
}
```

## get_processes_info

**函数签名**
```cpp
bool get_processes_info(const std::string& args, ResponseHandler handler);
```

**描述**
获取进程信息。

**参数**
* **args**(std::string)
    json格式的字符串，例如:
    ```json
    {
        "processes": [
            "agv_server_pubsub_node",
            "agv_task_manager"
        ]
    }
    ```
* **handler**(ResponseHandler)
    获取进程信息完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            {
                "CPU(s)": 34.51,
                "Mem(KB)": 16044,
                "Name": "agv_task_manager",
                "PID": 136035,
                "PPID": 136032,
                "Running(s)": "986.13"
            },
            {
                "CPU(s)": 17.09,
                "Mem(KB)": 26288,
                "Name": "agv_server_pubsub_node",
                "PID": 142939,
                "PPID": 132183,
                "Running(s)": "741.95"
            }
        ],
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_processes_info(args, handler);
if (!ret) {
    //失败处理
}
```

## check_fd_keep_alive

**函数签名**
```cpp
bool check_fd_keep_alive(const std::string& fd, ResponseHandler handler);
```

**描述**
检查socket fd 的keep alive 配置

**参数**
* **args**(std::string)
    套接字 fd, 例如： "21"
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            "TCP_KEEPIDLE (空闲时间，秒): 60",
            "TCP_KEEPINTVL (探测间隔，秒): 15",
            "TCP_KEEPCNT (探测次数/tcp_keepalive_probes): 3"
        ],
        "message": "SO_KEEPALIVE: 启用"
    }
    ```
    or
    ```json
    {
        "code": 0,
        "data": [],
        "message": "SO_KEEPALIVE: 禁用"
    }
    ```
    or
    ```json
    {
        "code": 1,
        "data": [],
        "message": "invalid socket fd"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->check_fd_keep_alive("21", handler);
if (!ret) {
    //失败处理
}
```

## get_plc_digital_io

**函数签名**
```cpp
bool get_plc_digital_io(ResponseHandler handler);
```

**描述**
获取跟plc有关的 digital io 。
该接口获取的dido是跟plc交互设置的dido，只有少量几个位。get_mcu2pc() 返回的是从微控制器获取的dido(跟冯工对接)。

**参数**
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "dis": [
                {
                    "id": 5,
                    "value": 0
                },
                {
                    "id": 6,
                    "value": 1
                }
            ],
            "dos": [
                {
                    "id": 2,
                    "value": 1
                }
            ]
        },
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                          |
    | dis           | json 数组      | digital input                                |
    | dos           | json 数组     | digital output                                |
    | id            | int     | di/do 口的序号                                      |
    | value         | int     | di/do 口的值, 0 or 1                                |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_plc_digital_io(handler);
if (!ret) {
    //失败处理
}
```

## get_wifi_list

**函数签名**
```cpp
bool get_wifi_list(ResponseHandler handler);
```

**描述**
获取wifi列表

**参数**
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            {
                "frequency": 2437,
                "in_use": false,
                "rate": 130,
                "security": "WPA2 WPA3",
                "signal_strength": 94,
                "ssid": "OPPO Reno13 f5fr"
            },
            {
                "frequency": 5805,
                "in_use": false,
                "rate": 130,
                "security": "WPA2",
                "signal_strength": 80,
                "ssid": "byd-agv"
            },
            {
                "frequency": 5805,
                "in_use": false,
                "rate": 130,
                "security": "WPA2",
                "signal_strength": 80,
                "ssid": "byd-prod"
            },
            {
                "frequency": 5200,
                "in_use": false,
                "rate": 270,
                "security": "WPA1 WPA2",
                "signal_strength": 79,
                "ssid": "AXIS"
            },
            {
                "frequency": 2422,
                "in_use": true,
                "rate": 270,
                "security": "WPA1 WPA2",
                "signal_strength": 79,
                "ssid": "Xiaomi_4D7B"
            },
            {
                "frequency": 5785,
                "in_use": true,
                "rate": 270,
                "security": "WPA1 WPA2",
                "signal_strength": 79,
                "ssid": "Xiaomi_4D7B_5G"
            }
        ],
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                          |
    | frequency      | int      | Wi-Fi 使用的无线电频率信道: 2.4GHz 或 5GHz     |
    | in_use         | bool     | 是否正在使用                                     |
    | rate           | int     | 速率, 单位 Mbit/s                                  |
    | security       | string     | 网络使用的加密方式                             |
    | signal_strength  | int     | 信号强度百分比, 最高 100                    |
    | ssid          | string     | wifi 名称                                         |


**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_wifi_list(handler);
if (!ret) {
    //失败处理
}
```

## set_wifi_config

**函数签名**
```cpp
bool set_wifi_config(const std::string& args, ResponseHandler handler);
```

**描述**
设置 wifi 配置

**参数**
* **args**
    json 格式的字符串，表示要设置的 wifi 配置信息，例如：
    ```json
    {
        "ssid": "Xiaomi_4D7B",
        "password": "12345678",
        "ip_address": "192.168.10.91/24",
        "gateway": "192.168.10.1",
        "dns": "8.8.8.8",
        "interface": "wlp3s0"
    }
    ```
    请求参数字段说明:

    | 字段名称      | 类型   | 含义                                                |
    |---------------|--------|-----------------------------------------------------|
    | ssid          | string | Service Set Identifier - 这是我们通常所说的 "Wi-Fi名字"，是人类可读的网络标识          |
    | password      | string | wifi 密码       |
    | ip_address    | string    | ip 地址，带掩码，例如: 1.2.3.4/24    |
    | gateway       | string | 网关           |
    | dns           | string | dns 服务器        |
    | interface     | string    | 网络接口名称，需要以 wl 开头: wireless(无线)的缩写  |

* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 34,
        "data": {
            "output": "--> 开始配置网络...\n    连接名称: mFi_253459\n    Wi-Fi SSID: mFi_253459\n    IP 地址: 192.168.192.91/24\n    网关: 192.168.192.1\n    DNS: 8.8.8.8\n    接口: wlo1\n--> 已清理旧的连接配置（如果存在）。\n--> 正在连接到 Wi-Fi: mFi_253459...\nError: Connection activation failed: (7) Secrets were required, but not provided.\n--> Wi-Fi 连接命令已发送！\n--> 正在配置静态 IP 地址...\n--> 静态 IP 配置成功！\n--> 正在重新激活连接以应用更改...\nPasswords or encryption keys are required to access the wireless network 'mFi_253459'.\n!!! 错误：重新激活连接失败。\n",
            "steps": [
                "--> 开始配置网络...",
                "--> 已清理旧的连接配置（如果存在）。",
                "--> 正在连接到 Wi-Fi: mFi_253459...",
                "--> Wi-Fi 连接命令已发送！",
                "--> 正在配置静态 IP 地址...",
                "--> 静态 IP 配置成功！",
                "--> 正在重新激活连接以应用更改...",
                "!!! 错误：重新激活连接失败。"
            ]
        },
        "message": "WiFi configuration failed"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                          |
    | output         | string      | 配置过程中的输出信息                            |
    | steps          | json 数组     | 配置过程中的步骤信息                           |
    | steps[0]     · | 数组元素     |  某个具体的步骤信息                         |

    **注意：设置成功后，gui 会断开连接，因为小车的ip变了**

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->set_wifi_config(args, handler);
if (!ret) {
    //失败处理
}
```

## get_network_interface

**函数签名**
```cpp
bool get_network_interface(ResponseHandler handler);
```

**描述**
获取网络接口名称

**参数**
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": [
            {
                "dns_servers": [],
                "gateway": "",
                "ip_address": "172.17.0.1",
                "ip_config_type": "static",
                "mac_address": "52:74:00:fe:7a:79",
                "name": "docker0",
                "netmask": "255.255.0.0",
                "status": "up"
            },
            {
                "dns_servers": [],
                "gateway": "",
                "ip_address": "192.168.2.100",
                "ip_config_type": "static",
                "mac_address": "68:ed:a4:76:22:84",
                "name": "eno1",
                "netmask": "255.255.255.0",
                "status": "active"
            },
            {
                "dns_servers": [],
                "gateway": "",
                "ip_address": "192.168.1.100",
                "ip_config_type": "static",
                "mac_address": "68:ed:a4:76:d5:b0",
                "name": "enp4s0",
                "netmask": "255.255.255.0",
                "status": "active"
            },
            {
                "dns_servers": [],
                "gateway": "",
                "ip_address": "",
                "ip_config_type": "no-ip",
                "mac_address": "68:ed:a4:76:d5:b1",
                "name": "enp5s0",
                "netmask": "",
                "status": "up"
            },
            {
                "dns_servers": [],
                "gateway": "",
                "ip_address": "",
                "ip_config_type": "no-ip",
                "mac_address": "68:ed:a4:76:22:85",
                "name": "enp7s0",
                "netmask": "",
                "status": "up"
            },
            {
                "dns_servers": [],
                "gateway": "192.168.10.1",
                "ip_address": "192.168.10.95",
                "ip_config_type": "static",
                "mac_address": "90:10:57:98:46:cb",
                "name": "wlp3s0",
                "netmask": "255.255.255.0",
                "status": "active"
            }
        ],
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 数组 | 响应数据                                          |
    | data[0]        | json 对象     | 表示一个网络接口对象，包含的信息如下：           |
    | name           | string     | 网络接口名称，例如，以太网接口： enp7s0, 无线网口: wlp3s0  |
    | ip_address      | string      | ip 地址，如果网口没有配置ip地址，为空字符串    |
    | netmask         | string     | 子网掩码                                     |
    | gateway         | string     | 网关                                  |
    | dns_servers     | 数组     | dns服务器的ip                             |
    | ip_config_type  | string     | ip配置类型: dhcp, static                  |
    | status       | string     | 网口状态: up - 启用了网卡, active - 启用了网卡且网口插线，可以收发数据 |


**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_network_interface(handler);
if (!ret) {
    //失败处理
}
```

## get_execution_queue

**函数签名**
```cpp
bool get_execution_queue(ResponseHandler handler);
```

**描述**
获取执行(任务，动作)队列

**参数**
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "current_execution_index": 1,
            "execution_queue": [
                {
                    "action": "set_do6",
                    "name": "AGV到达离开位-1753249407912692343",
                    "task": "",
                    "type": 1
                },
                {
                    "action": "wait_di5",
                    "name": "允许AGV进入-1753249407912705483",
                    "task": "",
                    "type": 1
                },
                {
                    "action": "",
                    "name": "1f2db7a4-5ed7-4cb9-9ac4-fa788d1267f6",
                    "task": "LM1 -> LM0 -> ",
                    "type": 0
                },
                {
                    "action": "set_do3",
                    "name": "AGV到达目标位-1753249407912747976",
                    "task": "",
                    "type": 1
                },
                {
                    "action": "wait_di8",
                    "name": "允许AGV离开-1753249407912756826",
                    "task": "",
                    "type": 1
                },
                {
                    "action": "",
                    "name": "a28b7634-2a20-4551-b267-00414eb55db9",
                    "task": "LM0 -> LM1 -> ",
                    "type": 0
                }
            ]
        },
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                          |
    | current_execution_index   | string     | 当前执行索引：基于 execution_queue 的索引  |
    | execution_queue           | json 数组      | 执行队列   |
    | execution_queue[0]        | string     | 执行队列元素                              |
    | action                    | string     | 动作名称，如果是任务，为空                 |
    | name                      | string     | 对于动作:动作名称，对于任务：order_id      |
    | task                      | string     | 对于动作：空字符串，对于任务：节点路径信息      |
    | type                      | string     | 类型：任务-0，动作-1 |


**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_execution_queue(handler);
if (!ret) {
    //失败处理
}
```

## get_qr_camera_data

**函数签名**
```cpp
bool get_qr_camera_data(ResponseHandler handler);
```

**描述**
获取相机扫码数据

**参数**
* **handler**(ResponseHandler)
    请求完成或出错时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "pos": {
                "angle": 1.54810704651897,
                "is_matrix": true,
                "stamp": {
                    "nanosec": 571447126,
                    "sec": 1756273402
                },
                "tag_number": 297441598587591,
                "x": -0.0011,
                "y": -0.0338
            },
            "rack": {
                "angle": 0,
                "is_matrix": false,
                "stamp": {
                    "nanosec": 0,
                    "sec": 0
                },
                "tag_number": 0,
                "x": 0,
                "y": 0
            }
        },
        "message": "success"
    }
    ```
    响应字段说明:
    | 字段名称       | 类型   | 含义                                                |
    |----------------|--------|-----------------------------------------------------|
    | code           | int      | 状态码，0表示请求处理成功，非0表示失败              |
    | message        | string   | 失败信息，例如参数为空                              |
    | data           | json 对象 | 响应数据                                          |
    | pos           | json 对象     | 下扫码相机的数据  |
    | stamp         | json 对象     | ROS 2 中用于表示时间点的标准消息类型                              |
    | stamp.sec     | int32     | 表示自 Epoch (通常是 1970-01-01 00:00:00 UTC) 以来的总秒数。                 |
    | stamp.nanosec | uint32     | 表示在当前秒数（sec 字段）之后经过的纳秒数。它的范围是 0 到 999,999,999。      |
    | x             | double     | x坐标，像素坐标。坐标原点是二维码的中心     |
    | y             | double     | y坐标，像素坐标。坐标原点是二维码的中心|
    | angle         | double     | 代表检测到的二维码的角度或方向。单位通常是弧度（radians）。这可以表示二维码相对于坐标系y轴的旋转角度。      |
    | tag_number    | uint64     | 代表从二维码中解码出的标签ID或数字。每个二维码可以编码一个独特的数字，用于识别物体。      |
    | is_matrix     | bool     | 这是一个标志位。根据名称 is_matrix 推断，它可能用来区分检测到的码是标准的 QR 码还是另一种矩阵码（如 Data Matrix）。或者它可能有其他特定的业务含义，例如表示标签是否属于一个矩阵阵列。      |
    | rack           | json 对象     | 上扫码相机的数据，结构同 pos |

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_qr_camera_data(handler);
if (!ret) {
    //失败处理
}
```

## get_scan2pointcloud

**函数签名**
```cpp
bool get_scan2pointcloud(ResponseHandler handler);
```

**描述**
获取小车避障点云
调用该函数后，client 库中会启动一个定时器，定期获取小车位置。如果要停止定时器，调用 cancel_get_scan2pointcloud()。重新获取点云数据，再次调用 get_scan2pointcloud 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车避障点云完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "clipping_radius": 5,                   // 裁剪半径，单位米，该范围外的点丢掉
        "code": 0,
        "data": {
            "frame_id": "base_link",            // 表示此多边形数据所在的坐标系的名称（例如 `"map"`, `"base_link"`, `"odom"`）
            "points": [
                {
                    "i": 0,
                    "x": -2.547891616821289,    // 单位米
                    "y": -4.289276123046875     // 单位米
                },
                {
                    "i": 0,
                    "x": -2.523392677307129,
                    "y": -4.278434753417969
                }
                //...
            ]
            },
            "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_scan2pointcloud(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_scan2pointcloud

**函数签名**
```cpp
void cancel_get_scan2pointcloud();
```

**描述**
取消获取小车避障点云

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_scan2pointcloud();
```

## get_obst_polygon

**函数签名**
```cpp
bool get_obst_polygon(ResponseHandler handler);
```

**描述**
获取小车避障轮廓。**注意：小车移动的时候才有数据**
调用该函数后，client 库中会启动一个定时器，定期获取小车避障轮廓。如果要停止定时器，调用 cancel_get_obst_polygon()。重新获取点云数据，再次调用 get_obst_polygon 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车避障轮廓信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "header": {
                "frame_id": "base_link",        // 表示此多边形数据所在的坐标系的名称（例如 `"map"`, `"base_link"`, `"odom"`）
                "nanosec": 669895844,           // 自 Unix 纪元（1970-01-01 00:00:00 UTC）以来的秒数。
                "sec": 1755764121               // 在 `sec` 基础上增加的纳秒数，用于更高精度的时间表示。
            },
            "polygon": {
                "points": [
                    {
                        "x": -1.024999976158142,        // 顶点在 `frame_id` 坐标系下的 x 坐标，// 单位米
                        "y": 0.4099999964237213,        // 顶点在 `frame_id` 坐标系下的 y 坐标。// 单位米
                        "z": 0                          // 顶点在 `frame_id` 坐标系下的 Z 坐标。// 单位米
                    },
                    {
                        "x": -1.024999976158142,
                        "y": -0.4099999964237213,
                        "z": 0
                    },
                    {
                        "x": -0.574999988079071,
                        "y": -0.4099999964237213,
                        "z": 0
                    },
                    {
                        "x": -0.574999988079071,
                        "y": 0.4099999964237213,
                        "z": 0
                    }
                ]
            }
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_obst_polygon(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_obst_polygon

**函数签名**
```cpp
void cancel_get_obst_polygon();
```

**描述**
取消获取小车避障轮廓

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_obst_polygon();
```

## get_obst_pcl

**函数签名**
```cpp
bool get_obst_pcl(ResponseHandler handler);
```

**描述**
获取小车障碍物点云**注意：小车避障的时候才有数据**
调用该函数后，client 库中会启动一个定时器，定期获取小车障碍物点云。如果要停止定时器，调用 cancel_get_obst_pcl()。重新获取点云数据，再次调用 get_obst_pcl 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车障碍物点云信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "frame_id": "base_link",            // 表示此多边形数据所在的坐标系的名称（例如 `"map"`, `"base_link"`, `"odom"`）
            "points": [
                {
                    "x": 0.874441385269165,     // 单位米
                    "y": 0.17772239446640015    // 单位米
                },
                {
                    "x": 0.8694524168968201,
                    "y": 0.17658254504203796
                },
                {
                    "x": 0.872018575668335,
                    "y": 0.18508648872375488
                },
                {
                    "x": 0.8638087511062622,
                    "y": 0.18587394058704376
                },
                {
                    "x": 0.871347188949585,
                    "y": 0.19091041386127472
                },
                {
                    "x": 0.8671715259552002,
                    "y": 0.19248218834400177
                },
                {
                    "x": 0.8688160181045532,
                    "y": 0.19582027196884155
                },
                {
                    "x": 0.8730858564376831,
                    "y": 0.20058690011501312
                }
            ]
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_obst_pcl(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_obst_pcl

**函数签名**
```cpp
void cancel_get_obst_pcl();
```

**描述**
取消获取小车障碍物点云

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_obst_pcl();
```

## get_model_polygon

**函数签名**
```cpp
bool get_model_polygon(ResponseHandler handler);
```

**描述**
获取小车模型轮廓。
调用该函数后，client 库中会启动一个定时器，定期获取小车模型轮廓。如果要停止定时器，调用 cancel_get_model_polygon()。重新获取模型轮廓数据，再次调用 get_model_polygon 就行。

**参数**
* **handler**(ResponseHandler)
    当获取小车模型轮廓信息完成或发生错误时调用的回调函数。其签名为：
    ```cpp
    using ResponseHandler = std::function<void(const std::string&)>
    ```
    其参数为操作的结果。json 格式的字符串，例如:
    ```json
    {
        "code": 0,
        "data": {
            "header": {
                "frame_id": "map",                      // 表示此多边形数据所在的坐标系的名称（例如 `"map"`, `"base_link"`, `"odom"`）
                "nanosec": 742755347,                   // 自 Unix 纪元（1970-01-01 00:00:00 UTC）以来的秒数。
                "sec": 1756190683                       // 在 `sec` 基础上增加的纳秒数，用于更高精度的时间表示。
            },
            "polygon": {
                "points": [
                    {
                        "x": -1.2287168136786537,       // 顶点在 `frame_id` 坐标系下的 x 坐标，// 单位米
                        "y": -0.3162647843895636,       // 顶点在 `frame_id` 坐标系下的 y 坐标。// 单位米
                        "z": 0                          // 顶点在 `frame_id` 坐标系下的 Z 坐标。// 单位米
                    },
                    {
                        "x": -0.4206435910414771,
                        "y": -0.1769174695549689,
                        "z": 0
                    },
                    {
                        "x": -0.2252174249838904,
                        "y": -1.3101908326683722,
                        "z": 0
                    },
                    {
                        "x": -1.033290647621067,
                        "y": -1.449538147502967,
                        "z": 0
                    }
                ]
            }
        },
        "message": "success"
    }
    ```

**返回值**
* true:
    * 发起异步操作成功，实际结果通过传递给回调函数的参数判断
* false:
    * 操作失败，例如，传入的回调函数为空；没有请求名称对应的命令字配置；未连接到服务器

**示例代码**
```cpp
bool ret = client_->get_model_polygon(handler);
if (!ret) {
    //错误处理
}
```

## cancel_get_model_polygon

**函数签名**
```cpp
void cancel_get_model_polygon();
```

**描述**
取消获取小车模型轮廓

**参数**
无

**返回值**
无

**示例代码**
```cpp
client_->cancel_get_model_polygon();
```

*************************************

# 日志输出

qclcpp 库中会打印日志到终端，有4个日志级别，级别从低到高分别为：

* error
* warn
* info
* debug

为了控制输出日志的数量，默认日志级别是最低的 error, 如果要输出 info 日志，那么日志级别要 >= LOG_LEVEL_INFO, 对于 QT 应用，在 pro 文件中添加以下配置:
```sh
# 添加 LOG_LEVEL_XXX 宏定义
DEFINES += LOG_LEVEL_INFO
```

**************************************


# 使用 api

1. 构造 qclcpp::Client 实例
2. 调用具体的api发送请求
3. 运行

## 命令行程序中使用 api

```cpp
#include "client.h"
#include <asio.hpp>
#include <iostream>
#include <string>
#include <memory>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace qclcpp;

void connect_callback(bool success)
{
    if (success) {
        std::cout << "connect success\n";
    } else {
        std::cout << "connect failed\n";
    }
}

void get_velocity_handler(const std::string& reply)
{
    // 解析请求响应
    // 响应数据参考前面的 api 说明
    json jreply = json::parse(reply, nullptr, false);
    if (jreply.is_discarded() || !jreply.is_object()) {
        std::cerr << "parse repley fail\n";
        return;
    }

    if (jreply.value("code", 1)) {
        // 请求失败
    } else {
        // 响应数据处理
        // jreply["data"].value("angular_velocity", 0.0);
        // jreply["data"].value("linear_velocity", 0.0);
    }
}

void get_log_file_handler(const std::string& reply)
{
    std::cout << "[" << __func__ << ":" << __LINE__ << "] " << reply << "\n";
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "usage: client <host_ip> <host_port>\n";
        return 1;
    }

    // 构造 rclcpp::Client 实例
    asio::io_context io_context;
    auto client = Client::create(io_context);

    // 连接到服务器
    client->connect(argv[1], argv[2], connect_callback);

    // 调用 api

    // 获取日志文件
    bool ret = client->get_log_file("agv_server_pubsub_node/agvcomm.log", get_log_file_handler);
    if (!ret) {
        std::cout << "get log file fail\n";
    }

    // 获取速度
    ret = client->get_velocity(get_velocity_handler);
    if (!ret) {
        std::cout << "get log file fail\n";
    }

    // 运行 io 服务 (开启事件循环)
    // run 会阻塞当前线程，直到所有的处理程序都被调度
    io_context.run();

    std::cout << "main end\n";

    return 0;
}
```

## qt 中使用 api

在 qt 应用中使用client库时，需要注意asio::io_context::run()的调用时机。asio::io_context::run()会阻塞当前线程，直到所有异步操作完成。gui操作不能被阻塞，因此，启动一个新的线程来运行 io_context.run(), 而为了防止此时没有异步任务导致的 run() 的返回，我们可以使用 asio::io_context::work 来保持 io_context 的运行状态。
asio::io_context::work 是一个辅助类，它会向 io_context 中添加一个虚拟的异步任务，只要这个 work 对象存在，io_context::run() 就不会返回，即使没有其他异步任务需要处理。

```cpp
//...

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client_(qclcpp::Client::create(io_context_))  // 构造一个 qclcpp::Client 实例
{
    ui->setupUi(this);
    std::thread t([this]() {
        // 创建一个 work 对象，用于保持 io_context 运行
        asio::io_context::work work(this->io_context_);

        // 启动事件循环
        this->io_context_.run();
        });
    t.detach();
}

// 然后在对应的槽函数中调用api

// 读取文本框输入的ip和port，connect 到服务器
void MainWindow::on_pushButton_start_clicked()
{
    // 读取 lineEdit_ip 和 lineEdit_port 的输入并赋值给两个 QString 变量
    QString ip = ui->lineEdit_ip->text();
    QString portStr = ui->lineEdit_port->text();

    qDebug()<< "ip="<< ip << ", port=" << portStr << "\n";
    client_->connect(ip.toStdString(), portStr.toStdString(), connect_callback);
}

// 发送请求
void MainWindow::on_pushButton_send1_clicked()
{
    QString strText = ui->plainTextEdit_send1->toPlainText();

    bool ret = client->get_log_file(strText.toStdString(), get_log_file_handler);
    if (!ret) {
        std::cout << "get log file fail\n";
    }
}
```

# api 设计和实现

qclcpp 设计用于给qt客户端使用，gui操作不能阻塞，一次gui操作后，无需等待操作完成，就可以继续其他gui操作。因此 qclcpp 提供的api必须是无阻塞的。qclcpp 用到了 asio 库。

## asio 简介

asio 是一个跨平台的c++库，专注于网络和底层io编程。他提供了丰富的功能来执行异步操作，如异步读写，定时器等。

asio 有两种变种，non-boost asio 和 boost.asio，我们使用 non-boost asio 。

**核心概念**

1. I/O 服务(io_context)
    它是asio的核心类之一，负责管理所有的io操作。你可以把她想象成一个事件循环或调度器，他会处理所有异步操作的完成通知。

2. 异步操作(async operation)
    asio 提供了一系列的异步操作函数，如 async_read、async_write、async_connect 等。这些函数会立即返回，而不会阻塞当前线程，操作的实际执行会在后台进行，并在操作完成后通过回调函数或其他机制通知用户代码。

3. 句柄对象(handler)
    当异步操作完成时，asio需要一种方式来通知你的程序。这通常通过你提供的回调函数或者 "句柄" 来实现。这个句柄是一个可调用对象(函数，lambda, 函数对象)，当异步操作完成时会被调用。它通常接受操作的结果作为参数，例如错误码和传输的字节数。

**使用异步操作流程**

1. 初始化 io 服务
    首先需要创建一个 io_context 对象

2. 发起异步操作
    然后，使用 asio 库提供的api发起异步操作，例如， async_connect, async_write, async_read_until 等 ，每个异步操作都需要至少一个完成句柄作为参数，该句柄将在操作完成后被调用。

3. 运行 io 服务
    最后，为了让异步操作得以执行并接收完成通知，你需要调用io_context::run()方法。**此方法会阻塞当前线程，直到没有更多工作要处理为止**。也可以通过多次调用io_context::poll()或io_context::run_one()来非阻塞地处理就绪的操作。

## 要解决的问题

1. 多个线程都调用 api 请求服务器的数据，如何保证不同请求的数据不交错？如何保证请求的响应跟请求对应？响应如何作用于gui？
    * 请求排队发送：发送完一个请求后再发送另一个请求
    * 每次请求服务器时，生成一个uuid跟请求关联起来，把uuid同请求数据一起发送到服务器，服务器响应时传回这个uuid, 客户端通过uuid把响应跟请求关联起来。
    * 客户程序需要提供操作的完成处理程序，以便在操作完成后被调用(其参数为操作结果)

2. asio 库中的一些函数不允许并发调用，例如：async_write (https://think-async.com/asio/asio-1.30.2/doc/asio/reference/async_write/overload1.html)
    * 使用锁+排队写，保证一次只发送一个请求，一个请求发送完成后，再发送下一个请求。


## 请求名称跟命令字段

**关系**
一个请求名称对应一个命令字段，用于在服务端区分请求，从而做不同的处理。

请求名称跟命令字段示例如下：
(计划跟qt同事一起规划操作的请求名称和命令字段，例如，地图有关的操作对应的命令字段为 0x1xxx, 任务有关的操作对应的命令字段为 0x2xxx, ...)

requestname2cmd.ini
```ini
[config]
# 控制 0x1xxx
RELOCATION         = 0x1001
CONFIRM_RELOCATION = 0x1002
TRANSLATION        = 0x1003
ROTATION           = 0x1004
LIFTING            = 0x1005
SET_SPEED          = 0x1006
SET_MOTOR_CONTROL  = 0x1007
SET_PALLET_ROTATION= 0x1008
SET_HINT           = 0x1009
PATH_NAVIGATION    = 0x100a
REMOTE_CONTROL     = 0x100b
EMERGENCY_STOP     = 0x100c
PALLET_ROTATION    = 0x100d
START_CHARGING     = 0x100e

# 地图 0x2xxx
PULL_MAP           = 0x2001
PUSH_MAP           = 0x2002
GET_AGV_POSITION   = 0x2003
GET_POINT_CLOUD    = 0x2004
GET_MAP_LIST       = 0x2005
GET_LOG_LIST       = 0x2006
GET_LOG_FILE       = 0x2007
BUILD_MAPPING      = 0x2008
LOCALIZATION_QUALITY   = 0x2009

# 任务 0x3xxx
QUERY_TASK         = 0x3001
START_TASK         = 0x3002
CANCEL_TASK        = 0x3003
PAUSE_TASK         = 0x3004
RESUME_TASK        = 0x3005
GET_RUN_TASK       = 0x3006

# 其他
UPLOAD_FILE        = 0xf001
SET_OPERATING_MODE = 0xf002
GET_OPERATING_MODE = 0xf003
REBOOT_OR_POWEROFF = 0xf004
SET_DATE_TIME      = 0xf005
GET_DATE_TIME      = 0xf006
TERMINAL_COMMAND   = 0xf007
OTA_UPGRADE        = 0xf008
GET_VELOCITY       = 0xf009
GET_MCU2PC         = 0xf00a
GET_CLIENTS        = 0xf00b
GET_SYSINFO        = 0xf00c
GET_MODEL_FILE     = 0xf00d
GET_CAMERA_POINT_CLOUD = 0xf00e
GET_CAMERA_VIDEO_LIST  = 0xf00f
GET_CAMERA_VIDEO   = 0xf010
GET_ERRORS         = 0xf011
STOP_CHARGING      = 0xf012
HEART_BEAT         = 0xfefe
SET_LOG_LEVEL      = 0Xfffe
UNKOWN_CMD         = 0xffff
```

**三个映射**

* **std::unordered_map<std::string, uint16_t> requestname2cmd_**
读取文件 requestname2cmd.ini 内容生成，用于在发送请求时，根据入参中的 requestname 找到对应的 cmd, 构造发送给服务器的应用层数据。
<br>
* **std::unordered_map<std::string, ResponseHandler> requestname2handlers_**
客户程序调用 api 时，api 内部会记录请求名称跟其完成处理程序的对应关系，在发送请求时，根据入参中的 requestname 找到对应的完成处理程序(handler)，然后记录到 uuid2handlers_
<br>
* **std::unordered_map<std::string, ResponseHandler> uuid2handlers_**
每请求都会生成一个 uuid, 记录请求的uuid跟其完成处理程序，保证在响应回来后根据uuid找到对应请求及其完成处理程序。

## 客户程序使用api的流程分析

D:\byd_agv_njc\文档\客户程序使用qclcpp api的流程分析.drawio
![alt text](image.png)


# 参考

[1. rclcpp API](https://docs.ros.org/en/iron/p/rclcpp/generated/index.html)
[2. 纳博特socket通讯协议](file:///D:/软件科/新人文档_20241216/新人文档/控制器系统通讯接口定义，Socket通讯协议.pdf)
[3. Asio](https://think-async.com/Asio/)
[4. nlohmann json](https://github.com/nlohmann/json/tree/v3.11.3#examples)