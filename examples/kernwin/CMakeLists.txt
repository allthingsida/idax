# Kernwin module examples

ida_add_plugin(action_manager_example
    SOURCES
        action_manager_example.cpp
    OUTPUT_NAME
        action_manager_example
)

target_link_libraries(action_manager_example PRIVATE idacpp::idacpp)
