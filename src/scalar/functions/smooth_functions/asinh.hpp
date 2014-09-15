#ifndef nomad__src__scalar__functions__smooth_functions__asinh_hpp
#define nomad__src__scalar__functions__smooth_functions__asinh_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/unary_var_node.hpp>

namespace nomad {
  
  inline double asinh(double x) { return std::asinh(x); }
  
  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>
    asinh(const var<AutodiffOrder, StrictSmoothness>& input) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    create_node<unary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    const double x = input.first_val();
    push_dual_numbers<AutodiffOrder>(std::asinh(x));
    
    push_inputs(input.dual_numbers());
    
    double d1 = 1.0 / (x * x + 1);
    double d2 = std::sqrt(d1);
    
    if (AutodiffOrder >= 1) push_partials(d2);
    if (AutodiffOrder >= 2) push_partials(-x * d1 * d2);
    if (AutodiffOrder >= 3) push_partials((-1 + 2 * x * x) * d1 * d1 * d2);

    return var<AutodiffOrder, StrictSmoothness>(next_body_idx_ - 1);
    
  }

}

#endif
