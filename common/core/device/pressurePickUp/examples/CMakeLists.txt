add_executable(pressurepickup_demo pressurepickup_demo.cpp)

target_include_directories(pressurepickup_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(pressurepickup_demo
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::pressurepickup
)

fz2_target_msvc_options(pressurepickup_demo)
fz2_setup_target(pressurepickup_demo "common/core/device/pressurePickUp/examples")
