#ifndef nomad__src__scalar__functions__smooth_functions__exp2_hpp
#define nomad__src__scalar__functions__smooth_functions__exp2_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/autodiff/validation.hpp>

namespace nomad {
  
  inline double exp2(double x) { return std::exp2(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness, bool ValidateIO>
  inline var<AutodiffOrder, StrictSmoothness, ValidateIO>
    exp2(const var<AutodiffOrder, StrictSmoothness, ValidateIO>& input) {
    
    if (ValidateIO) validate_input(input.first_val(), "exp2");
      
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double val = exp2(input.first_val());
    
    push_dual_numbers<AutodiffOrder>(val);
    
    push_inputs(input.dual_numbers());
    
    double log2 = 0.693147180559945;
    
    if (AutodiffOrder >= 1) push_partials(val *= log2);
    if (AutodiffOrder >= 2) push_partials(val *= log2);
    if (AutodiffOrder >= 3) push_partials(val *= log2);

    return var<AutodiffOrder, StrictSmoothness, ValidateIO>(next_body_idx_ - 1);
    
  }

}

#endif
