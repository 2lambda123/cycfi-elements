/*=================================================================================================
   Copyright (c) 2016-2023 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=================================================================================================*/
#include <elements/element/selection.hpp>
#include <elements/element/composite.hpp>
#include <elements/element/traversal.hpp>
#include <elements/view.hpp>
#include <infra/assert.hpp>

namespace cycfi::elements
{
   namespace
   {
      std::vector<std::size_t> get_selected(composite_base const& c)
      {
         std::vector<std::size_t> indices;
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
            {
               if (e->is_selected())
                  indices.push_back(i);
            }
         }
         return indices;
      }

      void set_selected(composite_base& c, std::vector<std::size_t> const selection)
      {
         CYCFI_ASSERT(selection.size() == c.size(), "Error: Mismatched size.");
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
               e->select(selection[i]);
         }
      }

      void unselect_all(composite_base const& c)
      {
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
               e->select(false);
         }
      }
   }

   bool selection_list_element::click(context const& ctx, mouse_button btn)
   {
      bool r = base_type::click(ctx, btn);

      if (auto c = find_subject<composite_base*>(this))
      {
         in_context_do(ctx, *c,
            [&](context const& cctx)
            {
               auto hit = c->hit_element(cctx, btn.pos, false);
               if (hit.element_ptr)
               {
                  if (btn.down && _multi_select && (btn.modifiers & mod_shift))
                  {
                     // Process shift-select
                     r = true;
                  }
                  else if (_multi_select && (btn.modifiers & mod_action))
                  {
                     if (btn.down)
                     {
                        // Process action-select
                        if (auto e = find_element<selectable*>(hit.element_ptr))
                        {
                           e->select(!e->is_selected());
                           r = true;
                           if (e->is_selected())
                              _hook = hit.index;
                        }
                     }
                  }
                  else
                  {
                     // Process select
                     if (auto e = find_element<selectable*>(hit.element_ptr))
                     {
                        if (!btn.down)
                        {
                           unselect_all(*c);
                           e->select(true);
                           _hook = hit.index;
                        }
                        r = true;
                     }
                  }
               }
               if (r)
               {
                  ctx.view.refresh(ctx.bounds);
                  on_select(_hook);
               }
            }
         );
      }
      return r;
   }

   bool selection_list_element::key(context const& ctx, key_info k)
   {
      bool r = base_type::key(ctx, k);
      return r;
   }

   selection_list_element::indices_type
   selection_list_element::get_selection() const
   {
      if (auto c = find_subject<composite_base const*>(this))
         return get_selected(*c);
      return {};
   }

   void selection_list_element::set_selection(indices_type const& selection)
   {
      if (auto c = find_subject<composite_base*>(this))
         return set_selected(*c, selection);
   }
}

