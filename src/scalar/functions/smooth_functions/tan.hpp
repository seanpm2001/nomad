#ifndef nomad__src__scalar__functions__smooth_functions__tan_hpp
#define nomad__src__scalar__functions__smooth_functions__tan_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>

namespace nomad {
  
  inline double tan(double x) { return std::tan(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>
    tan(const var<AutodiffOrder, StrictSmoothness>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double t = std::tan(input.first_val());
    
    push_dual_numbers<AutodiffOrder>(t);
    
    push_inputs(input.dual_numbers());
    
    double sec2 = 1.0 / std::cos(input.first_val());
    sec2 *= sec2;
    
    if (AutodiffOrder >= 1) push_partials(sec2);
    if (AutodiffOrder >= 2) push_partials(2 * sec2 * t);
    if (AutodiffOrder >= 3) push_partials(2 * sec2 * sec2 + 4 * sec2 * t * t);

    return var<AutodiffOrder, StrictSmoothness>(next_body_idx_ - 1);
    
  }

}

#endif
