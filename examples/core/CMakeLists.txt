# Core module examples

ida_add_plugin(objcontainer_example
    SOURCES
        objcontainer_example.cpp
    OUTPUT_NAME
        objcontainer_example
)

target_link_libraries(objcontainer_example PRIVATE idacpp::idacpp)
