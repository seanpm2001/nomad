#include <gtest/gtest.h>

#include <math.h>
#include <string>

#include <src/autodiff/base_functor.hpp>
#include <src/scalar/functions.hpp>
#include <src/test/finite_difference.hpp>

template <typename T>
class fabs_func: public nomad::base_functor<T> {
public:
  T operator()(const Eigen::VectorXd& x) const {
    T v = x[0];
    return exp(fabs(v));
  }
  static std::string name() { return "fabs"; }
};

TEST(ScalarNonSmoothFunctions, Fabs) {
  Eigen::VectorXd x = Eigen::VectorXd::Ones(1);
  
  x *= 0.576;
  nomad::tests::test_function<false, false, fabs_func>(x);
  
  x *= -1;
  nomad::tests::test_function<false, false, fabs_func>(x);
}

