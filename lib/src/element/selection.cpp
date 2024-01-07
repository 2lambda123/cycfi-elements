/*=================================================================================================
   Copyright (c) 2016-2024 Joel de Guzman

   Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
=================================================================================================*/
#include <elements/element/selection.hpp>
#include <elements/element/composite.hpp>
#include <elements/element/traversal.hpp>
#include <elements/view.hpp>

namespace cycfi::elements
{
   namespace detail
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

      void select_none(composite_base const& c)
      {
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
               e->select(false);
         }
      }

      void set_selected(composite_base& c, std::vector<std::size_t> const selection)
      {
         select_none(c);
         for (std::size_t i : selection)
         {
            if (i > c.size())
               continue; // Ignore out of bounds indices
            if (auto e = find_element<selectable*>(&c.at(i)))
               e->select(true);
         }
      }

      void select_all(composite_base const& c)
      {
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
               e->select(true);
         }
      }

      bool select(composite_base const& c, composite_base::hit_info const& hit, int& hook)
      {
         if (auto e = find_element<selectable*>(hit.element_ptr))
         {
            select_none(c);
            e->select(true);
            hook = hit.index;
            return true;
         }
         return false;
      }

      std::size_t count_selected(composite_base const& c)
      {
         std::size_t n = 0;
         for (std::size_t i = 0; i != c.size(); ++i)
         {
            if (auto e = find_element<selectable*>(&c.at(i)))
            {
               if (e->is_selected())
                  ++n;
            }
         }
         return n;
      }

      bool multi_select(composite_base const& c, composite_base::hit_info const& hit, int& hook)
      {
         if (auto e = find_element<selectable*>(hit.element_ptr))
         {
            e->select(!e->is_selected());
            if (e->is_selected())
               hook = e->is_selected()? hit.index : -1;
            if (count_selected(c) == 0)
               hook = -1;
            return true;
         }
         return false;
      }

      bool shift_select(composite_base const& c, composite_base::hit_info const& hit, int hook)
      {
         if (auto e = find_element<selectable*>(hit.element_ptr))
         {
            hook = std::max(hook, 0);
            auto from = std::min(hook, hit.index);
            auto to = std::max(hook, hit.index);
            select_none(c);

            for (int i = from; i <= to; ++i)
            {
               if (auto e = find_element<selectable*>(&c.at(i)))
                  e->select(true);
            }
            return true;
         }
         return false;
      }
   }

   using namespace detail;

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
                  if (_multi_select && (btn.modifiers & mod_action))
                  {
                     // Process action-select
                     if (btn.down)
                        r = multi_select(*c, hit, _hook);
                  }
                  else if (_multi_select && (btn.modifiers & mod_shift))
                  {
                     // Process shift-select
                     if (btn.down)
                        r = shift_select(*c, hit, _hook);
                  }
                  else
                  {
                     // Process select
                     if (!btn.down)
                        select(*c, hit, _hook);
                     r = true;
                  }
               }
               if (r && _hook)
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
      if (k.action == key_action::press || k.action == key_action::repeat)
      {
         switch (k.key)
         {
            case key_code::a:
               if (k.modifiers & mod_action)
               {
                  if (auto c = find_subject<composite_base*>(this))
                  {
                     detail::select_all(*c);
                     ctx.view.refresh();
                     return true;
                  }
               }
               break;

            default:
               break;
         }
      }
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

   int selection_list_element::get_hook() const
   {
      return _hook;
   }

   void selection_list_element::select_all()
   {
      if (auto c = find_subject<composite_base*>(this))
      {
         detail::select_all(*c);
         on_select(_hook);
      }
   }

   void selection_list_element::select_none()
   {
      if (auto c = find_subject<composite_base*>(this))
      {
         detail::select_none(*c);
         _hook = -1;
         on_select(_hook);
      }
   }
}

