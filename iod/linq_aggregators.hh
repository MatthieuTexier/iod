#pragma once

#include <type_traits>
#include <algorithm>
#include <iod/symbol.hh>
#include <iod/grammar.hh>
#include <iod/utils.hh>
#include <iod/symbols.hh>
#include <iod/linq_evaluate.hh>

namespace iod
{
  using s::_Avg;
  using s::_Avg_t;
  using s::_Sum;
  using s::_Sum_t;
  using s::_Elt;
  using s::_Cpt;

  namespace linq_internals
  {

    template <typename O, typename F, typename G>
    struct aggregator
    {
      template <typename T>
      void take(T t) { take_(t, attrs_); }
      auto result() const { return result_(attrs_); }

      O attrs_; F take_; G result_;
    };

    template <typename O, typename F, typename G>
    auto make_aggregator(O o, F f, G g)
    {
      return aggregator<O, F, G>{o, f, g};
    }

    template <typename... T>
    struct has_aggregator { enum { value = false }; };
    template <typename... T>
    struct has_aggregator<sio<T...>> { enum { value = has_aggregator<typename T::value_type...>::value }; };
    template <typename E, typename... T>
    struct has_aggregator<function_call_exp<_Avg_t, E>, T...> { enum { value = true }; };
    template <typename E, typename... T>
    struct has_aggregator<function_call_exp<_Sum_t, E>, T...> { enum { value = true }; };
    template <typename E, typename... T>
    struct has_aggregator<E, T...> { enum { value = has_aggregator<T...>::value }; };

    template <typename E, typename C>
    auto aggregate_initialize(E exp, C ctx)
    {
      typedef decltype(evaluate(exp, ctx)) M;
      return make_aggregator(D(_Elt = M()),
                             [=] (auto& t, auto& o) { o.elt = evaluate(exp, t); },
                             [] (const auto& o) { return o.elt; });
    }

    template <typename A, typename C>
    inline auto aggregate_initialize(function_call_exp<_Avg_t, A> f, C ctx)
    {
      typedef decltype((evaluate(A(), ctx) + evaluate(A(), ctx))) sum_type;
      return make_aggregator(D(_Cpt = int(0), _Sum = sum_type(0)),
                             [=] (auto& t, auto& o) { o.sum += evaluate(std::get<0>(f.args), t); o.cpt++; },
                             [] (const auto& o) { return o.sum / float(o.cpt); });
    }

    template <typename A, typename C>
    inline auto aggregate_initialize(function_call_exp<_Sum_t, A> f, C ctx)
    {
      typedef decltype((evaluate(A(), ctx) + evaluate(A(), ctx))) sum_type;
      return make_aggregator(D(_Sum = sum_type(0)),
                             [=] (auto& t, auto& o) { o.sum += evaluate(std::get<0>(f.args), t); },
                             [] (const auto& o) { return o.sum; });
    }

  }

}
