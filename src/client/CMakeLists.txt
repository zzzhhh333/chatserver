#将当前目录下所有的源文件放入变量SRC_LIST中
aux_source_directory(. SRC_LIST)


set(OTH_SRC ../server/model/user.cpp ../server/model/groupuser.cpp ../server/model/group.cpp)

list(APPEND SRC_LIST ${OTH_SRC})

#添加生成的可执行文件
add_executable(chatClient ${SRC_LIST})


#给指定目标文件链接需要的库
target_link_libraries(chatClient pthread)