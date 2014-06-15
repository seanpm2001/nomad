#ifndef nomad__scalar__operators__operator_unary_negative_hpp
#define nomad__scalar__operators__operator_unary_negative_hpp

#include <var/var.hpp>

namespace nomad {

  template <short autodiff_order>
  inline var<autodiff_order> operator-(const var<autodiff_order>& v1) {

    const short partials_order = 1;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
      var_body<autodiff_order, partials_order>::n_partials(n_inputs);
    
    new var_body<autodiff_order, partials_order>(n_inputs);
    
    push_dual_numbers<autodiff_order>(-v1.first_val());
    
    push_inputs(v1.dual_numbers());
    
    if (autodiff_order >= 1) push_partials(-1);
    
    return var<autodiff_order>(next_body_idx_ - 1);
    
  }

}
#endif