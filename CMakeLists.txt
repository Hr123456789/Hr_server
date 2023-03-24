#设置最小的cmake版本
cmake_minimum_required(VERSION 3.0)
#项目名字
project(sylar)

#显示执行构建过程中的详细的编译信息
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ldl -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ldl -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(/usr/local/include)



set(LIB_SRC
    sylar/log.cc
    sylar/mutex.cc
    sylar/util.cc
    sylar/config.cc
    sylar/thread.cc
    sylar/fiber.cc
    sylar/scheduler.cc
    sylar/iomanager.cc
    sylar/fd_manager.cc
    sylar/timer.cc
    sylar/hook.cc
    sylar/address.cc
    sylar/socket.cc
    sylar/bytearray.cc
    sylar/stream.cc
    sylar/tcp_server.cc
    sylar/util/crypto_util.cc
    sylar/util/hash_util.cc
    sylar/util/json_util.cc
    sylar/http/http_parser.cc
    sylar/http/http.cc
    sylar/http/http_session.cc
    sylar/http/http_server.cc
    sylar/http/servlet.cc
    sylar/streams/socket_stream.cc
    sylar/http/http11_parser.cc
    sylar/http/httpclient_parser.cc
    )

#生成库文件
add_library(sylar SHARED ${LIB_SRC})

set (LIB_LIB
    sylar
    pthread
    yaml-cpp
    jsoncpp
    ssl
    protobuf
)

#根据源文件生成可执行文件
add_executable(test ./tests/test.cc)
#指定依赖
add_dependencies(test sylar)
#链接动态库
target_link_libraries(test ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_config ./tests/test_config.cc)
#指定依赖
add_dependencies(test_config sylar)
#链接动态库
target_link_libraries(test_config ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_thread ./tests/test_thread.cc)
#指定依赖
add_dependencies(test_thread sylar)
#链接动态库
target_link_libraries(test_thread ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_fiber ./tests/test_fiber.cc)
#指定依赖
add_dependencies(test_fiber sylar)
#链接动态库
target_link_libraries(test_fiber ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_scheduler ./tests/test_scheduler.cc)
#指定依赖
add_dependencies(test_scheduler sylar)
#链接动态库
target_link_libraries(test_scheduler ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_iomanager ./tests/test_iomanager.cc)
#指定依赖
add_dependencies(test_iomanager sylar)
#链接动态库
target_link_libraries(test_iomanager ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_hook ./tests/test_hook.cc)
#指定依赖
add_dependencies(test_hook sylar)
#链接动态库
target_link_libraries(test_hook ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_tcp_server ./tests/test_tcp_server.cc)
#指定依赖
add_dependencies(test_tcp_server sylar)
#链接动态库
target_link_libraries(test_tcp_server ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(echo_tcp_server ./examples/echo_server.cc)
#指定依赖
add_dependencies(echo_tcp_server sylar)
#链接动态库
target_link_libraries(echo_tcp_server ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_http_parser ./tests/test_http_parser.cc)
#指定依赖
add_dependencies(test_http_parser sylar)
#链接动态库
target_link_libraries(test_http_parser ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(test_http_server ./tests/test_http_server.cc)
#指定依赖
add_dependencies(test_http_server sylar)
#链接动态库
target_link_libraries(test_http_server ${LIB_LIB})

#根据源文件生成可执行文件
add_executable(my_http_server ./samples/my_http_server.cc)
#指定依赖
add_dependencies(my_http_server sylar)
#链接动态库
target_link_libraries(my_http_server ${LIB_LIB})

#设置可执行文件的生成路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置库文件生成路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
