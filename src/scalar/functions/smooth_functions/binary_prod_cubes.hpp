#ifndef nomad__src__scalar__functions__smooth_functions__binary_prod_cubes_hpp
#define nomad__src__scalar__functions__smooth_functions__binary_prod_cubes_hpp

#include <math.h>
#include <src/var/var.hpp>
#include <src/var/derived/binary_var_node.hpp>

namespace nomad {
  
  inline double binary_prod_cubes(double x, double y) {
    return x * x * x * y * y * y;
  }
  
  template <short AutodiffOrder, bool StrictSmoothness>
  inline var<AutodiffOrder, StrictSmoothness>
    binary_prod_cubes(const var<AutodiffOrder, StrictSmoothness>& v1,
                      const var<AutodiffOrder, StrictSmoothness>& v2) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 2;
    
    create_node<binary_var_node<AutodiffOrder, partials_order>>(n_inputs);

    double x = v1.first_val();
    double y = v2.first_val();
    
    push_dual_numbers<AutodiffOrder>(binary_prod_cubes(x, y));
    
    push_inputs(v1.dual_numbers());
    push_inputs(v2.dual_numbers());
    
    if (AutodiffOrder >= 1) {
      push_partials(3 * x * x * y * y * y);
      push_partials(3 * x * x * x * y * y);
    }
    if (AutodiffOrder >= 2) {
      push_partials(6 * x * y * y * y);
      push_partials(9 * x * x * y * y);
      push_partials(6 * x * x * x * y);
    }
    if (AutodiffOrder >= 3) {
      push_partials(6 * y * y * y);
      push_partials(18 * x * y * y);
      push_partials(18 * x * x * y);
      push_partials(6 * x * x * x);
    }

    return var<AutodiffOrder, StrictSmoothness>(next_body_idx_ - 1);
    
  }

}

#endif
