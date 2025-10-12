/*
idacpp - Modern C++ extensions for IDA SDK
Copyright (c) 2025 Elias Bachaalany <elias.bachaalany@gmail.com>

Hexrays utilities module - Decompiler support
*/
#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>

#include <hexrays.hpp>

#include <idacpp/kernwin/kernwin.hpp>

namespace idacpp::hexrays
{

//----------------------------------------------------------------------------------
/**
 * @brief Default update state handler for Hexrays decompiler views (expression selected).
 *
 * Enables action only when a decompiler widget is active and an expression is selected.
 */
inline kernwin::update_state_ah_t default_enable_for_vd_expr = FO_ACTION_UPDATE([],
    auto vu = get_widget_vdui(widget);
    return (vu == nullptr) ? AST_DISABLE_FOR_WIDGET
                            : vu->item.citype == VDI_EXPR ? AST_ENABLE : AST_DISABLE;
);

/**
 * @brief Default update state handler for Hexrays decompiler views.
 *
 * Enables action whenever a decompiler widget is active.
 */
inline kernwin::update_state_ah_t default_enable_for_vd = FO_ACTION_UPDATE([],
    auto vu = get_widget_vdui(widget);
    return vu == nullptr ? AST_DISABLE_FOR_WIDGET : AST_ENABLE;
);

//----------------------------------------------------------------------------------
/**
 * @brief Enhanced ctree visitor with parent tracking and EA mapping.
 *
 * Extends ctree_parentee_t to build and maintain maps of parent relationships
 * and effective address to item mappings during tree traversal.
 */
class ctreeparent_visitor_t : public ctree_parentee_t
{
private:
    std::map<const citem_t*, const citem_t*> parent;   ///< Parent map
    std::map<const ea_t, const citem_t*> ea2item;      ///< EA to item map

public:
    /**
     * @brief Visit expression node.
     */
    int idaapi visit_expr(cexpr_t* e) override
    {
        ea2item[e->ea] = parent[e] = parent_expr();
        return 0;
    }

    /**
     * @brief Visit instruction node.
     */
    int idaapi visit_insn(cinsn_t* ins) override
    {
        parent[ins] = parent_insn();
        return 0;
    }

    /**
     * @brief Get parent of a tree item.
     *
     * @param item Tree item
     * @return Parent item, or nullptr if root
     */
    const citem_t* parent_of(const citem_t* item)
    {
        return parent[item];
    }

    /**
     * @brief Find tree item by effective address.
     *
     * @param ea Effective address
     * @return Tree item at EA, or nullptr if not found
     */
    const citem_t* by_ea(ea_t ea) const
    {
        auto p = ea2item.find(ea);
        return p == std::end(ea2item) ? nullptr : p->second;
    }

    /**
     * @brief Check if parent_item is ancestor of item.
     *
     * @param parent_item Potential ancestor
     * @param item Child item to check
     * @return true if parent_item is an ancestor of item
     */
    bool is_ancestor_of(const citem_t* parent_item, const citem_t* item)
    {
        while (item != nullptr)
        {
            item = parent_of(item);
            if (item == parent_item)
                return true;
        }
        return false;
    }
};

/// Unique pointer to ctree parent visitor
using ctreeparent_visitor_ptr_t = std::unique_ptr<ctreeparent_visitor_t>;

//----------------------------------------------------------------------------------
/**
 * @brief Get selection range from widget.
 *
 * @param widget Target widget
 * @param end_ea Output parameter for end address (optional)
 * @param widget_type Expected widget type, or -1 for any
 * @return Start address, or BADADDR if no selection
 */
inline ea_t get_selection_range(
    TWidget* widget,
    ea_t* end_ea = nullptr,
    int widget_type = BWN_DISASM)
{
    ea_t ea1 = BADADDR, ea2 = BADADDR;
    do
    {
        if (widget == nullptr || (widget_type != -1 && get_widget_type(widget) != widget_type))
            break;

        if (!read_range_selection(widget, &ea1, &ea2))
        {
            ea1 = get_screen_ea();

            if (ea1 == BADADDR)
                break;

            ea2 = next_head(ea1, BADADDR);
            if (ea2 == BADADDR)
                ea2 = ea1 + 1;
        }
    } while (false);

    if (end_ea != nullptr)
        *end_ea = ea2;

    return ea1;
}

//----------------------------------------------------------------------------------
/**
 * @brief Get the statement instruction containing a UI item.
 *
 * @param cfunc Decompiled function
 * @param ui_item Current UI item
 * @param ohelper Optional in/out parameter for parent visitor (for reuse)
 * @return Statement instruction, or nullptr if not found
 */
inline const cinsn_t* get_stmt_insn(
    cfunc_t* cfunc,
    const citem_t* ui_item,
    ctreeparent_visitor_ptr_t* ohelper = nullptr)
{
    auto func_body = &cfunc->body;

    const citem_t* item = ui_item;
    const citem_t* stmt_item;

    ctreeparent_visitor_t* helper = nullptr;

    if (ohelper != nullptr)
    {
        // Start a new helper
        if (*ohelper == nullptr)
        {
            helper = new ctreeparent_visitor_t();
            helper->apply_to(func_body, nullptr);

            ohelper->reset(helper);
        }
        else
        {
            helper = ohelper->get();
        }
    }

    auto get_parent = [func_body, &helper](const citem_t* item)
    {
        return helper == nullptr ? func_body->find_parent_of(item)
                                 : helper->parent_of(item);
    };

    // Get the top level statement from this item
    for (stmt_item = item;
         item != nullptr && item->is_expr();
         item = get_parent(item))
    {
        stmt_item = item;
    }

    // ...then the actual instruction item
    if (stmt_item->is_expr())
        stmt_item = get_parent(stmt_item);

    return (const cinsn_t*)stmt_item;
}

//----------------------------------------------------------------------------------
/**
 * @brief Get the block and position of a statement.
 *
 * @param cfunc Decompiled function
 * @param stmt_item Statement item
 * @param p_cblock Output parameter for containing block
 * @param p_pos Output parameter for position in block
 * @param helper Optional parent visitor
 * @return true if found, false otherwise
 */
inline bool get_stmt_block_pos(
    cfunc_t* cfunc,
    const citem_t* stmt_item,
    cblock_t** p_cblock,
    cblock_t::iterator* p_pos,
    ctreeparent_visitor_t* helper = nullptr)
{
    auto func_body = &cfunc->body;
    auto cblock_insn = (cinsn_t*)(
        helper == nullptr ? func_body->find_parent_of(stmt_item)
                          : helper->parent_of(stmt_item));

    if (cblock_insn == nullptr || cblock_insn->op != cit_block)
        return false;

    cblock_t* cblock = cblock_insn->cblock;

    for (auto p = cblock->begin(); p != cblock->end(); ++p)
    {
        if (&*p == stmt_item)
        {
            *p_pos = p;
            *p_cblock = cblock;
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------------
/**
 * @brief Check if any instruction in list is ancestor of item.
 *
 * @param h Parent visitor
 * @param inst Instruction list
 * @param item Item to check
 * @return true if any instruction is ancestor of item
 */
inline bool are_ancestor_of(
    ctreeparent_visitor_t* h,
    cinsnptrvec_t& inst,
    citem_t* item)
{
    for (auto parent : inst)
    {
        if (h->is_ancestor_of(parent, item))
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------------
/**
 * @brief Keep only lowest common ancestor instructions in list.
 *
 * Removes instructions that are ancestors of other instructions in the list,
 * keeping only the deepest (most specific) instructions.
 *
 * @param cfunc Decompiled function
 * @param helper Parent visitor
 * @param bulk_list Instruction list (modified in place)
 */
inline void keep_lca_cinsns(
    cfunc_t* cfunc,
    ctreeparent_visitor_t* helper,
    cinsnptrvec_t& bulk_list)
{
    cinsnptrvec_t new_list;
    while (!bulk_list.empty())
    {
        auto item = bulk_list.back();
        bulk_list.pop_back();

        if (!are_ancestor_of(helper, bulk_list, item) &&
            !are_ancestor_of(helper, new_list, item))
            new_list.push_back(item);
    }
    new_list.swap(bulk_list);
}

//----------------------------------------------------------------------------------
/**
 * @brief Find expressions in decompiled function using callback.
 *
 * @param func Decompiled function
 * @param cb Callback invoked for each expression (return 0 to continue, non-zero to stop)
 * @param flags Visitor flags (default: CV_FAST)
 * @param parent Starting parent item (nullptr for entire function)
 */
inline void find_expr(
    cfuncptr_t func,
    std::function<int(cexpr_t*)> cb,
    int flags = CV_FAST,
    citem_t* parent = nullptr)
{
    struct visitor_wrapper : public ctree_visitor_t
    {
        std::function<int(cexpr_t*)> cb;

        visitor_wrapper(int flags, std::function<int(cexpr_t*)> cb)
            : ctree_visitor_t(flags), cb(cb) {}

        virtual int idaapi visit_expr(cexpr_t* expr)
        {
            return cb(expr);
        }
    };

    visitor_wrapper v(flags, cb);
    v.apply_to(&func->body, parent);
}

}  // namespace idacpp::hexrays
