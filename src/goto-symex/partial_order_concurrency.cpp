/*******************************************************************\

Module: Add constraints to equation encoding partial orders on events

Author: Michael Tautschnig, michael.tautschnig@cs.ox.ac.uk

\*******************************************************************/

/// \file
/// Add constraints to equation encoding partial orders on events

#include "partial_order_concurrency.h"
#include "memory_model_util.h"
#include <util/arith_tools.h>
#include <util/bitvector_types.h>
#include <util/simplify_expr.h>
#include <iostream>

partial_order_concurrencyt::partial_order_concurrencyt(
  const namespacet &_ns):ns(_ns)
{
}

partial_order_concurrencyt::~partial_order_concurrencyt()= default;

void partial_order_concurrencyt::add_init_writes(
  symex_target_equationt &equation)
{
  std::unordered_set<address_t> init_done;
  bool spawn_seen=false;

  symex_target_equationt::SSA_stepst init_steps;
  FOR_EVERY_EVENT(e_it, equation){
    if(e_it->is_spawn()){
      spawn_seen=true;
      continue;
    }else if(!e_it->is_shared_read() &&!e_it->is_shared_write()) continue;
    const address_t &a=address(e_it);
    if(init_done.find(a)!=init_done.end())
      continue;
    if(spawn_seen ||e_it->is_shared_read() ||!e_it->guard.is_true()){
      init_steps.emplace_back(e_it->source, goto_trace_stept::typet::SHARED_WRITE);
      SSA_stept &SSA_step = init_steps.back();
      SSA_step.guard=true_exprt();
      // no SSA L2 index, thus nondet value
      SSA_step.ssa_lhs = remove_level_2(e_it->ssa_lhs);
      SSA_step.atomic_section_id=0;
    }
    init_done.insert(a);
  }
  equation.SSA_steps.splice(equation.SSA_steps.begin(), init_steps);
}

void partial_order_concurrencyt::build_event_lists(symex_target_equationt &equation,message_handlert &message_handler)
{
  add_init_writes(equation);
  // a per-thread counter
  std::map<clock_t, clock_t> counter;
  FOR_EVERY_EVENT(e_it, equation){
    if(e_it->is_shared_read() ||e_it->is_shared_write() ||e_it->is_spawn()){
      thread_idt thread_id= EVENT_THREAD_ID(e_it);
      if(!e_it->is_spawn()){
        auto v = GET_VARIABLE(address(e_it));
        if(!VARIABLE_IS_VALID(v)){
           CREATE_VARIABLE(address(e_it));
           v = GET_VARIABLE(address(e_it));
        }
        if(e_it->is_shared_read())
            GET_VARIABLE_READS(v).emplace_back(e_it);
        else // must be write
            GET_VARIABLE_WRITES(v).emplace_back(e_it);
      }
      // maps an event id to a per-thread counter
      clock_t clock=counter[thread_id]++;
      numbering[e_it]=clock;
    }
  }
}

irep_idt partial_order_concurrencyt::rw_clock_id(event_it event,axiomt axiom){
  if(event->is_shared_write())
    return id2string(id(event))+"$wclk$"+std::to_string(axiom);
  else if(event->is_shared_read())
    return id2string(id(event))+"$rclk$"+std::to_string(axiom);
  else
    UNREACHABLE;
}

symbol_exprt partial_order_concurrencyt::clock(
  event_it event,
  axiomt axiom)
{
  PRECONDITION(!numbering.empty());
  irep_idt identifier;

  if(event->is_shared_write())
    identifier=rw_clock_id(event, axiom);
  else if(event->is_shared_read())
    identifier=rw_clock_id(event, axiom);
  else if(event->is_spawn()){
    identifier="t"+std::to_string(event->source.thread_nr+1)+"$"+std::to_string(numbering[event])+"$spwnclk$"+std::to_string(axiom);
  }
  else
    UNREACHABLE;
  return symbol_exprt(identifier, clock_type);
}

void partial_order_concurrencyt::build_clock_type(){
  PRECONDITION(!numbering.empty());
  std::size_t width = address_bits(numbering.size());
  clock_type = unsignedbv_typet(width);
}

exprt partial_order_concurrencyt::before(
  event_it e1, event_it e2, unsigned axioms)
{
  const axiomt axiom_bits[]=
  {
    AX_SC_PER_LOCATION,
    AX_NO_THINAIR,
    AX_OBSERVATION,
    AX_PROPAGATION
  };

  exprt::operandst ops;
  ops.reserve(sizeof(axiom_bits)/sizeof(axiomt));

  for(auto ax : axiom_bits)
  {
    if((axioms &ax)==0)
      continue;

    if(e1->atomic_section_id!=0 &&
       e1->atomic_section_id==e2->atomic_section_id)
      ops.push_back(equal_exprt(clock(e1, ax), clock(e2, ax)));
    else
      ops.push_back(
        binary_relation_exprt(clock(e1, ax), ID_lt, clock(e2, ax)));
  }

  POSTCONDITION(!ops.empty());

  return conjunction(ops);
}

void partial_order_concurrencyt::add_constraint(
  symex_target_equationt &equation,
  const exprt &cond,
  const std::string &msg,
  const symex_targett::sourcet &source) const
{
  exprt tmp=cond;
  simplify(tmp, ns);
  equation.constraint(tmp, msg, source);
}

std::string partial_order_concurrencyt::show_event(partial_order_concurrencyt::event_it it) {
    std::ostringstream print;
    if(it->is_shared_read()){
        print<<it->source.function_id<<"  read:"<<id(it);
    } else if(it->is_shared_write()){
        print<<it->source.function_id<<"  write:"<<id(it);
    }else if(it->is_spawn()){
        print<<it->source.function_id<<"  spawn"<<it->source.function_id;
    }else if(it->is_memory_barrier()){
        print<<"memory barrier"<<std::endl;
    }
    return print.str();
}
