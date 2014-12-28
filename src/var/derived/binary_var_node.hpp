#ifndef nomad__src__var__derived__binary_var_node_hpp
#define nomad__src__var__derived__binary_var_node_hpp

#include <src/var/var_node.hpp>

namespace nomad {
  
  template<short AutodiffOrder, short PartialsOrder>
  class binary_var_node: public var_node_base {
  public:
    
    static inline void* operator new(size_t /* ignore */) {
      if (unlikely(nmd_stk::next_node_idx + 1 > nmd_stk::max_node_idx)) expand_var_nodes<AutodiffOrder>();
      if (unlikely(nmd_stk::next_partials_idx + nmd_stk::next_partials_delta > nmd_stk::max_partials_idx))
        expand_partials();
      if (unlikely(nmd_stk::next_inputs_idx + nmd_stk::next_inputs_delta > nmd_stk::max_inputs_idx))
        expand_inputs();
      return nmd_stk::var_nodes + nmd_stk::next_node_idx;
    }
    
    static inline void operator delete(void* /* ignore */) {}
    
    binary_var_node(): var_node_base(2) {}

    constexpr static bool dynamic_inputs() { return false; }
    
    inline nomad_idx_t n_first_partials() {
      return AutodiffOrder >= 1 && PartialsOrder >= 1 ? 2 : 0;
    }
    
    inline nomad_idx_t n_second_partials() {
      return AutodiffOrder >= 2 && PartialsOrder >= 2 ? 3 : 0;
    }
    
    inline nomad_idx_t n_third_partials() {
      return AutodiffOrder >= 3 && PartialsOrder >= 3 ? 4 : 0;
    }

    inline static nomad_idx_t n_partials() {
      if (AutodiffOrder >= 1 && PartialsOrder >= 1) return 2;
      if (AutodiffOrder >= 2 && PartialsOrder >= 2) return 5;
      if (AutodiffOrder >= 3 && PartialsOrder >= 3) return 9;
      return 0;
    }
    
    inline static nomad_idx_t n_partials(nomad_idx_t n_inputs) {
      if (AutodiffOrder >= 1 && PartialsOrder >= 1) return 2;
      if (AutodiffOrder >= 2 && PartialsOrder >= 2) return 5;
      if (AutodiffOrder >= 3 && PartialsOrder >= 3) return 9;
      return 0;
    }
    
    inline void first_order_forward_adj() {
      first_grad() = 0;
      if (PartialsOrder >= 1)
        first_grad() +=   first_grad(input())  * first_partials(0)
                        + first_grad(input(1)) * first_partials(1);
    }
    
    inline void first_order_reverse_adj() {
      if (PartialsOrder >= 1) {
        const double g = first_grad();
        first_grad(input()) += g * first_partials(0);
        first_grad(input(1)) += g * first_partials(1);
      }
    }
    
    void second_order_forward_val() {
      
      if (AutodiffOrder >= 2) {
        second_val() = 0;
        second_grad() = 0;
      
        if (PartialsOrder >= 1)
          second_val() +=   second_val(input())  * first_partials(0)
                          + second_val(input(1)) * first_partials(1);
      }
      
    }
    
    void second_order_reverse_adj() {
      
      if (AutodiffOrder >= 2) {
        
        if (PartialsOrder >= 1) {
          second_grad(input())  += second_grad() * first_partials(0);
          second_grad(input(1)) += second_grad() * first_partials(1);
        }
        
        if (PartialsOrder >= 2) {
          second_grad(input())  += first_grad() * (  second_val(input())  * first_partials(2)
                                                   + second_val(input(1)) * first_partials(3));
          second_grad(input(1)) += first_grad() * (  second_val(input())  * first_partials(3)
                                                   + second_val(input(1)) * first_partials(4));
        }
        
      }
      
    }
    
    void third_order_forward_val() {
      
      if (AutodiffOrder >= 3) {
        third_val() = 0;
        fourth_val() = 0;
        third_grad() = 0;
        fourth_grad() = 0;
        
        if (PartialsOrder >= 1) {
          third_val()  +=   third_val(input())  * first_partials(0)
                          + third_val(input(1)) * first_partials(1);
          
          fourth_val() +=   fourth_val(input())  * first_partials(0)
                          + fourth_val(input(1)) * first_partials(1);
        }
        
        if (PartialsOrder >= 2) {
          fourth_val() +=   second_val(input()) * third_val(input())   * first_partials(2)
                          + second_val(input()) * third_val(input(1))  * first_partials(3)
                          + second_val(input(1)) * third_val(input())  * first_partials(3)
                          + second_val(input(1)) * third_val(input(1)) * first_partials(4);
        }
        
      }
      
    } // third_order_forward_val
    
    void third_order_reverse_adj() {
      
      if (AutodiffOrder >= 3) {

        const double g1 = first_grad();
        const double g2 = second_grad();
        const double g3 = third_grad();
        const double g4 = fourth_grad();
        
        if (PartialsOrder >= 1) {
          third_grad(input())   += g3 * first_partials(0);
          third_grad(input(1))  += g3 * first_partials(1);
          
          fourth_grad(input())  += g4 * first_partials(0);
          fourth_grad(input(1)) += g4 * first_partials(1);
        }
        
        if (PartialsOrder >= 2) {
          third_grad(input())  += g1 * (  third_val(input())  * first_partials(2)
                                        + third_val(input(1)) * first_partials(3));
          
          third_grad(input(1)) += g1 * (  third_val(input(1)) * first_partials(3)
                                        + third_val(input())  * first_partials(4));
          
          fourth_grad(input())  +=  (  g1 * fourth_val(input())
                                     + g2 * third_val(input())
                                     + g3 * second_val(input())) * first_partials(2)
                                  + (  g1 * fourth_val(input(1))
                                     + g2 * third_val(input(1))
                                     + g3 * second_val(input(1))) * first_partials(3);
          
          fourth_grad(input(1)) +=  (  g1 * fourth_val(input())
                                     + g2 * third_val(input())
                                     + g3 * second_val(input())) * first_partials(3)
                                  + (  g1 * fourth_val(input(1))
                                     + g2 * third_val(input(1))
                                     + g3 * second_val(input(1))) * first_partials(4);
        }
        
        if(PartialsOrder >= 3) {
          fourth_grad(input())  +=   g1 * second_val(input(0)) * third_val(input(0)) * first_partials(5)
                                   + g1 * second_val(input(0)) * third_val(input(1)) * first_partials(6)
                                   + g1 * second_val(input(1)) * third_val(input(0)) * first_partials(6)
                                   + g1 * second_val(input(1)) * third_val(input(1)) * first_partials(7);
          fourth_grad(input(1)) +=   g1 * second_val(input(0)) * third_val(input(0)) * first_partials(6)
                                   + g1 * second_val(input(0)) * third_val(input(1)) * first_partials(7)
                                   + g1 * second_val(input(1)) * third_val(input(0)) * first_partials(7)
                                   + g1 * second_val(input(1)) * third_val(input(1)) * first_partials(8);
        }
      
      }
      
    } // third_order_reverse_adj
    
  };
  
}

#endif
