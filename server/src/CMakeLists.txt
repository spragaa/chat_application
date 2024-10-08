add_library(server_lib
    server.cpp
    db_manager.cpp
    user_metadata_repository.cpp
    message_metadata_repository.cpp
    message_text_repository.cpp
)

target_link_libraries(server_lib
    Boost::boost
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    pqxx
    redis++
    hiredis
)

target_include_directories(server_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PUBLIC ${CMAKE_SOURCE_DIR}/common/include
    PRIVATE ${nlohmann_json_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/libpqxx
    ${redis_plus_plus_SOURCE_DIR}/src/sw/redis++/
    ${hiredis_SOURCE_DIR}
)