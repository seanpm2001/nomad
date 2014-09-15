#ifndef nomad__src__scalar__functions__smooth_functions__lgamma_hpp
#define nomad__src__scalar__functions__smooth_functions__lgamma_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>
#include <src/scalar/functions/smooth_functions/polygamma.hpp>

namespace nomad {
  
  inline double lgamma(double x) { return std::lgamma(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>
    lgamma(const var<AutodiffOrder, StrictSmoothness>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double val = input.first_val();
    
    push_dual_numbers<AutodiffOrder>(lgamma(val));
    
    push_inputs(input.dual_numbers());
    
    if (AutodiffOrder >= 1) push_partials(digamma(val));
    if (AutodiffOrder >= 2) push_partials(trigamma(val));
    if (AutodiffOrder >= 3) push_partials(quadrigamma(val));

    return var<AutodiffOrder, StrictSmoothness>(next_body_idx_ - 1);
    
  }

}

#endif
