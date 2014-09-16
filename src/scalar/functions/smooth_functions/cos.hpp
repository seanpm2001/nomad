#ifndef nomad__src__scalar__functions__smooth_functions__cos_hpp
#define nomad__src__scalar__functions__smooth_functions__cos_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/autodiff/validation.hpp>

namespace nomad {
  
  inline double cos(double x) { return std::cos(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness, bool ValidateIO>
  inline var<AutodiffOrder, StrictSmoothness, ValidateIO>
    cos(const var<AutodiffOrder, StrictSmoothness, ValidateIO>& input) {
    
    if (ValidateIO) validate_input(input.first_val(), "cos");
      
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);
      
    double c = std::cos(input.first_val());
    double s = std::sin(input.first_val());
    
    push_dual_numbers<AutodiffOrder>(c);
    
    push_inputs(input.dual_numbers());
    
    if (AutodiffOrder >= 1) push_partials(-s);
    if (AutodiffOrder >= 2) push_partials(-c);
    if (AutodiffOrder >= 3) push_partials(s);

    return var<AutodiffOrder, StrictSmoothness, ValidateIO>(next_node_idx_ - 1);
    
  }

}

#endif
