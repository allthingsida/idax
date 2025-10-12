/*
idacpp example: action_manager_t demonstration

This plugin demonstrates the usage of idacpp::kernwin::action_manager_t,
showing how to create and manage IDA actions with lambda handlers.
*/

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

#include <idacpp/kernwin/kernwin.hpp>

using namespace idacpp::kernwin;

//--------------------------------------------------------------------------
// Plugin module
struct plugin_ctx_t : public plugmod_t
{
    action_manager_t actions;

    plugin_ctx_t()
        : actions(this)  // Pass plugin module as owner
    {
        // Create simple action with lambda handlers
        actions.add_action(
            AMAHF_NONE,
            "idacpp:hello",
            "Say Hello",
            "Ctrl-Shift-H",
            // Update handler - enable always
            FO_ACTION_UPDATE([], {
                return AST_ENABLE_ALWAYS;
            }),
            // Activate handler
            FO_ACTION_ACTIVATE([])
            {
                msg("Hello from idacpp action!\n");
                return 1;
            },
            "Print a hello message",
            IDAICONS::LIGHT_BULB
        );

        // Create action enabled only in disassembly view
        actions.add_action(
            AMAHF_IDA_POPUP,
            "idacpp:show_ea",
            "Show Current EA",
            nullptr,
            // Update handler - enable only in disassembly
            actions.default_enable_for_disasm,
            // Activate handler - show current address
            FO_ACTION_ACTIVATE([])
            {
                ea_t ea = get_screen_ea();
                if (ea != BADADDR)
                    msg("Current EA: %a\n", ea);
                else
                    msg("No current EA\n");
                return 1;
            },
            "Display current effective address",
            IDAICONS::EYE_GREEN
        );

        // Create action for both disasm and decompiler
        actions.add_action(
            AMAHF_IDA_POPUP,
            "idacpp:widget_info",
            "Widget Info",
            nullptr,
            // Enable in both disasm and decompiler
            actions.default_enable_for_vd_disasm,
            // Show widget information
            FO_ACTION_ACTIVATE([])
            {
                TWidget* widget = ctx->widget;
                if (widget)
                {
                    qstring title;
                    get_widget_title(&title, widget);
                    int type = get_widget_type(widget);
                    msg("Widget: '%s' (type=%d)\n", title.c_str(), type);
                }
                return 1;
            },
            "Show information about current widget",
            IDAICONS::NOTEPAD_1
        );

        // Attach actions to menu
        attach_action_to_menu("Edit/Plugins/", "idacpp:hello", SETMENU_APP);
        attach_action_to_menu("Edit/Plugins/", "idacpp:show_ea", SETMENU_APP);
        attach_action_to_menu("Edit/Plugins/", "idacpp:widget_info", SETMENU_APP);

        msg("idacpp action_manager example loaded\n");
        msg("  - Ctrl-Shift-H: Say Hello\n");
        msg("  - Right-click in views for context menu actions\n");
    }

    virtual ~plugin_ctx_t()
    {
        // Clean up all actions
        actions.remove_actions();
        msg("idacpp action_manager example unloaded\n");
    }

    virtual bool idaapi run(size_t arg) override
    {
        msg("Use Ctrl-Shift-H or the menu actions\n");
        return true;
    }
};

//--------------------------------------------------------------------------
static plugmod_t* idaapi init()
{
    return new plugin_ctx_t;
}

//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_MULTI,              // Keep plugin loaded, support multiple instances
    init,                      // Initialize
    nullptr,                   // Terminate
    nullptr,                   // Invoke
    "idacpp action_manager example",  // Long comment
    "Demonstrates action_manager_t usage with lambda handlers",  // Multiline help
    "idacpp actions",          // Preferred short name
    nullptr                    // Preferred hotkey
};
