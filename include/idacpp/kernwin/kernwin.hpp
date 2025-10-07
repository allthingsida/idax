/*
idacpp - Modern C++ extensions for IDA SDK
Copyright (c) 2025 Elias Bachaalany <elias.bachaalany@gmail.com>

Kernwin utilities module - UI and action management
*/
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <kernwin.hpp>

#include <idacpp/core/core.hpp>

namespace idacpp::kernwin
{

//----------------------------------------------------------------------------------
/**
 * @brief Named constants for IDA's built-in GUI icons.
 *
 * Makes icon usage more readable than magic numbers.
 */
namespace IDAICONS
{
enum
{
    EYE_GLASSES_EDIT      = 43,   ///< Eye glasses with a small pencil overlay
    GREEN_DOT             = 356,  ///< Filled green circle (used to designate a disabled breakpoint)
    GREEN_PLAY_BUTTON     = 376,  ///< Green play button (start process)
    RED_DOT               = 59,   ///< Filled red circle (used to designate an active breakpoint)
    DISABLED              = 62,   ///< Circle with line through it (used to designate a disabled item)
    GRAPH_WITH_FUNC       = 77,   ///< Nodes in a graph icon with a smaller function icon overlapped on top
    YELLOW_COG_WHEEL      = 156,  ///< Yellow cog wheel
    FLASH                 = 171,  ///< Flash icon
    KEYBOARD_GRAY         = 173,  ///< A grayish keyboard
    EYE_GREEN             = 50,   ///< Eye icon with a green color
    PRINTER               = 158,  ///< Printer icon
    GRAY_X_CIRCLE         = 175,  ///< A filled gray circle with an X in it
    NOTEPAD_1             = 73,   ///< A notepad icon
    NOTEPAD_2             = 339,  ///< A notepad icon
    LIGHT_BULB            = 174,  ///< A light bulb icon
    TABLE_BLUE_CELLS_3X2  = 100,  ///< A table with blue cells (3x2)
    TABLE_WHITE_CELLS_4X2 = 418,  ///< A table with white cells (4x2)
};
}  // namespace IDAICONS

//----------------------------------------------------------------------------------
/// Function type for action update/state callbacks
using update_state_ah_t = std::function<action_state_t(action_update_ctx_t* ctx, bool is_widget)>;

/// Function type for action activation callbacks
using activate_ah_t = std::function<int(action_activation_ctx_t* ctx)>;

//----------------------------------------------------------------------------------
/**
 * @brief Function object-based action handler.
 *
 * Utility class that allows using function objects instead of inheriting from
 * action_handler_t each time.
 */
struct fo_action_handler_ah_t : public action_handler_t
{
    qstring name;                    ///< Action name
    const char* popup_path;          ///< Popup menu path for attachment
    update_state_ah_t f_update;      ///< Update/state callback
    activate_ah_t f_activate;        ///< Activation callback

    fo_action_handler_ah_t(
        const char* name,
        update_state_ah_t f_update,
        activate_ah_t f_activate,
        const char* popup_path = nullptr)
        : name(name), f_update(f_update), f_activate(f_activate), popup_path(popup_path)
    {
    }

    action_state_t idaapi update(action_update_ctx_t* ctx) override
    {
        return f_update(ctx, false);
    }

    virtual int idaapi activate(action_activation_ctx_t* ctx) override
    {
        return f_activate(ctx);
    }

    action_state_t idaapi get_state(TWidget* widget)
    {
        return f_update((action_update_ctx_t*)widget, true);
    }
};

/// Vector of action handler pointers
using fo_action_handler_vec_t = std::vector<fo_action_handler_ah_t*>;

//----------------------------------------------------------------------------------
/**
 * @brief Helper macro to create update/state lambda for actions.
 *
 * @param captures Lambda capture list (e.g., [], [this], [&])
 * @param update Function body that returns action_state_t
 *
 * @example
 * @code
 * update_state_ah_t my_update = FO_ACTION_UPDATE([],
 *     return get_screen_ea() != BADADDR ? AST_ENABLE : AST_DISABLE;
 * );
 * @endcode
 */
#define FO_ACTION_UPDATE(captures, update)                             \
    captures(action_update_ctx_t* ctx, bool is_widget)->action_state_t \
    {                                                                  \
        TWidget* widget = is_widget ? (TWidget*)ctx : ctx->widget;    \
        update                                                         \
    }

/**
 * @brief Helper macro to create activation lambda for actions.
 *
 * @param captures Lambda capture list (e.g., [], [this], [&])
 *
 * @example
 * @code
 * activate_ah_t my_activate = FO_ACTION_ACTIVATE([](action_activation_ctx_t* ctx) {
 *     msg("Action activated!\n");
 *     return 1;
 * });
 * @endcode
 */
#define FO_ACTION_ACTIVATE(captures) \
    captures(action_activation_ctx_t* ctx)->int

//----------------------------------------------------------------------------------
/**
 * @brief Manages IDA action lifecycle and popup menu attachment.
 *
 * Provides simplified interface for creating and managing IDA actions with
 * automatic cleanup and popup menu integration.
 */
class action_manager_t
{
    // Action manager action handler flags:
    #define AMAHF_NONE      0x00  ///< No special flags
    #define AMAHF_HXE_POPUP 0x01  ///< Attach to Hexrays popup
    #define AMAHF_IDA_POPUP 0x04  ///< Attach to IDA popup

    core::objcontainer_t<fo_action_handler_ah_t> action_handlers;  ///< Owned action handlers
    core::objcontainer_t<qstring> popup_paths;                     ///< Owned popup path strings

    fo_action_handler_vec_t want_hxe_popup;  ///< Actions for Hexrays popup
    fo_action_handler_vec_t want_ida_popup;  ///< Actions for IDA popup
    const void* plg_owner;                   ///< Plugin owner
    const char* current_popup_path = nullptr;///< Current popup path for new actions

public:
    /// Default enable state for disassembly view
    update_state_ah_t default_enable_for_disasm = FO_ACTION_UPDATE([],
        return get_widget_type(widget) == BWN_DISASM ? AST_ENABLE_FOR_WIDGET : AST_DISABLE_FOR_WIDGET;
    );

    /// Default enable state for both disassembly and decompiler views
    update_state_ah_t default_enable_for_vd_disasm = FO_ACTION_UPDATE([],
        auto t = get_widget_type(widget);
        return (t == BWN_DISASM || t == BWN_PSEUDOCODE) ? AST_ENABLE_FOR_WIDGET : AST_DISABLE_FOR_WIDGET;
    );

    /**
     * @brief Set popup path for subsequently created actions.
     *
     * @param path Popup menu path, or nullptr to clear
     */
    void set_popup_path(const char* path = nullptr)
    {
        if (path == nullptr)
            current_popup_path = nullptr;
        else
            current_popup_path = popup_paths.create(path)->c_str();
    }

    /**
     * @brief UI notification handler for IDA popup menus.
     */
    ssize_t on_ui_finish_populating_widget_popup(va_list va)
    {
        TWidget* widget          = va_arg(va, TWidget*);
        TPopupMenu* popup_handle = va_arg(va, TPopupMenu*);
        maybe_attach_to_popup(false, widget, popup_handle);
        return 0;
    }

    /**
     * @brief Hexrays notification handler for decompiler popup menus.
     */
    ssize_t on_hxe_populating_popup(va_list va)
    {
        TWidget* widget   = va_arg(va, TWidget*);
        TPopupMenu* popup = va_arg(va, TPopupMenu*);
        maybe_attach_to_popup(true, widget, popup);
        return 0;
    }

    /**
     * @brief Attach specific action to popup menu.
     *
     * @param act Action handler to attach
     * @param widget Target widget
     * @param popup_handle Popup menu handle
     * @param popuppath Menu path (uses action's path if nullptr)
     * @param flags Attachment flags
     * @return true if attached successfully
     */
    bool attach_to_popup(
        fo_action_handler_ah_t* act,
        TWidget* widget,
        TPopupMenu* popup_handle,
        const char* popuppath = nullptr,
        int flags = 0)
    {
        if (popuppath == nullptr)
            popuppath = act->popup_path;

        return attach_action_to_popup(
            widget,
            popup_handle,
            act->name.c_str(),
            popuppath,
            flags);
    }

    /**
     * @brief Maybe attach registered actions to popup menu.
     *
     * @param via_hxe true if called from Hexrays, false if from IDA
     * @param widget Target widget
     * @param popup_handle Popup menu handle
     * @param popuppath Override popup path (optional)
     * @param flags Attachment flags
     */
    void maybe_attach_to_popup(
        bool via_hxe,
        TWidget* widget,
        TPopupMenu* popup_handle,
        const char* popuppath = nullptr,
        int flags = 0)
    {
        auto& lst = via_hxe ? want_hxe_popup : want_ida_popup;
        for (auto& act : lst)
        {
            if (is_action_enabled(act->get_state(widget)))
            {
                attach_action_to_popup(
                    widget,
                    popup_handle,
                    act->name.c_str(),
                    popuppath == nullptr ? act->popup_path : popuppath,
                    flags);
            }
        }
    }

    /**
     * @brief Construct action manager.
     *
     * @param owner Plugin owner pointer (for plugmod_t)
     */
    action_manager_t(const void* owner = nullptr) : plg_owner(owner) {}

    /**
     * @brief Register and add a new action.
     *
     * @param amflags Action manager flags (AMAHF_*)
     * @param name Action name (must be unique)
     * @param label Action label (displayed in UI)
     * @param shortcut Keyboard shortcut (e.g., "Ctrl-Shift-A")
     * @param f_update Update/state callback
     * @param f_activate Activation callback
     * @param tooltip Tooltip text (optional)
     * @param icon Icon ID (use IDAICONS or -1 for none)
     * @return Pointer to created action handler, or nullptr on failure
     */
    fo_action_handler_ah_t* add_action(
        int amflags,  // one of AMAHF_* flags
        const char* name,
        const char* label,
        const char* shortcut,
        update_state_ah_t f_update,
        activate_ah_t f_activate,
        const char* tooltip = nullptr,
        int icon = -1)
    {
        bool ok = register_action(ACTION_DESC_LITERAL_PLUGMOD(
            name,
            label,
            action_handlers.create(name, f_update, f_activate, current_popup_path),
            plg_owner,
            shortcut,
            tooltip,
            icon));

        fo_action_handler_ah_t* act = nullptr;

        if (ok)
        {
            act = action_handlers[-1];
            if (amflags & AMAHF_HXE_POPUP)
                want_hxe_popup.push_back(act);
            if (amflags & AMAHF_IDA_POPUP)
                want_ida_popup.push_back(act);
        }
        else
        {
            action_handlers.pop_back();
        }

        return act;
    }

    /**
     * @brief Remove and unregister all managed actions.
     */
    void remove_actions()
    {
        for (auto& ah : action_handlers)
            unregister_action(ah->name.c_str());
        action_handlers.clear();
    }
};

}  // namespace idacpp::kernwin
