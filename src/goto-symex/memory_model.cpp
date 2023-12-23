/*******************************************************************\

Module: Memory model for partial order concurrency

Author: Michael Tautschnig, michael.tautschnig@cs.ox.ac.uk

\*******************************************************************/

/// \file
/// Memory model for partial order concurrency

#include "memory_model.h"
#include "memory_model_util.h"
#include <util/std_expr.h>

memory_model_baset::memory_model_baset(const namespacet &_ns,abstract_goto_modelt& model)
  : partial_order_concurrencyt(_ns),goto_model(model),var_cnt(0)
{
}
memory_model_baset::~memory_model_baset()= default;
symbol_exprt memory_model_baset::nondet_bool_symbol(const std::string &prefix)
{
  return {"memory_model::choice_" + prefix + std::to_string(var_cnt++), bool_typet()};
}
bool memory_model_baset::po(event_it e1, event_it e2){
  // within same thread
  if(IS_IN_SAME_THREAD(e1, e2))
    return numbering[e1] < numbering[e2];
  else{
    // in general un-ordered, with exception of thread-spawning
    return false;
  }
}

void memory_model_baset::read_from(symex_target_equationt &equation)
{
  // We iterate over all the reads, and
  // make them match at least one
  // (internal or external) write.
  FOR_EVERY_VARIABLE(variable){
     auto& reads =GET_VARIABLE_READS(variable);
     auto& writes = GET_VARIABLE_WRITES(variable);
    for(const auto &read_event : reads){
      exprt::operandst rf_choice_symbols;
      rf_choice_symbols.reserve(writes.size());
      // this is quadratic in #events per address
      for(const auto &write_event : writes){
        // rf cannot contradict program order
        if(!po(read_event, write_event)){
          rf_choice_symbols.push_back(register_read_from_choice_symbol(read_event, write_event, equation));
        }
      }
      // uninitialised global symbol like symex_dynamic::dynamic_object*
      // or *$object
      if(!rf_choice_symbols.empty()){
        // Add the read's guard, each of the writes' guards is implied
        // by each entry in rf_some
        add_constraint(equation,implies_exprt{read_event->guard, disjunction(rf_choice_symbols)},"rf-some",read_event->source);
      }
    }
  }
}

symbol_exprt memory_model_baset::register_read_from_choice_symbol(const event_it &r,const event_it &w,symex_target_equationt &equation){
  symbol_exprt s = nondet_bool_symbol("rf");
  // record the symbol
  RECORD_RF_RELATION(r, w, s);
  bool is_rfi = IS_IN_SAME_THREAD(r, w);
  // Uses only the write's guard as precondition, read's guard
  // follows from rf_some
    // We rely on the fact that there is at least
    // one write event that has guard 'true'.
  add_constraint(equation,
                 implies_exprt{s, and_exprt{w->guard, equal_exprt{r->ssa_lhs, w->ssa_lhs}}},
                    is_rfi ? "rfi" : "rf",
                    r->source);
  if(!is_rfi){
    // if r reads from w, then w must have happened before r
    add_constraint(equation, implies_exprt{s, before(w, r)}, "rf-order", r->source);
  }
  return s;
}
