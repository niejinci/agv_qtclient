QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# 添加 LOG_LEVEL_ERROR 宏定义
DEFINES += LOG_LEVEL_INFO

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client.cpp \
    main.cpp \
    mainwindow.cpp\
    message.cpp

HEADERS += \
    SimpleIni.h \
    client.h \
    mainwindow.h\
    message.h

# 指定头文件路径
INCLUDEPATH += D:/robotics/3rd/asio-1.30.2/asio-1.30.2/include ./include

# 添加 libssh2 支持
win32 {
    # Windows configuration
    message("current os": Windows)
    INCLUDEPATH += $$PWD/include/libssh2
    LIBS += -L$$PWD/lib -lssh2 -lcrypto -lssl -lz -lws2_32
}
unix:!macx {
    # Ubuntu/Linux configuration
    # ubuntu 编译环境先安装 libssh2 库: sudo apt-get install libssh2-1-dev
    message("current os": Unix/Linux/macOS)
    INCLUDEPATH += /usr/include
    LIBS += -L/usr/lib/x86_64-linux-gnu -lssh2
}

# 指定要链接的库
LIBS += -lpthread

FORMS += \
    mainwindow.ui

# $${TARGET} 是当前项目名称的变量引用
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 如果是动态链接，确保运行时能找到 DLL
# 可以通过设置环境变量 PATH 来实现，或者在运行时手动拷贝 DLL 到可执行文件目录

# windows 平台配置
win32 {
    # 区分构建模式
    CONFIG(release, debug|release) {
        # $$OUT_PWD-表示构建输出的目录
        DESTDIR = $$OUT_PWD/release
    } else {
        DESTDIR = $$OUT_PWD/debug
    }

    # 列出需要复制的所有 DLL 文件
    # $$PWD-表示 .pro 文件所在的目录
    #dll_files.files = $$PWD/bin/*.dll
    #dll_files.path = $$DESTDIR
    #COPIES += dll_files

    # 指定 DLL 文件源位置
    DLL_SOURCE = $$PWD/bin

    # 确保目标目录存在
    QMAKE_POST_LINK += $$quote(cmd /c if not exist \"$$shell_path($$DESTDIR)\" mkdir \"$$shell_path($$DESTDIR)\")$$escape_expand(\\n\\t)
    
    # 复制所有 DLL 文件到目标目录
    QMAKE_POST_LINK += $$quote(cmd /c xcopy /y /i \"$$shell_path($$DLL_SOURCE)\\*.dll\" \"$$shell_path($$DESTDIR)\")$$escape_expand(\\n\\t)
}