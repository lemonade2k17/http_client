cmake_minimum_required(VERSION 3.20)
project(http_client C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(http_client main.c
        net.c
        net.h
        http.c
        http.h
        util.c
        util.h
        url.c
        url.h)

# HTTP 客户端本身不依赖任何第三方库：
# 网络部分全部来自 Winsock 2，链接 ws2_32 就够了。
if (WIN32)
    target_link_libraries(http_client PRIVATE ws2_32)
endif ()
