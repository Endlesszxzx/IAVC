//
// Created by liu-ye-ye on 2023/11/5.
//
#include "interrupt_atomic_violation_check.h"

#include "memory_model_util.h"

#include "analyses/guard_expr.h"
#include "goto-programs/remove_skip.h"
std::string interrupt_atomic_violation_check::get_id(
  event_it e1,
  event_it e2,
  event_it e3
  )
{
  std::string  res="av-check in irq:";
  res+=e1->source.function_id.c_str();
  res+="[";
  res+= print_event(e1);
  res+=",";
  res+= print_event(e2);
  res+=",";
  res+= print_event(e3);
  res+="]";
  return res;
}
void interrupt_atomic_violation_check::add_assertion(
  const event_it before,
  const std::string& id,
  const exprt& guard,
  const exprt& con,
  const std::string& msg){
  auto& fun_p = get_function_program(before->source.function_id);
  auto assertion = goto_programt::make_assertion(
    guard,//implies_exprt(guard,con)
    before->source.pc->source_location()
  );
  auto target = fun_p.insert_after(before->source.pc,assertion);//插入一个真实的assert事件
  target->source_location_nonconst().set_property_id(id);
  auto source = before->source;
  source.pc = target;
  guard_expr_managert unused;
  guard_exprt ge(guard,unused);
  equation.assertion(
    guard,//guard guard=>con
    ge.guard_expr(con),//con
    msg,
    source
  );
  assertion_num++;
}
void interrupt_atomic_violation_check::add_atomic_violation_assertions(){
  for(auto &t:per_thread_map){
    std::cout<<t.first<<std::endl;
    for(auto &e:t.second){
      auto s = print_event(e);
      if(!s.empty()){
        std::cout<<print_event(e)<<std::endl;
      }
    }
  }
    //获取main函数
  add_av_check();//扫描main函数
  mes.print(0,"assertion num:"+ std::to_string(assertion_num)+"\n");
}
void interrupt_atomic_violation_check::add_av_check()
{
  std::set<std::string> ignore_fun={"__CPROVER_initialize","disable_isr","init","enable_isr"};
    FOR_EVERY_TWO_THREAD(t1,t2,per_thread_map){
        std::vector<event_it> & high = DEPTH(t1->second.front())>DEPTH(t2->second.front())?t1->second:t2->second;
        std::vector<event_it> & low = DEPTH(t1->second.front())>DEPTH(t2->second.front())?t2->second:t1->second;
        std::map<irep_idt,std::vector<event_it>> fun_map;
        for(auto e: low){
          if(!e->is_shared_write()&&!e->is_shared_read()){
            continue ;
          }
          if(ignore_fun.find(e->source.function_id.c_str())!=ignore_fun.end()){
            continue ;
          }
          fun_map[e->source.function_id].emplace_back(e);
        }
        for(auto& f_p:fun_map){
          //构建 low的address map
          std::map<decltype(address({})),std::vector<event_it>> a_map;
          for(auto e: f_p.second){
            a_map[address(e)].emplace_back(e);
          }
          for(auto& ap:a_map){
            for(auto e1 = ap.second.begin();e1!=ap.second.end();e1++){
              std::vector<exprt> last_guard;
              for(auto e3 = e1+1;e3!=ap.second.end();e3++){
                if((*e1)->is_shared_read()&&(*e3)->is_shared_read()){
                  add_assertions((*e1),high,goto_trace_stept::typet::SHARED_WRITE,(*e3),last_guard);
                }else if((*e1)->is_shared_read()&&(*e3)->is_shared_write()){
                  add_assertions((*e1),high,goto_trace_stept::typet::SHARED_WRITE,(*e3),last_guard);
                }else if((*e1)->is_shared_write()&&(*e3)->is_shared_read()){
                  add_assertions((*e1),high,goto_trace_stept::typet::SHARED_WRITE,(*e3),last_guard);
                }else if((*e1)->is_shared_write()&&(*e3)->is_shared_write()){
                  add_assertions((*e1),high,goto_trace_stept::typet::SHARED_READ,(*e3),last_guard);
                }
                last_guard.emplace_back(not_exprt((*e3)->guard));
              }
            }
          }
        }
    }
}
std::vector<interrupt_atomic_violation_check::event_it> &interrupt_atomic_violation_check::get_thread_events(thread_idt id) {
#ifdef debugger
  assert(!per_thread_map.empty());
  auto idt = per_thread_map.find(id);
  assert(idt!=per_thread_map.end());
  return idt->second;
#else
  return per_thread_map.find(id)->second;
#endif
}
std::string interrupt_atomic_violation_check::print_event(const event_it &e)
{
  std::string line = e->source.pc->source_location().get_line().c_str();
  std::string type;
  if(e->is_shared_write()){
        type="W";
  }else if(e->is_shared_read()){
        type="R";
  }else{
        return "";
  }
  return std::string("{")+type+":"+e->ssa_lhs.get_l1_object_identifier().c_str()+"("+e->ssa_lhs.get_level_2().c_str()+")"+" in "+" line:"+line+"}";
}
void interrupt_atomic_violation_check::add_assertions(
  event_it e1,
  std::vector<event_it> &e2s,
  EventType e2_t,
  event_it e3,
  std::vector<exprt>& l_guards)
{
  assert(address(e1)== address(e3));
  std::vector<exprt> last_guards;
  for(auto e2:e2s){
      if(address(e1)!= address(e2)){
          continue;
      }
      if(e2->type==e2_t){
          auto it = caches.find(cachet{e1,e2->source.thread_nr,e3});
          if(it==caches.end()){
            exprt symbol = handle.nondet_bool_symbol(print_event(e1)+ std::to_string(e2->source.thread_nr)+ print_event(e2));
            exprt cond = and_exprt(handle.before(e1, get_thread_events(e2->source.thread_nr).front()),
                                   handle.before(get_thread_events(e2->source.thread_nr).back(),e3),
                                   conjunction(l_guards)
                                   );
            handle.add_constraint(equation,implies_exprt(symbol,cond),"av-check-impl",e1->source);
            it = caches.insert(it,std::pair<cachet,exprt>{cachet{e1,e2->source.thread_nr,e3},symbol});
          }
          std::string  id= get_id(e1,e2,e3);
          exprt guard = and_exprt(e1->guard,e2->guard,e3->guard);
          //and_exprt(it->second, conjunction(last_guards))
          add_assertion(e3,id,guard,not_exprt(and_exprt(it->second, conjunction(last_guards))),"av-check");
          last_guards.emplace_back(not_exprt(e2->guard));
      }
  }
}
