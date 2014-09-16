#ifndef nomad__src__scalar__functions__nonsmooth_functions__floor_hpp
#define nomad__src__scalar__functions__nonsmooth_functions__floor_hpp

#include <math.h>
#include <type_traits>

#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/autodiff/validation.hpp>

namespace nomad {
  
  inline double floor(double x) { return std::floor(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness, bool ValidateIO>
  inline typename std::enable_if<!StrictSmoothness, var<AutodiffOrder, StrictSmoothness, ValidateIO> >::type
    floor(const var<AutodiffOrder, StrictSmoothness, ValidateIO>& input) {
    
    if (ValidateIO) validate_input(input.first_val(), "floor");
      
    const short partials_order = 0;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    try {
      push_dual_numbers<AutodiffOrder, ValidateIO>(ceil(input.first_val()));
    } catch(nomad_error& e) {
      throw nomad_output_value_error("floor");
    }
      
    push_inputs(input.dual_numbers());

    return var<AutodiffOrder, StrictSmoothness, ValidateIO>(next_node_idx_ - 1);
    
  }

}

#endif
