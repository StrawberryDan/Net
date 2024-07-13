cmake_minimum_required(VERSION 3.25)
project(StrawberryNet)
include(FetchContent)


if (PROJECT_IS_TOP_LEVEL)
    FetchContent_Declare(StrawberryConfig
            GIT_REPOSITORY "https://github.com/StrawberryDan/Config.git"
            GIT_TAG "main")
    FetchContent_MakeAvailable(StrawberryConfig)
    include(StrawberryConfig)
endif ()


if (NOT TARGET StrawberryNet)
    find_package(OpenSSL REQUIRED)


    if (NOT TARGET StrawberryCore AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../Core)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Core ${CMAKE_CURRENT_BINARY_DIR}/Core)
    elseif (NOT TARGET StrawberryCore)
        message(FATAL_ERROR "Could not find Strawberry Core at ${CMAKE_CURRENT_SOURCE_DIR}/../Core")
    endif ()


    add_library(StrawberryNet STATIC
            src/Strawberry/Net/HTTP/Client.hpp
            src/Strawberry/Net/HTTP/Client.cpp
            src/Strawberry/Net/HTTP/Client.inl
            src/Strawberry/Net/HTTP/Constants.hpp
            src/Strawberry/Net/HTTP/Constants.cpp
            src/Strawberry/Net/HTTP/Header.hpp
            src/Strawberry/Net/HTTP/Header.cpp
            src/Strawberry/Net/HTTP/Request.hpp
            src/Strawberry/Net/HTTP/Request.cpp
            src/Strawberry/Net/HTTP/Response.hpp
            src/Strawberry/Net/HTTP/Response.cpp
            src/Strawberry/Net/RTP/Packet.hpp
            src/Strawberry/Net/Socket/API.hpp
            src/Strawberry/Net/Socket/API.cpp
            src/Strawberry/Net/Socket/TCPSocket.hpp
            src/Strawberry/Net/Socket/TCPSocket.cpp
            src/Strawberry/Net/Socket/TLSSocket.hpp
            src/Strawberry/Net/Socket/TLSSocket.cpp
            src/Strawberry/Net/Socket/UDPSocket.hpp
            src/Strawberry/Net/Socket/UDPSocket.cpp
            src/Strawberry/Net/Websocket/Client.hpp
            src/Strawberry/Net/Websocket/Client.cpp
            src/Strawberry/Net/Websocket/Client.inl
            src/Strawberry/Net/Websocket/Message.hpp
            src/Strawberry/Net/Websocket/Message.cpp
            src/Strawberry/Net/Address.hpp
            src/Strawberry/Net/Address.cpp
            src/Strawberry/Net/Endpoint.hpp
            src/Strawberry/Net/Endpoint.cpp
            src/Strawberry/Net/Error.hpp
            src/Strawberry/Net/Socket/TCPListener.hpp
            src/Strawberry/Net/Socket/TCPListener.cpp
            src/Strawberry/Net/Socket/Types.hpp
            src/Strawberry/Net/Socket/BufferedSocket.hpp
    )
    target_include_directories(StrawberryNet PUBLIC src/)
    target_link_libraries(StrawberryNet PUBLIC StrawberryCore OpenSSL::SSL OpenSSL::Crypto)
    set_target_properties(StrawberryNet PROPERTIES CXX_STANDARD 23)


    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(StrawberryNet PUBLIC ws2_32)
    endif ()
endif ()


if (NOT TARGET StrawberryNet_Test)
    add_executable(StrawberryNet_Test
            test/Main.cpp)
    target_link_libraries(StrawberryNet_Test PUBLIC StrawberryNet)
    set_target_properties(StrawberryNet_Test PROPERTIES CXX_STANDARD 23)
endif ()
