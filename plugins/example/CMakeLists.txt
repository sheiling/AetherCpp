# ============================================================
# Example / demo plugins (optional)
#
# Enable with: cmake -DDEMO_PLUGINS_EXAMPLE_BUILD=ON ...
# ============================================================

message(STATUS "  [plugins/example] Configuring example plugins...")

add_subdirectory(home)
add_subdirectory(settings)
add_subdirectory(mvvm)
add_subdirectory(middleware)
add_subdirectory(invoker)
add_subdirectory(container)
