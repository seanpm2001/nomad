#ifndef nomad__src__scalar__functions__smooth_functions__log2_hpp
#define nomad__src__scalar__functions__smooth_functions__log2_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/autodiff/validation.hpp>

namespace nomad {
  
  inline double log2(double x) { return std::log2(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness, bool ValidateIO>
  inline var<AutodiffOrder, StrictSmoothness, ValidateIO>
    log2(const var<AutodiffOrder, StrictSmoothness, ValidateIO>& input) {
    
    if (ValidateIO) {
      double val = input.first_val();
      validate_input(val, "log2");
      validate_lower_bound(val, 0, "log2");
    }
      
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double val = input.first_val();
    
    push_dual_numbers<AutodiffOrder>(log2(val));
    
    push_inputs(input.dual_numbers());
    
    double val_inv = 1.0 / val;
    
    if (AutodiffOrder >= 1) push_partials(val = val_inv * 1.44269504088896);
    if (AutodiffOrder >= 2) push_partials(val *= - val_inv);
    if (AutodiffOrder >= 3) push_partials(val *= - 2.0 * val_inv);

    return var<AutodiffOrder, StrictSmoothness, ValidateIO>(next_body_idx_ - 1);
    
  }

}

#endif
