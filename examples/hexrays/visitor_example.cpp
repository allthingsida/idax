/*
idacpp example: hexrays_ctreeparent_visitor_t demonstration

This plugin demonstrates the usage of idacpp::hexrays utilities,
including parent tracking visitor and decompiler helpers.
*/

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <hexrays.hpp>

#include <idacpp/hexrays/hexrays.hpp>

using namespace idacpp::hexrays;
using namespace idacpp::kernwin;

//--------------------------------------------------------------------------
// Demonstrate hexrays_ctreeparent_visitor_t
static void demonstrate_visitor(vdui_t* vu)
{
    if (!vu || !vu->cfunc)
    {
        msg("No decompiled function available\n");
        return;
    }

    msg("=== ctreeparent_visitor_t Example ===\n");
    msg("Function: %a\n\n", vu->cfunc->entry_ea);

    // Create visitor and build parent maps
    ctreeparent_visitor_t visitor;
    visitor.apply_to(&vu->cfunc->body, nullptr);

    // If user has selected an item, show its ancestry
    if (vu->item.citype != VDI_NONE)
    {
        const citem_t* item = vu->item.e ? (citem_t*)vu->item.e : (citem_t*)vu->item.i;

        msg("Selected item type: %s\n", get_ctype_name(item->op));

        // Walk up the parent chain
        msg("\nParent chain:\n");
        const citem_t* current = item;
        int level = 0;
        while (current != nullptr)
        {
            msg("  [%d] %s\n", level, get_ctype_name(current->op));
            current = visitor.parent_of(current);
            level++;
        }

        // Find item by EA
        if (item->ea != BADADDR)
        {
            msg("\nLookup by EA %a:\n", item->ea);
            const citem_t* found = visitor.by_ea(item->ea);
            if (found)
                msg("  Found: %s\n", get_ctype_name(found->op));
            else
                msg("  Not found\n");
        }
    }
    else
    {
        msg("No item selected. Select an expression or statement in decompiler view.\n");
    }

    msg("\n");
}

//--------------------------------------------------------------------------
// Demonstrate hexrays_find_expr
static void demonstrate_find_expr(vdui_t* vu)
{
    if (!vu || !vu->cfunc)
    {
        msg("No decompiled function available\n");
        return;
    }

    msg("=== find_expr Example ===\n");
    msg("Finding all number expressions in function %a:\n\n", vu->cfunc->entry_ea);

    int count = 0;
    find_expr(
        vu->cfunc,
        [&count](cexpr_t* expr) -> int {
            if (expr->op == cot_num)
            {
                qstring s;
                expr->print1(&s, nullptr);
                msg("  [%d] Number at %a: %s\n", count, expr->ea, s.c_str());
                count++;
            }
            return 0;  // Continue
        }
    );

    msg("\nTotal numbers found: %d\n\n", count);
}

//--------------------------------------------------------------------------
// Plugin module
struct plugin_ctx_t : public plugmod_t
{
    action_manager_t actions;

    plugin_ctx_t()
        : actions(this)
    {
        // Action to demonstrate visitor
        actions.add_action(
            AMAHF_HXE_POPUP,
            "idacpp:demo_visitor",
            "Show Item Parents",
            nullptr,
            default_enable_for_vd,
            FO_ACTION_ACTIVATE([])
            {
                auto vu = get_widget_vdui(ctx->widget);
                if (vu)
                    demonstrate_visitor(vu);
                return 1;
            },
            "Demonstrate parent visitor",
            IDAICONS::EYE_GREEN
        );

        // Action to demonstrate find_expr
        actions.add_action(
            AMAHF_HXE_POPUP,
            "idacpp:demo_find_expr",
            "Find Numbers",
            nullptr,
            default_enable_for_vd,
            FO_ACTION_ACTIVATE([])
            {
                auto vu = get_widget_vdui(ctx->widget);
                if (vu)
                    demonstrate_find_expr(vu);
                return 1;
            },
            "Find all number expressions",
            IDAICONS::FLASH
        );

        msg("idacpp hexrays example loaded\n");
        msg("  - Right-click in decompiler view for actions\n");
    }

    virtual ~plugin_ctx_t()
    {
        actions.remove_actions();
        msg("idacpp hexrays example unloaded\n");
    }

    virtual bool idaapi run(size_t arg) override
    {
        msg("Open a decompiler view and right-click for actions\n");
        return true;
    }
};

//--------------------------------------------------------------------------
static plugmod_t* idaapi init()
{
    if (!init_hexrays_plugin())
    {
        msg("Hexrays decompiler not available\n");
        return nullptr;
    }

    return new plugin_ctx_t;
}

//--------------------------------------------------------------------------
static void idaapi term()
{
    term_hexrays_plugin();
}

//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_MULTI,              // Keep plugin loaded
    init,                      // Initialize
    term,                      // Terminate
    nullptr,                   // Invoke
    "idacpp hexrays example",  // Long comment
    "Demonstrates hexrays utilities: visitor and expression finder",  // Multiline help
    "idacpp hexrays",          // Preferred short name
    nullptr                    // Preferred hotkey
};
