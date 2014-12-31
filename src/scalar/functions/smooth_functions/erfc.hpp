#ifndef nomad__src__scalar__functions__smooth_functions__erfc_hpp
#define nomad__src__scalar__functions__smooth_functions__erfc_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/validation/inputs.hpp>

namespace nomad {
  
  inline double erfc(double x) { return std::erfc(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness, bool ValidateIO>
  inline var<AutodiffOrder, StrictSmoothness, ValidateIO>
    erfc(const var<AutodiffOrder, StrictSmoothness, ValidateIO>& input) {
    
    if (ValidateIO) validate_input(input.first_val(), "erfc");
      
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double x = input.first_val();
    
    try {
      push_dual_numbers<AutodiffOrder, ValidateIO>(erfc(x));
    } catch (nomad_error) {
      throw nomad_output_value_error("erfc");
    }
      
    push_inputs(input.dual_numbers());
    
    double C = - 2 * 0.56418958354776 * exp(- x * x);
    
    try {
      if (AutodiffOrder >= 1) push_partials<ValidateIO>(C);
      if (AutodiffOrder >= 2) push_partials<ValidateIO>(- 2 * x * C);
      if (AutodiffOrder >= 3) push_partials<ValidateIO>(2 * (2 * x * x - 1) * C);
    } catch (nomad_error) {
      throw nomad_output_partial_error("erfc");
    }
      
    return var<AutodiffOrder, StrictSmoothness, ValidateIO>(next_node_idx_ - 1);
    
  }

}

#endif
