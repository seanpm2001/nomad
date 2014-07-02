#ifndef nomad__scalar__functions__log1p_hpp
#define nomad__scalar__functions__log1p_hpp

#include <math.h>
#include <var/var.hpp>
#include <var/derived/unary_var_body.hpp>

namespace nomad {
  
  inline double log1p(double x) { return std::log1p(x); }
  
  template <short autodiff_order>
  inline var<autodiff_order> log1p(const var<autodiff_order>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
      unary_var_body<autodiff_order, partials_order>::n_partials();
    
    new unary_var_body<autodiff_order, partials_order>();

    double val = input.first_val();
    
    push_dual_numbers<autodiff_order>(log1p(val));
    
    push_inputs(input.dual_numbers());
    
    double val_inv = 1.0 / (1 + val);
    
    if (autodiff_order >= 1) push_partials(val = val_inv);
    if (autodiff_order >= 2) push_partials(val *= - val_inv);
    if (autodiff_order >= 3) push_partials(val *= - 2.0 * val_inv);

    return var<autodiff_order>(next_body_idx_ - 1);
    
  }

}

#endif
