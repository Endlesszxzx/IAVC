//
// Created by liu-ye-ye on 2023/11/6.
//

#include "symex_target_equation.h"
#include "goto-programs/abstract_goto_model.h"
#include "memory_model_sc_expand.h"
#ifndef CBMC_INTERRUPT_ATOMIC_VIOLATION_CHECK_H
#define CBMC_INTERRUPT_ATOMIC_VIOLATION_CHECK_H

class interrupt_atomic_violation_check
{
private:
  using thread_idt = unsigned;
  using event_it=symex_target_equationt::SSA_stepst::const_iterator;
  using address_t = irep_idt;
  using EventType = goto_trace_stept::typet;
  symex_target_equationt &equation;
  message_handlert& mes;
  std::map<unsigned, std::vector<event_it>>& per_thread_map;
  abstract_goto_modelt & goto_model;
  memory_model_baset& handle;
  std::set<std::pair<event_it,thread_idt>> spawns_before_set;
  unsigned assertion_num = 0;
  static address_t address(event_it event) {
    return remove_level_2(event->ssa_lhs).get_identifier();
  }
  void add_assertion(event_it before,
                const std::string& id,
                const exprt& guard,
                const exprt& con,
                const std::string& msg);
  std::string get_id(event_it e1,
                     event_it e2,
                     event_it e3);
  std::string print_event(const event_it& e);
  goto_programt& get_function_program(dstringt thread_id){
      auto  fun = goto_model.get_goto_functions().function_map.find(thread_id);
      assert(fun!=goto_model.get_goto_functions().function_map.end());
      return const_cast<goto_programt&>(fun->second.body);
  }

  void add_av_check();
  std::vector<event_it>& get_thread_events(thread_idt id);
  std::map<event_it,thread_idt> spawns_mapping;
  void add_assertions(  event_it e1,
                      std::vector<event_it> &e2s,
                      EventType e2_t,
                      event_it e3,
                      std::vector<exprt>& last_guards);
  struct cachet{
      event_it  e1,e3;
      thread_idt inserter;
      cachet(const event_it &e1, thread_idt inserter ,const event_it &e3): e1(e1), e3(e3), inserter(inserter){

      }
      bool operator<(const cachet &rhs) const
      {
        if(e1 < rhs.e1)
          return true;
        if(rhs.e1 < e1)
          return false;
        if(e3 < rhs.e3)
          return true;
        if(rhs.e3 < e3)
          return false;
        return inserter < rhs.inserter;
      }
      bool operator>(const cachet &rhs) const
      {
        return rhs < *this;
      }
      bool operator<=(const cachet &rhs) const
      {
        return !(rhs < *this);
      }
      bool operator>=(const cachet &rhs) const
      {
        return !(*this < rhs);
      }
  };
  std::map<cachet,exprt> caches{};
public:
  interrupt_atomic_violation_check(symex_target_equationt& equation,
                                   message_handlert& mes,
                                   std::map<unsigned, std::vector<event_it>>& per_thread_map,
                                   abstract_goto_modelt & goto_model,
                                   memory_model_baset& handle
                                   ):equation(equation),
                                    mes(mes),
                                    per_thread_map(per_thread_map),
                                    goto_model(goto_model),
                                    handle(handle){


  }
  void add_atomic_violation_assertions();
};

#endif //CBMC_INTERRUPT_ATOMIC_VIOLATION_CHECK_H
