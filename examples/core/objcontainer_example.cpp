/*
idacpp example: objcontainer_t demonstration

This plugin demonstrates the usage of idacpp::core::objcontainer_t,
a RAII container for managing object lifetimes automatically.
*/

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

#include <idacpp/core/core.hpp>

//--------------------------------------------------------------------------
// Example class to store in container
struct my_data_t
{
    qstring name;
    int value;

    my_data_t(const char* n, int v) : name(n), value(v)
    {
        msg("Creating my_data_t: %s = %d\n", n, v);
    }

    ~my_data_t()
    {
        msg("Destroying my_data_t: %s\n", name.c_str());
    }
};

//--------------------------------------------------------------------------
// Demonstrate objcontainer_t usage
static void demonstrate_objcontainer()
{
    using namespace idacpp::core;

    msg("=== objcontainer_t Example ===\n");

    objcontainer_t<my_data_t> container;

    // Create objects with different constructors
    auto* obj1 = container.create("First", 100);
    auto* obj2 = container.create("Second", 200);
    auto* obj3 = container.create("Third", 300);

    msg("\nContainer now has %zu objects\n", container.size());

    // Access by positive index
    msg("\nAccessing by positive index:\n");
    if (auto* obj = container[0])
        msg("  container[0]: %s = %d\n", obj->name.c_str(), obj->value);
    if (auto* obj = container[1])
        msg("  container[1]: %s = %d\n", obj->name.c_str(), obj->value);

    // Access by negative index (from end)
    msg("\nAccessing by negative index:\n");
    if (auto* obj = container[-1])
        msg("  container[-1] (last): %s = %d\n", obj->name.c_str(), obj->value);
    if (auto* obj = container[-2])
        msg("  container[-2]: %s = %d\n", obj->name.c_str(), obj->value);

    // Out of bounds returns nullptr
    msg("\nOut of bounds access:\n");
    if (auto* obj = container[100])
        msg("  container[100]: Found (unexpected!)\n");
    else
        msg("  container[100]: nullptr (expected)\n");

    msg("\nExiting scope - all objects will be automatically destroyed:\n");
    // Container destroyed here, all objects automatically cleaned up
}

//--------------------------------------------------------------------------
// Plugin initialization
plugmod_t* idaapi init()
{
    demonstrate_objcontainer();
    return PLUGIN_SKIP;  // Don't keep plugin loaded
}

//--------------------------------------------------------------------------
// Plugin definition
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_HIDE,           // Plugin flags
    init,                  // Initialize
    nullptr,               // Terminate
    nullptr,               // Invoke
    nullptr,               // Long comment
    nullptr,               // Multiline help
    "idacpp objcontainer example",  // Preferred short name
    nullptr                // Preferred hotkey
};
