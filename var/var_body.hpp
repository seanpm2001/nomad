#ifndef nomad__var__var_body_hpp
#define nomad__var__var_body_hpp

#include <iostream>
#include <string>

#include <autodiff/autodiff_stack.hpp>

namespace nomad {
  
  class var_base {
  protected:
    
    index_t dual_numbers_idx_;
    index_t partials_idx_;
    index_t inputs_idx_;
    
    unsigned int n_inputs_;
    
  public:
    
    var_base(): n_inputs_(0) {

      dual_numbers_idx_ = next_dual_number_idx_;
      partials_idx_ = next_partials_idx_;
      inputs_idx_ = next_inputs_idx_;
      
      next_body_idx_++;
    
    };

    var_base(unsigned int n_inputs):
    n_inputs_(n_inputs) {
      
      dual_numbers_idx_ = next_dual_number_idx_;
      partials_idx_ = next_partials_idx_;
      inputs_idx_ = next_inputs_idx_;
      
      next_body_idx_++;
    }
    
    virtual ~var_base() {};

    inline index_t dual_numbers() { return dual_numbers_idx_; }
    inline unsigned int n_inputs() { return n_inputs_; }
    
    index_t input() { return inputs_[inputs_idx_]; }
    index_t input(unsigned int k) { return inputs_[inputs_idx_ + k]; }
    
    // Are these used?
    index_t begin() { return inputs_[inputs_idx_]; }
    index_t end() { return inputs_[inputs_idx_ + n_inputs_]; }
    
    inline double& first_val()   { return dual_numbers_[dual_numbers_idx_];     }
    inline double& first_grad()  { return dual_numbers_[dual_numbers_idx_ + 1]; }
    inline double& second_val()  { return dual_numbers_[dual_numbers_idx_ + 2]; }
    inline double& second_grad() { return dual_numbers_[dual_numbers_idx_ + 3]; }
    inline double& third_val()   { return dual_numbers_[dual_numbers_idx_ + 4]; }
    inline double& third_grad()  { return dual_numbers_[dual_numbers_idx_ + 5]; }
    inline double& fourth_val()  { return dual_numbers_[dual_numbers_idx_ + 6]; }
    inline double& fourth_grad() { return dual_numbers_[dual_numbers_idx_ + 7]; }
    
    inline static double& first_val(index_t idx)   { return dual_numbers_[idx];     }
    inline static double& first_grad(index_t idx)  { return dual_numbers_[idx + 1]; }
    inline static double& second_val(index_t idx)  { return dual_numbers_[idx + 2]; }
    inline static double& second_grad(index_t idx) { return dual_numbers_[idx + 3]; }
    inline static double& third_val(index_t idx)   { return dual_numbers_[idx + 4]; }
    inline static double& third_grad(index_t idx)  { return dual_numbers_[idx + 5]; }
    inline static double& fourth_val(index_t idx)  { return dual_numbers_[idx + 6]; }
    inline static double& fourth_grad(index_t idx) { return dual_numbers_[idx + 7]; }

    inline double* first_partials()  { return partials_ + partials_idx_; }
    inline double* second_partials() { return partials_ + partials_idx_ + n_first_partials(); }
    inline double* third_partials()  { return partials_ + partials_idx_ + n_second_partials(); }

    inline unsigned int n_first_partials() { return 0; }
    inline unsigned int n_second_partials() { return 0; }
    inline unsigned int n_third_partials() { return 0; }
    
    inline static unsigned int n_partials(unsigned int n_inputs) { return 0; }

    template<short autodiff_order>
    void print(std::ostream* output, std::string prefix = "") {
      if(!output) return;
      
      *output << prefix << "Dual numbers stored at index " << dual_numbers_idx_ << std::endl;
      
      if (autodiff_order >= 1) {
        *output << prefix << "  first val = " << first_val()
                << ", first grad = " << first_grad() << std::endl;
      }
      if (autodiff_order >= 2) {
        *output << prefix << "  second val = " << second_val()
                << ", second grad = " << second_grad() << std::endl;
      }
      if (autodiff_order >= 2) {
        *output << prefix << "  third val = " << third_val()
                << ", third grad = " << third_grad() << std::endl;
        *output << prefix << "  fourth val = " << fourth_val()
                << ", fourth grad = " << fourth_grad() << std::endl;
      }
      *output << std::endl;
      
      if (n_inputs_) {
        *output << prefix << "Inputs stored at index " << inputs_idx_ << std::endl;
        *output << prefix << "Dual number index of inputs: " << std::endl;
        for (int i = 0; i < n_inputs_; ++i)
          *output << prefix << "  (" << i << ") " << input(i) << std::endl;
        *output << std::endl;
      }
      
      if (n_partials(n_inputs_)) {
        *output << prefix << "Partials stored at index = " << partials_idx_ << std::endl;
        
        if (n_first_partials()) *output << prefix << "First partials: " << std::endl;
        for (int i = 0; i < n_first_partials(); ++i)
          *output << prefix << "  (" << i << ") " << first_partials()[i] << std::endl;
        
        if (n_second_partials()) *output << prefix << "Second partials: " << std::endl;
        for (int i = 0; i < n_second_partials(); ++i)
          *output << prefix << "  (" << i << ") " << second_partials()[i] << std::endl;
        
        if (n_third_partials()) *output << prefix << "Third partials: " << std::endl;
        for (int i = 0; i < n_third_partials(); ++i)
          *output << prefix << "  (" << i << ") " << third_partials()[i] << std::endl;
        
        *output << std::endl;
      }
      
    }

    inline virtual void first_order_forward_adj()  {};
    inline virtual void first_order_reverse_adj()  {};
    virtual void second_order_forward_val() {};
    virtual void second_order_reverse_adj() {};
    virtual void third_order_forward_val()  {};
    virtual void third_order_reverse_adj()  {};
    
  };
  
  template<short autodiff_order>
  void expand_var_bodies() {
    
    if (!max_body_idx) {
      max_body_idx = base_body_size_;
      var_bodies_ = new var_base[max_body_idx];
      next_body_idx_ -= max_body_idx;
    } else {
      max_body_idx *= 2;
      
      var_base* new_stack = new var_base[max_body_idx];
      for (int i = 0; i < next_body_idx_; ++i)
        new_stack[i] = var_bodies_[i];
      delete[] var_bodies_;
      
      var_bodies_ = new_stack;
    }
    
    expand_dual_numbers<autodiff_order>();
    
  }
  
  template<short autodiff_order, short partials_order>
  class var_body: public var_base {
  public:
    
    static inline void* operator new(size_t /* ignore */) {
      if (unlikely(next_body_idx_ + 1 > max_body_idx)) expand_var_bodies<autodiff_order>();
      if (unlikely(next_partials_idx_ + next_partials_delta > max_partials_idx)) expand_partials();
      if (unlikely(next_inputs_idx_ + next_inputs_delta > max_inputs_idx)) expand_inputs();
      return var_bodies_ + next_body_idx_;
    }
    
    static inline void operator delete(void* /* ignore */) {};
    
    var_body(unsigned int n_inputs): var_base(n_inputs) {};

    inline unsigned int n_first_partials() {
      return autodiff_order >= 1 && partials_order >= 1 ?
             n_inputs_ : 0;
    }
    
    inline unsigned int n_second_partials() {
      return autodiff_order >= 2 && partials_order >= 2 ?
             n_inputs_ * (n_inputs_ + 1) / 2 : 0;
    }
    
    inline unsigned int n_third_partials() {
      return autodiff_order >= 3 && partials_order >= 3 ?
             n_inputs_ * (n_inputs_ + 1) * (n_inputs_ + 2) / 6 : 0;
    }
    
    inline static unsigned int n_partials(unsigned int n_inputs) {
      if (autodiff_order >= 1 && partials_order >= 1)
        return n_inputs;
      
      // n + n * (n + 1) / 2
      if (autodiff_order >= 2 && partials_order >= 2)
        return n_inputs * (n_inputs + 3) / 2;
      
      // n + n * (n + 1) / 2 + n * (n + 1) * (n + 2) / 6
      if (autodiff_order >= 3 && partials_order >= 3)
        return n_inputs * (11 + 6 * n_inputs + n_inputs * n_inputs) / 6;
    }
    
    inline void first_order_forward_adj() {
      
      if (n_inputs_) first_grad() = 0;
      
      if (partials_order >= 1) {
        
        double* first_partial = first_partials();
        
        double g = 0;
        
        for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial)
          g += first_grad(input(i)) * *first_partial;
        first_grad() += g;
        
      }
      
    }
    
    inline void first_order_reverse_adj() {
      if (partials_order >= 1) {
        double* first_partial = first_partials();
        const double g = first_grad();
        for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial)
          first_grad(input(i)) += g * *first_partial;
      }
      
    }
    
    void second_order_forward_val() {
      
      if (autodiff_order >= 2) {
        
        if (n_inputs_) second_val() = 0;
        second_grad() = 0;
        
        if (partials_order >= 1) {
          
          double* first_partial = first_partials();
          double v2 = 0;
          
          for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial)
            v2 += second_val(input(i)) * *first_partial;
          second_val() += v2;
          
        }
        
      }
      
    }
    
    virtual void second_order_reverse_adj() {
      
      if (autodiff_order >= 2) {
        
        if (partials_order >= 1) {
          
          double* first_partial = first_partials();
          const double g = second_grad();
          
          for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial)
            second_grad(input(i)) += g * *first_partial;
          
        }
        
        if (partials_order >= 2) {
          
          const double g1 = first_grad();
          
          for (unsigned int i = 0; i < n_inputs_; ++i) {
            
            double g2 = 0;
            
            double* second_partial = second_partials() + i * (i + 1) / 2;
            for (unsigned int j = 0; j < n_inputs_; ++j) {
              
              g2 += g1 * second_val(input(j)) * *second_partial;
              
              if (j < i) ++second_partial;
              else       second_partial += j * (j + 1) / 2 + 1;
              
            }
            
            second_grad(input(i)) += g2;
            
          }
          
        }
        
      }
      
    }
    
    virtual void third_order_forward_val() {
      
      if (autodiff_order >= 3) {
        
        if (n_inputs_) {
          third_val() = 0;
          fourth_val() = 0;
        }
        
        third_grad() = 0;
        fourth_grad() = 0;
        
        if (partials_order >= 1) {
          
          double* first_partial = first_partials();
          double v3 = 0;
          double v4 = 0;
          
          for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial) {
            v3 += third_val(input(i)) * *first_partial;
            v4 += fourth_val(input(i)) * *first_partial;
          }
          third_val() += v3;
          fourth_val() += v4;
          
        }
        
        if (partials_order >= 2) {
          
          double v4 = 0;
          
          for (unsigned int i = 0; i < n_inputs_; ++i) {
            
            const double s2 = second_val(input(i));
            
            // Benchmark this in the future?
            if(!s2) continue;
            
            double* second_partial = second_partials() + i * (i + 1) / 2;
            for (unsigned int j = 0; j < n_inputs_; ++j) {
              
              v4 += s2 * third_val(input(j)) * *second_partial;
              
              if (j < i) ++second_partial;
              else       second_partial += j * (j + 1) / 2 + 1;
              
            }
          }

          fourth_val() += v4;
          
        }
        
      }
      
    } // third_order_forward_val
    
    virtual void third_order_reverse_adj() {
      
      if (autodiff_order >= 3) {

        const double g1 = first_grad();
        const double g2 = second_grad();
        const double g3 = third_grad();
        const double g4 = fourth_grad();
        
        if (partials_order >= 1) {
          
          double* first_partial = first_partials();
          
          for (unsigned int i = 0; i < n_inputs_; ++i, ++first_partial) {
            third_grad(input(i))  += g3 * *first_partial;
            fourth_grad(input(i)) += g4 * *first_partial;
          }
        }
        
        if (partials_order >= 2) {
          
          for (unsigned int i = 0; i < n_inputs_; ++i) {
            
            double in_g3 = 0;
            double in_g4 = 0;
            
            double* second_partial = second_partials() + i * (i + 1) / 2;
            for (unsigned int j = 0; j < n_inputs_; ++j) {
              
              if(!*second_partial) {
                if (j < i) ++second_partial;
                else       second_partial += j * (j + 1) / 2 + 1;
                continue;
              }
              
              in_g3 += g1 * third_val(input(j)) * *second_partial;
              
              double alpha =   g1 * fourth_val(input(j))
                             + g2 * third_val(input(j))
                             + g3 * second_val(input(j));
              
              in_g4 += alpha * *second_partial;
              
              if (j < i) ++second_partial;
              else       second_partial += j * (j + 1) / 2 + 1;

            }

            third_grad(input(i)) += in_g3;
            fourth_grad(input(i)) += in_g4;
            
          }
          
        }
        
        if(partials_order >= 3) {
          
          for (int i = 0; i < n_inputs_; ++i) {
            
            double in_g4 = 0;
            
            for (int j = 0; j < n_inputs_; ++j) {
              
              const double in_v2 = second_val(input(j));
              
              double* third_partial = third_partials()
                                      + i * (i + 1) * (i + 2) / 6
                                      + j * (j + 1) / 2;
              
              for (unsigned int k = 0; k < n_inputs_; ++k) {
                
                in_g4 +=   g1
                         * in_v2
                         * third_val(input(k))
                         * *third_partial;
                
                if (k < j)  ++third_partial;
                else        third_partial += k * (k + 1) / 2 + 1;
                if (j >= i) third_partial += j * (j + 1) * (j + 2) / 6 + 1;
                
                //std::cout << "\t" << g1 << "\t" << in_v2 << "\t" << third_val(input(k))
                //<< "\t" << *third_partial << std::endl;
                
              }
            }
            
            //std::cout << "third order contribution: " << in_g4 << std::endl;
            
            fourth_grad(input(i)) += in_g4;
            
          }
          
        }
        
      }
      
    } // third_order_reverse_adj
    
  };
  
  template<short autodiff_order>
  class var_body<autodiff_order, 0>: public var_base {
    
  public:
    
    static inline void* operator new(size_t /* ignore */) {
      if (unlikely(next_body_idx_ + 1 > max_body_idx)) expand_var_bodies<autodiff_order>();
      // no partials
      // no inputs
      return var_bodies_ + next_body_idx_;
    }
    
    static inline void operator delete(void* /* ignore */) {};
    
    var_body(): var_base() {};
    
    var_body(unsigned int n_inputs): var_base(n_inputs) {};
    
    void first_order_forward_adj() {
      if (autodiff_order >= 1) {
        if (n_inputs_) first_grad() = 0;
      }
    }
    
    void second_order_forward_val() {
      if (n_inputs_) second_val() = 0;
      second_grad() = 0;
    }
    
    virtual void third_order_forward_val() {
      if (n_inputs_) {
        third_val() = 0;
        fourth_val() = 0;
      }
      third_grad() = 0;
      fourth_grad() = 0;
    }
    
  };
  
}

#endif
