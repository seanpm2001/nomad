#ifndef nomad__autodiff__second_order_hpp
#define nomad__autodiff__second_order_hpp

#include <iomanip>
#include <string>
#include <Eigen/Core>

#include <var/var.hpp>
#include <autodiff/exceptions.hpp>
#include <autodiff/first_order.hpp>

namespace nomad {

  template<class T_var>
  void second_order_forward_val(const T_var& v) {
    for (index_t i = 1; i <= v.body(); ++i)
      var_bodies_[i].second_order_forward_val();
  }
  
  template<class T_var>
  void second_order_reverse_adj(const T_var& v) {
    var_bodies_[v.body()].second_grad() = 0;
    for (index_t i = v.body(); i > 0; --i)
      var_bodies_[i].second_order_reverse_adj();
  }

  template <typename F>
  void hessian(const F& functional,
               const Eigen::VectorXd& x,
               double& f,
               Eigen::VectorXd& g,
               Eigen::MatrixXd& H) {
    
    reset();
    
    size_t d = x.size();
    
    try {
      
      auto f_var = functional(x);
      
      if (f_var.order() < 2) {
        reset();
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the hessian");
      }
      
      f = f_var.first_val();
      
      // First-order
      first_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        g(i) = var_bodies_[i + 1].first_grad();
      
      // Second-order
      for (size_t i = 0; i < d; ++i) {
        
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].second_val() = static_cast<double>(i == j);
        
        second_order_forward_val(f_var);
        second_order_reverse_adj(f_var);
        
        for (size_t j = 0; j < d; ++j)
          H(i, j) = var_bodies_[j + 1].second_grad();
        
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "Hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    reset();
    
  }
  
  template <typename F>
  void hessian(const F& functional,
               const Eigen::VectorXd& x,
               Eigen::MatrixXd& H) {
    double f;
    Eigen::VectorXd g(x.size());
    try {
      hessian(functional, x, f, g, H);
    } catch (std::runtime_error& e) {
      throw e;
    }
  }
  
  template <typename F>
  void finite_diff_hessian(const F& functional,
                           const Eigen::VectorXd& x,
                           Eigen::MatrixXd& H,
                           const double epsilon = 1e-6) {
    
    size_t d = x.size();
    
    Eigen::VectorXd x_dynam(x);
    Eigen::VectorXd g_auto(d);
    
    try {
      
      auto f_var = functional(x);
      reset();
      
      if (f_var.order() < 1) {
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the finite difference hessian");
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "Finite difference hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    for (size_t i = 0; i < d; ++i) {
      
      Eigen::VectorXd g_diff = Eigen::VectorXd::Zero(d);
      
      x_dynam(i) += epsilon;
      gradient(functional, x_dynam, g_auto);
      g_diff += g_auto;
      
      x_dynam(i) -= 2.0 * epsilon;
      gradient(functional, x_dynam, g_auto);
      g_diff -= g_auto;
      
      x_dynam(i) += epsilon;
      g_diff /= 2.0 * epsilon;
      
      H.col(i) = g_diff;
      
    }
    
  }
  
  template <typename F>
  void test_hessian(const F& functional,
                    const Eigen::VectorXd& x,
                    const double epsilon = 1e-6) {
    
    size_t d = x.size();
    
    Eigen::MatrixXd auto_H(x.size(), x.size());
    try {
      hessian(functional, x, auto_H);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Test" << std::endl;
      throw e;
    }
    
    Eigen::MatrixXd diff_H(x.size(), x.size());
    try {
      finite_diff_hessian(functional, x, diff_H, epsilon);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Test" << std::endl;
      throw e;
    }
    
    std::cout.precision(6);
    int width = 12;
    int n_column = 5;
    
    std::cout << "Hessian Test:" << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "Row"
              << std::setw(width) << std::left << "Column"
              << std::setw(width) << std::left << "Automatic"
              << std::setw(width) << std::left << "Finite"
              << std::setw(width) << std::left << "Delta / "
              << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "(i)"
              << std::setw(width) << std::left << "(j)"
              << std::setw(width) << std::left << "Derivative"
              << std::setw(width) << std::left << "Difference"
              << std::setw(width) << std::left << "Stepsize^{2}"
              << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    
    for (size_t i = 0; i < d; ++i) {
      for (size_t j = 0; j < d; ++j) {
        std::cout << "    "
                  << std::setw(width) << std::left << i
                  << std::setw(width) << std::left << j
                  << std::setw(width) << std::left << auto_H(i, j)
                  << std::setw(width) << std::left << diff_H(i, j)
                  << std::setw(width) << std::left
                  << (auto_H(i, j) - diff_H(i, j)) / (epsilon * epsilon)
                  << std::endl;
      }
    }
    
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << std::endl;
    
  }
  
  template <typename F>
  void hessian_dot_vector(const F& functional,
                          const Eigen::VectorXd& x,
                          const Eigen::VectorXd& v,
                          double& f,
                          Eigen::VectorXd& g,
                          Eigen::VectorXd& hessian_dot_v) {
    
    reset();
    
    size_t d = x.size();
    
    try {
      
      auto f_var = functional(x);
      
      if (f_var.order() < 2) {
        reset();
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the hessian dot vector");
      }
      
      f = f_var.first_val();
      
      // First-order
      first_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        g(i) = var_bodies_[i + 1].first_grad();
      
      // Second-order
      for (size_t i = 0; i < d; ++i)
        var_bodies_[i + 1].second_val() = v(i);
      
      second_order_forward_val(f_var);
      second_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        hessian_dot_v(i) = var_bodies_[i + 1].second_grad();
      
    } catch (partial_fail_ex& e) {
      std::cout << "Hessian dot vector cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
  
    reset();
    
  }
  
  template <typename F>
  void hessian_dot_vector(const F& functional,
                          const Eigen::VectorXd& x,
                          const Eigen::VectorXd& v,
                          Eigen::VectorXd& hessian_dot_v) {
    double f;
    Eigen::VectorXd g(x.size());
    try {
      hessian_dot_vector(functional, x, v, f, g, hessian_dot_v);
    } catch (std::runtime_error& e) {
      throw e;
    }
  }
  
  template <typename F>
  void test_hessian_dot_vector(const F& functional,
                               const Eigen::VectorXd& x,
                               const Eigen::VectorXd& v) {
    
    Eigen::MatrixXd H_auto(x.size(), x.size());
    try {
      hessian(functional, x, H_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Doc Vector Test" << std::endl;
      throw e;
    }
    
    Eigen::VectorXd H_dot_v_auto(x.size());
    try {
      hessian_dot_vector(functional, x, v, H_dot_v_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Doc Vector Test" << std::endl;
      throw e;
    }
    
    std::cout.precision(6);
    int width = 12;
    int n_column = 3;
    
    std::cout << "Hessian Dot Vector Test:" << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "Component"
              << std::setw(width) << std::left << "Automatic"
              << std::setw(width) << std::left << "Exact"
              << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "(i)"
              << std::setw(width) << std::left << "Derivative"
              << std::setw(width) << std::left << ""
              << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    
    Eigen::VectorXd H_dot_v = H_auto * v;
    
    for (size_t i = 0; i < x.size(); ++i) {
      
      std::cout << "    "
                << std::setw(width) << std::left << i
                << std::setw(width) << std::left << H_dot_v_auto(i)
                << std::setw(width) << std::left << H_dot_v(i)
                << std::endl;
      
    }
    
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << std::endl;
    
  }
  
  template <typename F>
  void trace_matrix_times_hessian(const F& functional,
                                  const Eigen::VectorXd& x,
                                  const Eigen::MatrixXd& M,
                                  double& f,
                                  Eigen::VectorXd& g,
                                  double& trace_m_times_h) {
    
    reset();
    
    size_t d = x.size();
    
    try {
      
      auto f_var = functional(x);
      
      if (f_var.order() < 2) {
        reset();
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the trace of a matrix times the Hessian");
      }
      
      f = f_var.first_val();
      
      // First-order
      first_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        g(i) = var_bodies_[i + 1].first_grad();
      
      // Second-order
      trace_m_times_h = 0;
      
      for (size_t i = 0; i < d; ++i) {
        
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].second_val() = M(j, i);
        
        second_order_forward_val(f_var);
        second_order_reverse_adj(f_var);
        
        trace_m_times_h += var_bodies_[i + 1].second_grad();
        
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "Trace of a matrix times the Hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    reset();
    
  }
  
  template <typename F>
  void trace_matrix_times_hessian(const F& functional,
                                  const Eigen::VectorXd& x,
                                  const Eigen::MatrixXd& M,
                                  double& trace_m_times_h) {
    double f;
    Eigen::VectorXd g(x.size());
    try {
      trace_matrix_times_hessian(functional, x, M, f, g, trace_m_times_h);
    } catch (std::runtime_error& e) {
      throw e;
    }
  }
  
  template <typename F>
  void test_trace_matrix_times_hessian(const F& functional,
                                       const Eigen::VectorXd& x,
                                       const Eigen::MatrixXd& M) {
    
    Eigen::MatrixXd H_auto(x.size(), x.size());
    try {
      hessian(functional, x, H_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Trace Matrix Times Hessian Test" << std::endl;
      throw e;
    }
    
    double trace_m_times_h = (M * H_auto).trace();
    
    double trace_m_times_h_auto;
    try {
      trace_matrix_times_hessian(functional, x, M, trace_m_times_h_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Trace Matrix Times Hessian Test" << std::endl;
      throw e;
    }
    
    std::cout.precision(6);
    int width = 12;
    int n_column = 2;
    
    std::cout << "Trace Matrix Times Hessian Test:" << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "Automatic"
              << std::setw(width) << std::left << "Exact"
              << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "Derivative"
              << std::setw(width) << std::left << ""
              << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    
    std::cout << "    "
              << std::setw(width) << std::left << trace_m_times_h_auto
              << std::setw(width) << std::left << trace_m_times_h
              << std::endl;
    
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << std::endl;
    
  }
  
}

#endif