#ifndef nomad__scalar__functions__pow_hpp
#define nomad__scalar__functions__pow_hpp

#include <math.h>
#include <var/var.hpp>
#include <var/derived/unary_var_body.hpp>
#include <var/derived/binary_var_body.hpp>

namespace nomad {
  
  inline double pow(double x, double y) { return std::pow(x, y); }
  
  template <short autodiff_order>
  inline var<autodiff_order> pow(const var<autodiff_order>& v1,
                                 const var<autodiff_order>& v2) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 2;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
      binary_var_body<autodiff_order, partials_order>::n_partials();
    
    new binary_var_body<autodiff_order, partials_order>();
    
    double x = v1.first_val();
    double y = v2.first_val();
    double val = std::pow(x, y);
    
    push_dual_numbers<autodiff_order>(val);
    
    push_inputs(v1.dual_numbers());
    push_inputs(v2.dual_numbers());
    
    double lx = std::log(x);
    
    if (autodiff_order >= 1) {
      push_partials(val * y / x);
      push_partials(val * lx);
    }
    if (autodiff_order >= 2) {
      push_partials( y * (y - 1.0) * val / (x * x) );
      push_partials( (1 + y * lx) * val / x );
      push_partials( lx * lx * val );
    }
    if (autodiff_order >= 3) {
      push_partials((y - 2.0) * (y - 1.0) * y * val / (x  * x * x) );
      push_partials( ((y - 1.0) * y * lx + 2 * y - 1.0) * val / (x * x) );
      push_partials( lx * (2.0 + y * lx) * val / x );
      push_partials( lx * lx * lx * val );
    }
    
    return var<autodiff_order>(next_body_idx_ - 1);
    
  }
  
  template <short autodiff_order>
  inline var<autodiff_order> pow(double x,
                                 const var<autodiff_order>& v2) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
    unary_var_body<autodiff_order, partials_order>::n_partials();
    
    new unary_var_body<autodiff_order, partials_order>();
    
    double y = v2.first_val();
    double val = std::pow(x, y);
    
    push_dual_numbers<autodiff_order>(val);
    
    push_inputs(v2.dual_numbers());
    
    double lx = std::log(x);
    
    if (autodiff_order >= 1) push_partials(val * lx);
    if (autodiff_order >= 2) push_partials(lx * lx * val);
    if (autodiff_order >= 3) push_partials(lx * lx * lx * val);

    return var<autodiff_order>(next_body_idx_ - 1);
    
  }
  
  template <short autodiff_order>
  inline var<autodiff_order> pow(const var<autodiff_order>& v1,
                                 double y) {
    
    const short partials_order = 3;
    const unsigned int n_inputs = 1;
    
    next_inputs_delta = n_inputs;
    next_partials_delta =
    unary_var_body<autodiff_order, partials_order>::n_partials();
    
    new unary_var_body<autodiff_order, partials_order>();
    
    double x = v1.first_val();
    double val = std::pow(x, y);
    
    push_dual_numbers<autodiff_order>(val);
    
    push_inputs(v1.dual_numbers());
    
    double lx = std::log(x);
    
    if (autodiff_order >= 1) push_partials(val * y / x);
    if (autodiff_order >= 2) push_partials( y * (y - 1.0) * val / (x * x) );
    if (autodiff_order >= 3) push_partials((y - 2.0) * (y - 1.0) * y * val / (x  * x * x) );
    
    return var<autodiff_order>(next_body_idx_ - 1);
    
  }

}

#endif
