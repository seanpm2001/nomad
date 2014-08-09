#include <gtest/gtest.h>

#include <math.h>
#include <string>

#include <src/autodiff/base_functor.hpp>
#include <src/scalar/functions.hpp>
#include <src/scalar/operators.hpp>
#include <src/test/finite_difference.hpp>

template <typename T>
class operator_greater_than_vv_func: public nomad::base_functor<T> {
public:
  T operator()(const Eigen::VectorXd& x) const {
    T v1 = x[0];
    T v2 = x[1];
    if (v1 > v2)
      return exp(v1);
    else
      return exp(v2);
  }
  static std::string name() { return "operator_greater_than_vv"; }
};

template <typename T>
class operator_greater_than_vd_func: public nomad::base_functor<T> {
public:
  T operator()(const Eigen::VectorXd& x) const {
    T v = x[0];
    if (v > 1.0)
      return exp(-v);
    else
      return exp(v);
  }
  static std::string name() { return "operator_greater_than_vd"; }
};

template <typename T>
class operator_greater_than_dv_func: public nomad::base_functor<T> {
public:
  T operator()(const Eigen::VectorXd& x) const {
    T v = x[0];
    if (1.0 > v)
      return exp(v);
    else
      return exp(-v);
  }
  static std::string name() { return "operator_greater_than_dv"; }
};


TEST(ScalarNonSmoothOperators, OperatorGreaterThan) {
  Eigen::VectorXd x1 = Eigen::VectorXd::Ones(2);
  x1[0] = 1.5;
  x1[1] = 0.5;
  
  nomad::tests::test_function<false, operator_greater_than_vv_func>(x1);
  
  x1[0] = 0.5;
  x1[1] = 1.5;
  nomad::tests::test_function<false, operator_greater_than_vv_func>(x1);
  
  Eigen::VectorXd x2 = Eigen::VectorXd::Ones(1);
  
  x2[0] = 1.5;
  nomad::tests::test_function<false, operator_greater_than_vd_func>(x2);
  nomad::tests::test_function<false, operator_greater_than_dv_func>(x2);
  
  x2[0] = 0.5;
  nomad::tests::test_function<false, operator_greater_than_vd_func>(x2);
  nomad::tests::test_function<false, operator_greater_than_dv_func>(x2);
}