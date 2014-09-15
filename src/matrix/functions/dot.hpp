#ifndef nomad__src__matrix__functions__dot_hpp
#define nomad__src__matrix__functions__dot_hpp

#include <Eigen/Core>

#include <src/var/var.hpp>
#include <src/var/derived/dot_var_node.hpp>

namespace nomad {
  
  template<typename DerivedA, typename DerivedB>
  inline typename
  std::enable_if<
    is_var<typename Eigen::MatrixBase<DerivedA>::Scalar>::value &&
    is_var<typename Eigen::MatrixBase<DerivedB>::Scalar>::value &&
    std::is_same<typename Eigen::MatrixBase<DerivedA>::Scalar,
                 typename Eigen::MatrixBase<DerivedB>::Scalar>::value,
    typename Eigen::MatrixBase<DerivedA>::Scalar >::type
  dot(const Eigen::MatrixBase<DerivedA>& v1,
      const Eigen::MatrixBase<DerivedB>& v2) {
    
    const short AutodiffOrder = Eigen::MatrixBase<DerivedA>::Scalar::order();
    const short StrictSmoothness = Eigen::MatrixBase<DerivedA>::Scalar::strict();

    eigen_idx_t N = v1.size();
    const nomad_idx_t n_inputs = static_cast<nomad_idx_t>(2 * N);
    
    create_node<dot_var_node<AutodiffOrder>>(n_inputs);
    
    double sum = 0;
    
    for (eigen_idx_t n = 0; n < N; ++n)
      sum += v1(n).first_val() * v2(n).first_val();

    push_dual_numbers<AutodiffOrder>(sum);
    
    for (eigen_idx_t n = 0; n < N; ++n)
      push_inputs(v1(n).dual_numbers());
      
    for (eigen_idx_t n = 0; n < N; ++n)
      push_inputs(v2(n).dual_numbers());
    
    return var<AutodiffOrder, StrictSmoothness>(next_body_idx_ - 1);
    
  }
  
}

#endif
