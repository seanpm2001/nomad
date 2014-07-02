#ifndef nomad__scalar__functions__Phi_approx_hpp
#define nomad__scalar__functions__Phi_approx_hpp

#include <var/var.hpp>
#include <scalar/functions/inv_logit.hpp>
#include <scalar/functions/pow.hpp>

namespace nomad {
  
  // Approximation of the unit normal CDF for variables.
  // http://www.jiem.org/index.php/jiem/article/download/60/27
  
  inline double Phi_approx(double x) {
    return inv_logit(0.07056 * pow(x, 3.0) + 1.5976 * x);
  }
  
  template <short autodiff_order>
  inline var<autodiff_order> Phi_approx(const var<autodiff_order>& input) {
    return inv_logit(0.07056 * pow(input, 3.0) + 1.5976 * input);
    
  }

}

#endif
