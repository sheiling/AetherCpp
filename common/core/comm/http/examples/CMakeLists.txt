# ============================================================
# http_comm 示例：简单客户端/服务端
# ============================================================

add_executable(http_client_example
    http_client_example.cpp
)
target_link_libraries(http_client_example PRIVATE ${COMMON_TARGET_NAMESPACE}::http_comm)
fz2_setup_target(http_client_example "common/core/comm/http/examples")

add_executable(http_server_example
    http_server_example.cpp
)
target_link_libraries(http_server_example PRIVATE ${COMMON_TARGET_NAMESPACE}::http_comm)
fz2_setup_target(http_server_example "common/core/comm/http/examples")

