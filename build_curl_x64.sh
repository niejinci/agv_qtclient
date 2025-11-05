#!/bin/bash

# 设置环境变量
export CC=x86_64-w64-mingw32-gcc
export AR=x86_64-w64-mingw32-ar
export RANLIB=x86_64-w64-mingw32-ranlib

# 下载curl源码包（如果尚未下载）
if [ ! -d "curl-8.13.0" ]; then
    wget https://curl.se/download/curl-8.13.0.tar.gz
    tar -zxvf curl-8.13.0.tar.gz
fi

# 进入curl目录
cd curl-8.13.0

# 配置选项
./configure \
    --host=x86_64-w64-mingw32 \
    --prefix=/mnt/d/byd_agv_in_gitee/agv_qtclient/curl-install-here \
    --disable-shared \
    --enable-static \
    --with-winssl

# 编译
make clean
make -j$(nproc)

# 安装（这会把生成的文件复制到指定的前缀目录）
make install

echo "静态库已生成并安装至 /mnt/d/byd_agv_in_gitee/agv_qtclient/curl-install-here"
