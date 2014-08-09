#ifndef nomad__src__scalar__functions__smooth_functions__atanh_hpp
#define nomad__src__scalar__functions__smooth_functions__atanh_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_body.hpp>

namespace nomad {
  
  inline double atanh(double x) { return std::atanh(x); }
  
  template <short autodiff_order, bool strict_smoothness>
  inline var<autodiff_order, strict_smoothness>
    atanh(const var<autodiff_order, strict_smoothness>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
      unary_var_body<autodiff_order, partials_order>::n_partials();
    
    new unary_var_body<autodiff_order, partials_order>();

    const double x = input.first_val();
    push_dual_numbers<autodiff_order>(std::atanh(x));
    
    push_inputs(input.dual_numbers());
    
    double d1 = 1.0 / (1 - x * x);
    
    if (autodiff_order >= 1) push_partials(d1);
    if (autodiff_order >= 2) push_partials(2 * x * d1 * d1);
    if (autodiff_order >= 3) push_partials((2 + 6 * x * x) * d1 * d1 * d1);

    return var<autodiff_order, strict_smoothness>(next_body_idx_ - 1);
    
  }

}

#endif