/*******************************************************************\

Module: Memory models for partial order concurrency

Author: Michael Tautschnig, michael.tautschnig@cs.ox.ac.uk

\*******************************************************************/

/// \file
/// Memory models for partial order concurrency

#ifndef CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H
#define CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H

#include "memory_model.h"
#include "memory_model_util.h"
create_memory_model(memory_model_sct,memory_model_baset)
 override_run(memory_model_sct);
protected:
  exprt before(event_it e1, event_it e2) override;
  virtual bool program_order_is_relaxed(
    partial_order_concurrencyt::event_it e1,
    partial_order_concurrencyt::event_it e2) const;

  static void build_per_thread_map(
    const symex_target_equationt &equation,
    per_thread_mapt &dest) ;
  virtual void thread_spawn(
    symex_target_equationt &equation,
    const per_thread_mapt &per_thread_map);
  virtual void program_order(symex_target_equationt &equation);

    virtual void from_read(symex_target_equationt &equation);

    virtual void write_serialization_external(symex_target_equationt &equation);
  // W-W pair
  // built by write_serialization_external()

  // built by from_read()
  typedef std::map<std::pair<event_it,event_it>,exprt> constrains_t;
  constrains_t ws_relations;
  constrains_t fr_relations;
create_memory_model_end
#endif // CPROVER_GOTO_SYMEX_MEMORY_MODEL_SC_H
