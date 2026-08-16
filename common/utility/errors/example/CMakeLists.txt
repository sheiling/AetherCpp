add_executable(errors_example
    main.cpp
)

target_link_libraries(errors_example
    PRIVATE
        ${COMMON_TARGET_NAMESPACE}::errors
)

target_compile_features(errors_example PRIVATE cxx_std_17)
fz2_target_msvc_options(errors_example)
fz2_setup_target(errors_example "common/utility/errors/example")

