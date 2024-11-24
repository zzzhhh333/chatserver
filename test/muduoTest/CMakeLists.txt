cmake_minimum_required(VERSION 3.0)
project(main)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#设置可执行文件最终储存路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件搜索路径
#include_directories()

#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduoServer.cpp)

#把./文件下所有源文件名字放入变量SRC_LIST里面
#aux_source_directory(. SRC_LIST)
#表示将源文件SRC_LIST编译，并生成可执行文件server
add_executable(server ${SRC_LIST})

#表示目标文件server需要链接的库
target_link_libraries(server muduo_net muduo_base pthread)