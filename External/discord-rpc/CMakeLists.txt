add_library(Discord-RPC STATIC
        src/discord_rpc.cpp
        src/serialization.cpp
        src/rpc_connection.cpp
)

target_include_directories(Discord-RPC PRIVATE
        include
        ../rapidjson/include
)
target_link_libraries(Discord-RPC PRIVATE RapidJSON)

if(WIN32)
    target_sources(Discord-RPC PRIVATE
            src/dllmain.cpp
            src/connection_win.cpp
            src/discord_register_win.cpp
    )
elseif(APPLE)
    target_sources(Discord-RPC PRIVATE
            src/connection_unix.cpp
            src/discord_register_osx.m
    )
    target_link_libraries(Discord-RPC PRIVATE
            "-framework Foundation"
            "-framework Cocoa"
    )
elseif(UNIX)
    target_sources(Discord-RPC PRIVATE
            src/connection_unix.cpp
            src/discord_register_linux.cpp
    )
endif()