#ifndef nomad__src__scalar__functions__smooth_functions__inv_square_hpp
#define nomad__src__scalar__functions__smooth_functions__inv_square_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_body.hpp>

namespace nomad {
  
  inline double inv_square(double x) {
    return 1.0 / (x * x);
  }
  
  template <short autodiff_order, bool strict_smoothness>
  inline var<autodiff_order, strict_smoothness>
    inv_square(const var<autodiff_order, strict_smoothness>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
      unary_var_body<autodiff_order, partials_order>::n_partials();
    
    new unary_var_body<autodiff_order, partials_order>();

    double d = 1.0 / input.first_val();
    double val = d * d;
    
    push_dual_numbers<autodiff_order>(val);
    
    push_inputs(input.dual_numbers());
    
    if (autodiff_order >= 1) push_partials(val *= -2 * d);
    if (autodiff_order >= 2) push_partials(val *= -3 * d);
    if (autodiff_order >= 3) push_partials(val *= -4 * d);

    return var<autodiff_order, strict_smoothness>(next_body_idx_ - 1);
    
  }

}

#endif