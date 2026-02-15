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


    find_strawberry_library(NAMES Core)


    new_strawberry_library(NAME StrawberryNet SOURCE
      src/Strawberry/Net/Address.cpp
      src/Strawberry/Net/Address.hpp
      src/Strawberry/Net/Endpoint.cpp
      src/Strawberry/Net/Endpoint.hpp
      src/Strawberry/Net/Error.hpp
      src/Strawberry/Net/HTTP/Constants.cpp
      src/Strawberry/Net/HTTP/Constants.hpp
      src/Strawberry/Net/HTTP/HTTPClient.cpp
      src/Strawberry/Net/HTTP/HTTPClient.hpp
      src/Strawberry/Net/HTTP/HTTPClient.inl
      src/Strawberry/Net/HTTP/Header.cpp
      src/Strawberry/Net/HTTP/Header.hpp
      src/Strawberry/Net/HTTP/Request.cpp
      src/Strawberry/Net/HTTP/Request.hpp
      src/Strawberry/Net/HTTP/Response.cpp
      src/Strawberry/Net/HTTP/Response.hpp
      src/Strawberry/Net/RTP/Packet.hpp
      src/Strawberry/Net/Socket/API.cpp
      src/Strawberry/Net/Socket/API.hpp
      src/Strawberry/Net/Socket/BufferedSocket.hpp
      src/Strawberry/Net/Socket/Platform.hpp
      src/Strawberry/Net/Socket/TCPListener.cpp
      src/Strawberry/Net/Socket/TCPListener.hpp
      src/Strawberry/Net/Socket/TCPSocket.cpp
      src/Strawberry/Net/Socket/TCPSocket.hpp
      src/Strawberry/Net/Socket/TLSSocket.cpp
      src/Strawberry/Net/Socket/TLSSocket.hpp
      src/Strawberry/Net/Socket/Types.hpp
      src/Strawberry/Net/Socket/UDPSocket.cpp
      src/Strawberry/Net/Socket/UDPSocket.hpp
      src/Strawberry/Net/Websocket/Message.cpp
      src/Strawberry/Net/Websocket/Message.hpp
      src/Strawberry/Net/Websocket/WebsocketClient.cpp
      src/Strawberry/Net/Websocket/WebsocketClient.hpp
      src/Strawberry/Net/Websocket/WebsocketClient.inl
    )
    target_include_directories(StrawberryNet PUBLIC src/)
    target_link_libraries(StrawberryNet PUBLIC
      StrawberryCore
      OpenSSL::SSL
      OpenSSL::Crypto)


    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(StrawberryNet PUBLIC ws2_32)
    endif ()

    new_strawberry_tests(NAME "StrawberryNet" TESTS
      test/TCP.cpp
      test/UDP.cpp
      test/HTTP.cpp
    )
endif ()
