# ============================================================
# tcp example — TCP echo server/client 示例
# ============================================================

add_executable(tcp_echo_server
    tcp_echo_server.cpp
)
target_link_libraries(tcp_echo_server
    PRIVATE ${COMMON_TARGET_NAMESPACE}::tcp_comm
)
fz2_target_msvc_options(tcp_echo_server)
fz2_setup_target(tcp_echo_server "common/core/comm/tcp/examples")

add_executable(tcp_echo_client
    tcp_echo_client.cpp
)
target_link_libraries(tcp_echo_client
    PRIVATE ${COMMON_TARGET_NAMESPACE}::tcp_comm
)
fz2_target_msvc_options(tcp_echo_client)
fz2_setup_target(tcp_echo_client "common/core/comm/tcp/examples")

