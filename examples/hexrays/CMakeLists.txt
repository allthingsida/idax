# Hexrays module examples

ida_add_plugin(visitor_example
    SOURCES
        visitor_example.cpp
    OUTPUT_NAME
        visitor_example
)

target_link_libraries(visitor_example PRIVATE idacpp::idacpp)
