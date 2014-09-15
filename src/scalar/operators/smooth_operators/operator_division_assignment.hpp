#ifndef nomad__src__scalar__operators__smooth_operators__operator_division_assignment_hpp
#define nomad__src__scalar__operators__smooth_operators__operator_division_assignment_hpp

#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/var/derived/binary_var_node.hpp>

namespace nomad {

  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>&
    operator/=(var<AutodiffOrder, StrictSmoothness>& v1,
               const var<AutodiffOrder, StrictSmoothness>& v2) {

    const short partials_order = 3;
    const unsigned int n_inputs = 2;
    
    create_node<binary_var_node<AutodiffOrder, partials_order>>(n_inputs);
    
    double x = v1.first_val();
    double y_inv = 1.0 / v2.first_val();
    double val = x * y_inv;
    
    push_dual_numbers<AutodiffOrder>(val);
    
    push_inputs(v1.dual_numbers());
    push_inputs(v2.dual_numbers());
    
    double y_inv_n = y_inv * y_inv;
    
    if (AutodiffOrder >= 1) {
      push_partials(y_inv);
      push_partials(- val * y_inv);
    }
    if (AutodiffOrder >= 2) {
      push_partials(0);
      push_partials(-y_inv_n);
      push_partials(2 * val * y_inv_n);
    }
    if (AutodiffOrder >= 3) {
      y_inv_n *= y_inv;
      push_partials(0);
      push_partials(0);
      push_partials(2 * y_inv_n);
      push_partials(-6 * val * y_inv_n);
    }
    
    v1.set_body(next_body_idx_ - 1);
    return v1;
    
  }
  
  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>&
    operator/=(var<AutodiffOrder, StrictSmoothness>& v1,
               double y) {
    
    const short partials_order = 1;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);
    
    double x = v1.first_val();
    double y_inv = 1.0 / y;
    
    push_dual_numbers<AutodiffOrder>(x * y_inv);
    
    push_inputs(v1.dual_numbers());
    
    if (AutodiffOrder >= 1) push_partials(y_inv);
    
    v1.set_body(next_body_idx_ - 1);
    return v1;
    
  }

}
#endif
