/*******************************************************************\

Module: Memory models for partial order concurrency

Author: Lihao Liang, lihao.liang@cs.ox.ac.uk

\*******************************************************************/

#ifndef CPROVER_GOTO_SYMEX_MEMORY_MODEL_INTERRUPT_H
#define CPROVER_GOTO_SYMEX_MEMORY_MODEL_INTERRUPT_H

#include "memory_model_sc_expand.h"

create_memory_model(memory_model_interruptt, memory_model_sc_expand)
protected:
  void read_from(symex_target_equationt &equation) override;
  void from_read(symex_target_equationt &equation) override;
  void write_serialization_external(symex_target_equationt &equation) override;
  exprt last(const event_it &from, const event_it &to);
create_memory_model_end

#endif