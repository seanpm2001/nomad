#ifndef nomad__autodiff__third_order_hpp
#define nomad__autodiff__third_order_hpp

#include <iomanip>
#include <string>
#include <Eigen/Core>

#include <var/var.hpp>
#include <autodiff/exceptions.hpp>
#include <autodiff/first_order.hpp>
#include <autodiff/second_order.hpp>

namespace nomad {

  template<class T_var>
  void third_order_forward_val(const T_var& v) {
    for (index_t i = 1; i <= v.body(); ++i)
      var_bodies_[i].third_order_forward_val();
  }
  
  template<class T_var>
  void third_order_reverse_adj(const T_var& v) {
    var_bodies_[v.body()].third_grad() = 0;
    var_bodies_[v.body()].fourth_grad() = 0;
    for (index_t i = v.body(); i > 0; --i)
      var_bodies_[i].third_order_reverse_adj();
  }

  template <typename F>
  void grad_hessian(const F& functional,
                    const Eigen::VectorXd& x,
                    double& f,
                    Eigen::VectorXd& g,
                    Eigen::MatrixXd& H,
                    Eigen::MatrixXd& grad_H) {
    
    reset();
    
    size_t d = x.size();
    
    try {
      
      auto f_var = functional(x);

      if (f_var.order() < 3) {
        reset();
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the gradient of the Hessian");
      }
      
      f = f_var.first_val();
      
      // First-order
      first_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        g(i) = var_bodies_[i + 1].first_grad();
      
      Eigen::VectorXd v(d);
      
      for (size_t i = 0; i < d; ++i) {
        
        // Second-order
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].second_val() = static_cast<double>(i == j);
        
        second_order_forward_val(f_var);
        second_order_reverse_adj(f_var);
        
        for (size_t j = 0; j < d; ++j)
          v(j) = var_bodies_[j + 1].second_grad();
        
        H.col(i) = v;
        
        // Third-order
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].fourth_val() = 0;
        
        for (size_t k = 0; k <= i; ++k) {
          
          for (size_t j = 0; j < d; ++j)
            var_bodies_[j + 1].third_val() = static_cast<double>(k == j);
          
          third_order_forward_val(f_var);
          
          for (size_t j = 0; j < d; ++j)
            v(j) = var_bodies_[j + 1].fourth_grad();
          
          /*
          for (size_t j = 1; j < next_body_idx_; ++j) {
            std::cout << "Node " << j << std::endl;
            var_bodies_[j].print<3>(&std::cout);
          }
          */
          
          third_order_reverse_adj(f_var);
          
          for (size_t j = 0; j < d; ++j)
            v(j) = var_bodies_[j + 1].fourth_grad();
          
          grad_H.block(0, i * d, d, d).col(k) = v;
          grad_H.block(0, k * d, d, d).col(i) = v;
          
        }
        
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "Gradient of the Hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    reset();
    
  }
  
  template <typename F>
  void grad_hessian(const F& functional,
                    const Eigen::VectorXd& x,
                    Eigen::MatrixXd& grad_H) {
    double f;
    Eigen::VectorXd g(x.size());
    Eigen::MatrixXd H(x.size(), x.size());
    try {
      grad_hessian(functional, x, f, g, H, grad_H);
    } catch (std::runtime_error& e) {
      throw e;
    }
  }
  
  template <typename F>
  void finite_diff_grad_hessian(const F& functional,
                                const Eigen::VectorXd& x,
                                Eigen::MatrixXd& grad_H,
                                const double epsilon = 1e-6) {
    size_t d = x.size();
    
    Eigen::VectorXd x_dynam(x);
    Eigen::MatrixXd H_diff(d, d);
    Eigen::MatrixXd H_auto(d, d);
    
    try {
      
      auto f_var = functional(x);
      reset();
      
      if (f_var.order() < 2) {
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the finite difference grad hessian");
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "Finite difference grad hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    for (size_t k = 0; k < d; ++k) {
      
      H_diff.setZero();
      
      x_dynam(k) += epsilon;
      hessian(functional, x_dynam, H_auto);
      H_diff += H_auto;
      
      x_dynam(k) -= 2.0 * epsilon;
      hessian(functional, x_dynam, H_auto);
      H_diff -= H_auto;
      
      x_dynam(k) += epsilon;
      H_diff /= 2.0 * epsilon;
      
      grad_H.block(0, k * d, d, d) = H_diff;
      
    }
    
  }
  
  template <typename F>
  void test_grad_hessian(const F& functional,
                         const Eigen::VectorXd& x,
                         const double epsilon = 1e-6) {
    
    size_t d = x.size();
    
    Eigen::MatrixXd auto_grad_H(d, d * d);
    try {
      grad_hessian(functional, x, auto_grad_H);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Gradient Test" << std::endl;
      throw e;
    }
    
    Eigen::MatrixXd diff_grad_H(d, d * d);
    try {
      finite_diff_grad_hessian(functional, x, diff_grad_H, epsilon);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Hessian Gradient Test" << std::endl;
      throw e;
    }
    
    std::cout.precision(6);
    int width = 12;
    int n_column = 6;
    
    std::cout << "Hessian Gradient Test:" << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "Component"
              << std::setw(width) << std::left << "Row"
              << std::setw(width) << std::left << "Column"
              << std::setw(width) << std::left << "Automatic"
              << std::setw(width) << std::left << "Finite"
              << std::setw(width) << std::left << "Delta /"
              << std::endl;
    std::cout << "    "
              << std::setw(width) << std::left << "(i)"
              << std::setw(width) << std::left << "(j)"
              << std::setw(width) << std::left << "(k)"
              << std::setw(width) << std::left << "Derivative"
              << std::setw(width) << std::left << "Difference"
              << std::setw(width) << std::left << "Stepsize^{2}"
              << std::endl;
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    
    for (size_t k = 0; k < d; ++k) {
      for (size_t i = 0; i < d; ++i) {
        for (size_t j = 0; j < d; ++j) {
          std::cout << "    "
                    << std::setw(width) << std::left << k
                    << std::setw(width) << std::left << i
                    << std::setw(width) << std::left << j
                    << std::setw(width) << std::left
                    << auto_grad_H.block(0, k * d, d, d)(i, j)
                    << std::setw(width) << std::left
                    << diff_grad_H.block(0, k * d, d, d)(i, j)
                    << std::setw(width) << std::left
                    << (auto_grad_H.block(0, k * d, d, d)(i, j) - diff_grad_H.block(0, k * d, d, d)(i, j))
                       / (epsilon * epsilon)
                    << std::endl;
        }
      }
    }
    
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << std::endl;
    
  }
  
  template <typename F>
  void grad_trace_matrix_times_hessian(const F& functional,
                                       const Eigen::VectorXd& x,
                                       const Eigen::MatrixXd& M,
                                       double& f,
                                       Eigen::VectorXd& g,
                                       Eigen::MatrixXd& H,
                                       Eigen::VectorXd& grad_trace_m_times_h) {
    
    reset();
    
    size_t d = x.size();
    
    try {
      
      auto f_var = functional(x);
      
      if (f_var.order() < 3) {
        reset();
        throw autodiff_fail_ex("Autodiff order " + std::to_string(f_var.order())
                               + " insufficient for computing the gradient of the trace of a matrix times the Hessian");
      }
      
      f = f_var.first_val();
      
      // First-order
      first_order_reverse_adj(f_var);
      
      for (size_t i = 0; i < d; ++i)
        g(i) = var_bodies_[i + 1].first_grad();
      
      Eigen::VectorXd v(d);
      grad_trace_m_times_h.setZero();
      
      for (size_t i = 0; i < d; ++i) {
        
        // Second-order
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].second_val() = static_cast<double>(i == j);
        
        second_order_forward_val(f_var);
        second_order_reverse_adj(f_var);
        
        for (size_t j = 0; j < d; ++j)
          v(j) = var_bodies_[j + 1].second_grad();
        
        H.col(i) = v;
        
        // Third-order
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].fourth_val() = 0;
        
        for (size_t j = 0; j < d; ++j)
          var_bodies_[j + 1].third_val() = M(j, i);
        
        third_order_forward_val(f_var);
        third_order_reverse_adj(f_var);
        
        for (size_t j = 0; j < d; ++j)
          v(j) = var_bodies_[j + 1].fourth_grad();
        
        grad_trace_m_times_h += v;
        
      }
      
    } catch (partial_fail_ex& e) {
      std::cout << "The gradient of the trace of a matrix times the Hessian cannot be computed -- " << std::endl;
      std::cout << e.what() << std::endl;
      reset();
      throw e;
    }
    
    reset();
    
  }
  
  template <typename F>
  void grad_trace_matrix_times_hessian(F& functional,
                                       const Eigen::VectorXd& x,
                                       const Eigen::MatrixXd& M,
                                       Eigen::VectorXd& grad_trace_m_times_h) {
    double f;
    Eigen::VectorXd g(x.size());
    Eigen::MatrixXd H(x.size(), x.size());
    try {
      grad_trace_matrix_times_hessian(functional, x, M, f, g, H, grad_trace_m_times_h);
    } catch (std::runtime_error& e) {
      throw e;
    }
  }
  
  template <typename F>
  void test_grad_trace_matrix_times_hessian(const F& functional,
                                            const Eigen::VectorXd& x,
                                            const Eigen::MatrixXd& M) {
    
    size_t d = x.size();
    Eigen::MatrixXd grad_hessian_auto(d, d * d);
    try {
      grad_hessian(functional, x, grad_hessian_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Gradient of Trace Matrix Times Hessian Test" << std::endl;
      throw e;
    }
    
    Eigen::VectorXd grad_trace_m_times_h = Eigen::VectorXd::Zero(d);
    
    for (size_t i = 0; i < d; ++i) {
      grad_trace_m_times_h(i) = 0;
      for (size_t j = 0; j < d; ++j)
        grad_trace_m_times_h(i) += grad_hessian_auto.block(0, i * d, d, d).col(j).dot(M.row(j));
    }
    
    Eigen::VectorXd grad_trace_m_times_h_auto(x.size());
    try {
      grad_trace_matrix_times_hessian(functional, x, M, grad_trace_m_times_h_auto);
    } catch (std::runtime_error& e) {
      std::cout << "Cannot compute Gradient of Trace Matrix Times Hessian Test" << std::endl;
      throw e;
    }
    
    std::cout.precision(6);
    int width = 12;
    int n_column = 3;
    
    std::cout << "Gradient of Trace Matrix Times Hessian Test:" << std::endl;
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
    
    for (size_t i = 0; i < d; ++i) {
      std::cout << "    "
                << std::setw(width) << std::left << i
                << std::setw(width) << std::left << grad_trace_m_times_h_auto(i)
                << std::setw(width) << std::left << grad_trace_m_times_h(i)
                << std::endl;
    }
    
    std::cout << "    " << std::setw(n_column * width) << std::setfill('-')
              << "" << std::setfill(' ') << std::endl;
    std::cout << std::endl;
    
  }
  
}

#endif