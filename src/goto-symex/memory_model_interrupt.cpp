/*******************************************************************\

Module: Memory model for partial order concurrency

Author: Lihao Liang, lihao.liang@cs.ox.ac.uk

\*******************************************************************/

#include <util/std_expr.h>
#include <algorithm>
#include<set>
#include "memory_model_interrupt.h"
#include "memory_model_util.h"
/*******************************************************************\

Function: memory_model_interruptt::operator()

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

/*******************************************************************\

Function: memory_model_interruptt::read_from

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
void memory_model_interruptt::read_from(symex_target_equationt &equation)
{
  memory_model_sc_expand:: read_from(equation);
  FOR_EVERY_RF_RELATION(rf_relation){
    GET_MES_FROM_RELATION(r, w, s, rf_relation);
    assert(!po(r, w));
    //注意一定要注意判断是否在相同的线程  否则会出现扯蛋的情况 自己想一想
    if(!IS_IN_SAME_THREAD(w,r)&&DEPTH(w) >= DEPTH(r)){
      // must use before(w, r) instead of c_it.second
      exprt cond=implies_exprt(and_exprt(before(w, r), w->guard, r->guard),last(w, r));
      add_constraint(equation, cond, "rf-irq", r->source);
    }
  }
}

/*******************************************************************\

Function: memory_model_interruptt::write_serialization_external

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void memory_model_interruptt::write_serialization_external(symex_target_equationt &equation){
  memory_model_sc_expand::write_serialization_external(equation);
  FOR_EVERY_WS_RELATION(ws_relation){
        GET_MES_FROM_RELATION(w1, w2, s, ws_relation);
        if(DEPTH(w1) >= DEPTH(w2)){
          add_constraint(
            equation,
            implies_exprt(
              and_exprt(s, w1->guard, w2->guard),
              last(w1, w2)),
            "ws-irq",
            w1->source);
        }
        if(DEPTH(w1) <= DEPTH(w2)){
          add_constraint(
            equation,
            implies_exprt(
              and_exprt(not_exprt(s), w1->guard, w2->guard),
              last(w2, w1)),
            "ws-irq",
            w1->source);
        }
  }
}

/*******************************************************************\

Function: memory_model_interruptt::from_read

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void memory_model_interruptt::from_read(symex_target_equationt &equation)
{
  // from-read: (w', w) in ws and (w', r) in rf -> (r, w) in fr
  memory_model_sc_expand::from_read(equation);
  FOR_EVERY_FR_RELATION(fr_re){
      GET_MES_FROM_RELATION(r,w,fr,fr_re);
      //注意一定要注意判断是否在相同的线程  否则会出现扯蛋的情况 自己想一想
      if(!IS_IN_SAME_THREAD(r,w)&&DEPTH(r)>=DEPTH(w)){
          add_constraint(equation,implies_exprt(fr,last(r,w)),"fr-irq",r->source);
      }
  }
}

/*******************************************************************\

Function: memory_model_interruptt::last

  Inputs:

 Outputs:

 Purpose: compute a before constraint from the last event of the thread
          which the from event is in to the to event

\*******************************************************************/

exprt memory_model_interruptt::last(const event_it &from, const event_it &to)
{
  const auto &events= get_thread_events(get_thread_id(from));
  exprt::operandst pty_operands;
  pty_operands.reserve(1);
  auto e_it=--events.end();
  assert(std::find(events.begin(), events.end(), from)!=events.end());
  while(from!=*e_it){
    if(IS_MEMORY_MODEL_RELATED(*e_it)){
      pty_operands.push_back(before(*e_it, to));
      break;
    }
    --e_it;
  }
  return conjunction(pty_operands);
}

