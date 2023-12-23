/*******************************************************************\

Module: Memory model for partial order concurrency

Author: Michael Tautschnig, michael.tautschnig@cs.ox.ac.uk

\*******************************************************************/

/// \file
/// Memory model for partial order concurrency

#include "memory_model_sc.h"

#include <util/std_expr.h>

run_start(equation,message_handler,memory_model_sct)
  program_order(equation);
  read_from(equation);
  write_serialization_external(equation);
  from_read(equation);
run_end
exprt memory_model_sct::before(event_it e1, event_it e2)
{
  return partial_order_concurrencyt::before(e1, e2, AX_PROPAGATION);
}

bool memory_model_sct::program_order_is_relaxed(
  partial_order_concurrencyt::event_it e1,
  partial_order_concurrencyt::event_it e2) const
{
  PRECONDITION(e1->is_shared_read() || e1->is_shared_write());
  PRECONDITION(e2->is_shared_read() || e2->is_shared_write());

  return false;
}

void memory_model_sct::build_per_thread_map(
  const symex_target_equationt &equation,
  per_thread_mapt &dest)
{
  // this orders the events within a thread

    FOR_EVERY_EVENT(e_it, equation){
    // concurrency-related?
        if(!IS_MEMORY_MODEL_RELATED(e_it)) continue;
        dest[EVENT_THREAD_ID(e_it)].push_back(e_it);
    }
}

void memory_model_sct::thread_spawn(
  symex_target_equationt &equation,
  const per_thread_mapt &per_thread_map)
{
  // thread spawn: the spawn precedes the first
  // instruction of the new thread in program order

  unsigned next_thread_id=0;
  FOR_EVERY_EVENT(e_it, equation){
    if(e_it->is_spawn()){
      auto next_thread=per_thread_map.find(++next_thread_id);
      if(next_thread==per_thread_map.end())
        continue;
      // add a constraint for all events,
      // considering regression/cbmc-concurrency/pthread_create_tso1
      for(auto n_it : next_thread->second){
        if(!n_it->is_memory_barrier())
          add_constraint(equation,before(e_it, n_it),"thread-spawn",e_it->source);
      }

    }
  }
}

#if 0
void memory_model_sct::thread_spawn(
  symex_target_equationt &equation,
  const per_thread_mapt &per_thread_map)
{
  // thread spawn: the spawn precedes the first
  // instruction of the new thread in program order

  unsigned next_thread_id=0;
  for(eventst::const_iterator
      e_it=equation.SSA_steps.begin();
      e_it!=equation.SSA_steps.end();
      e_it++)
  {
    if(is_spawn(e_it))
    {
      per_thread_mapt::const_iterator next_thread=
        per_thread_map.find(++next_thread_id);
      if(next_thread==per_thread_map.end())
        continue;

      // For SC and several weaker memory models a memory barrier
      // at the beginning of a thread can simply be ignored, because
      // we enforce program order in the thread-spawn constraint
      // anyway. Memory models with cumulative memory barriers
      // require explicit handling of these.
      event_listt::const_iterator n_it=next_thread->second.begin();
      for( ;
          n_it!=next_thread->second.end() &&
          (*n_it)->is_memory_barrier();
          ++n_it)
      {
      }

      if(n_it!=next_thread->second.end())
        add_constraint(
          equation,
          before(e_it, *n_it),
          "thread-spawn",
          e_it->source);
    }
  }
}
#endif

void memory_model_sct::program_order(
  symex_target_equationt &equation)
{
  CREATE_THREAD_MAP(per_thread_map);
  thread_spawn(equation, per_thread_map);
  // iterate over threads
  FOR_EVERY_THREAD(thread, per_thread_map){
    // iterate over relevant events in the thread
    bool first = true;
    event_it previous;
    for(auto event : GET_THREAD_EVENTS(thread))
    {
      if(event->is_memory_barrier())
         continue;
      if(first){
        // first one?
        previous=event;
        first= false;
        continue;
      }
      add_constraint(equation,before(previous, event),"po",event->source);
      previous=event;
    }
  }
}

void memory_model_sct::write_serialization_external(symex_target_equationt &equation){
    FOR_EVERY_VARIABLE(variable){
    // This is quadratic in the number of writes
    // per address. Perhaps some better encoding
    // based on 'places'?
        FOR_EVERY_WRITE_PAIR(w_it1, w_it2, GET_VARIABLE_WRITES(variable)){
            // external?
            if(IS_IN_SAME_THREAD(*w_it1, *w_it2))continue;
            // ws is a total order, no two elements have the same rank
            // s -> w_evt1 before w_evt2; !s -> w_evt2 before w_evt1
            symbol_exprt s=nondet_bool_symbol("ws-ext");
            ws_relations.insert({{*w_it1, *w_it2}, s});
            // write-to-write edge
            add_constraint(equation,implies_exprt(s, before(*w_it1, *w_it2)),"ws-ext",(*w_it1)->source);
            add_constraint(equation,implies_exprt(not_exprt(s), before(*w_it2, *w_it1)),"ws-ext",(*w_it1)->source);
            //show(*w_it1,*w_it2);
        }
    }
}

void memory_model_sct::from_read(symex_target_equationt &equation)
{
  // from-read: (w', w) in ws and (w', r) in rf -> (r, w) in fr

    FOR_EVERY_VARIABLE(variable){
        FOR_EVERY_WRITE_PAIR(w1, w2, GET_VARIABLE_WRITES(variable)){
            exprt ws1, ws2;
            if(po(*w1, *w2) &&!program_order_is_relaxed(*w1, *w2))
            {
              ws1=true_exprt();
              ws2=false_exprt();
            }
            else if(po(*w2, *w1)&&!program_order_is_relaxed(*w2, *w1)){
              ws1=false_exprt();
              ws2=true_exprt();
            }else{
              ws1=before(*w1, *w2);
              ws2=before(*w2, *w1);
            }
            // smells like cubic
            FOR_EVERY_RF_RELATION(rf_relation){
                GET_MES_FROM_RELATION(r_e, w_e, rf_symbol, rf_relation);
                exprt cond;
                cond.make_nil();
                if(w_e==*w1 && !ws1.is_false()){
                    exprt fr=before(r_e, *w2);
                    // the guard of w_prime follows from rf; with rfi
                    // optimisation such as the previous write_symbol_primed
                    // it would even be wrong to add this guard
                    const auto and_exp = and_exprt(r_e->guard, (*w2)->guard, ws1, rf_symbol);
                    cond=implies_exprt(and_exp,fr);
                    RECORD_FR_RELATION(r_e, *w2, fr);
                }else if(w_e==*w2 && !ws2.is_false()){
                    exprt fr=before(r_e, *w1);
                    // the guard of w follows from rf; with rfi
                    // optimisation such as the previous write_symbol_primed
                    // it would even be wrong to add this guard
                    const auto and_exp = and_exprt(r_e->guard, (*w1)->guard, ws2, rf_symbol);
                    cond=implies_exprt(and_exp,fr);
                    RECORD_FR_RELATION(r_e, *w1, fr);
                }
                  if(cond.is_not_nil()) add_constraint(equation,cond, "fr", r_e->source);
            }
        }
    }
}
