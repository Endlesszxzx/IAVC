//
// Created by liu-ye-ye on 2023/6/15.
//

#include "memory_model_sc_expand.h"
#include "analyses/guard_expr.h"
#include "interrupt_atomic_violation_check.h"
#ifndef CBMC_MEMORY_MODEL_SC_EXPAND_C
#define CBMC_MEMORY_MODEL_SC_EXPAND_C
run_start(equation,messageHandlert,memory_model_sc_expand)
    log.set_message_handler(messageHandlert);
    log.statistics()<<"using sc expand memory model"<<messaget::eom;
    build_per_thread_map(equation,per_thread_map);
    //打印一下信息
    log.statistics()<<"start to make constraint"<<messaget::eom;
    make_constraint(equation,messageHandlert);
    log.statistics()<<"success make constraint"<<messaget::eom;
#ifndef NDEBUG
    log.statistics()<<"--------------------log constraint nums:------------------"<<messaget::eom;
    long long unsigned con_nums = 0;
    for(auto& p:cons_nums){
        con_nums+=p.second;
        log.statistics()<<p.first<<" : "<<p.second<<messaget::eom;
    }
    log.statistics()<<"constraints sum : "<<con_nums<<messaget::eom;
    print_other_mes(equation,log);
#endif
    log.statistics()<<"start to add atomic violation assertions"<<messaget::eom;
    interrupt_atomic_violation_check check(equation,messageHandlert,per_thread_map,goto_model,*this);
    check.add_atomic_violation_assertions();
    log.statistics()<<"success add atomic violation assertions"<<messaget::eom;
run_end
void memory_model_sc_expand::read_from(symex_target_equationt &equation) {
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
                    if(isSpecialRF(read_event,write_event))
                        addSpecialRF(equation,read_event,write_event);
                    else
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

void memory_model_sc_expand::write_serialization_external(symex_target_equationt &equation) {
    FOR_EVERY_VARIABLE(variable){
        // This is quadratic in the number of writes
        // per address. Perhaps some better encoding
        // based on 'places'?
        FOR_EVERY_WRITE_PAIR(w_it1, w_it2, GET_VARIABLE_WRITES(variable)){
                // external?
                if(IS_IN_SAME_THREAD(*w_it1, *w_it2))continue;
                if(isSpecialWS(*w_it1,*w_it2)){
                    addSpecialWS(equation,*w_it1,*w_it2);
                }else{
                    // ws is a total order, no two elements have the same rank
                    // s -> w_evt1 before w_evt2; !s -> w_evt2 before w_evt1
                    symbol_exprt s=nondet_bool_symbol("ws-ext");
                    ws_relations.insert({{*w_it1, *w_it2}, s});
                    add_constraint(equation,implies_exprt(s, before(*w_it1, *w_it2)),"ws-ext",(*w_it1)->source);
                    add_constraint(equation,implies_exprt(not_exprt(s), before(*w_it2, *w_it1)),"ws-ext",(*w_it1)->source);
                }
                //show(*w_it1,*w_it2);
            }
    }
}
void memory_model_sc_expand::from_read(symex_target_equationt &equation)
{
    // from-read: (w', w) in ws and (w', r) in rf -> (r, w) in fr
    FOR_EVERY_VARIABLE(variable){
        FOR_EVERY_WRITE_PAIR(w1, w2, GET_VARIABLE_WRITES(variable)){
                exprt ws1, ws2;
                //ws只有两种 wsi  wse
                if(po(*w1, *w2) &&!program_order_is_relaxed(*w1, *w2)){
                    ws1=true_exprt();
                    ws2=false_exprt();
                }
                else if(po(*w2, *w1)&&!program_order_is_relaxed(*w2, *w1)){
                    ws1=false_exprt();
                    ws2=true_exprt();
                }else{
                    auto p = ws_relations.find({*w1,*w2});
                    if(p!=ws_relations.end()){
                        ws1 = p->second;
                        ws2 = not_exprt(p->second);
                    }else{
                        p = ws_relations.find({*w2,*w1});
                        assert(p!=ws_relations.end());
                        ws2 = p->second;
                        ws1 = not_exprt(p->second);
                    }
                }
                // smells like cubic
                FOR_EVERY_RF_RELATION(rf_relation){
                    GET_MES_FROM_RELATION(r_e, w_e, rf_symbol, rf_relation);
                    exprt cond;
                    cond.make_nil();
                    if(w_e==*w1 && !ws1.is_false()){
                        if(isSpecialFR(r_e,*w1,*w2)){
                            addSpecialFR(equation,r_e,*w1,*w2,rf_symbol,ws1);
                            continue;
                        }
                        exprt fr=before(r_e, *w2);
                        // the guard of w_prime follows from rf; with rfi
                        // optimisation such as the previous write_symbol_primed
                        // it would even be wrong to add this guard
                        const auto and_exp = and_exprt( (*w2)->guard, ws1, rf_symbol,r_e->guard);
                        cond=implies_exprt(and_exp,fr);
                        RECORD_FR_RELATION(r_e, *w2, and_exp);
                    }else if(w_e==*w2 && !ws2.is_false()){
                        if(isSpecialFR(r_e,*w2,*w1)){
                            addSpecialFR(equation,r_e,*w2,*w1,rf_symbol,ws2);
                            continue;
                        }
                        exprt fr=before(r_e, *w1);
                        // the guard of w follows from rf; with rfi
                        // optimisation such as the previous write_symbol_primed
                        // it would even be wrong to add this guard
                        const auto and_exp = and_exprt((*w1)->guard, ws2, rf_symbol,r_e->guard);
                        cond=implies_exprt(and_exp,fr);
                        RECORD_FR_RELATION(r_e, *w1, and_exp);
                    }
                    if(cond.is_not_nil()) add_constraint(equation,cond, "fr", r_e->source);
                }
            }
    }
}

bool
memory_model_sc_expand::isSpecialRF(partial_order_concurrencyt::event_it r, partial_order_concurrencyt::event_it w) {
    return false;
}

void memory_model_sc_expand::addSpecialRF(symex_target_equationt &equation, partial_order_concurrencyt::event_it r,
                                          partial_order_concurrencyt::event_it w) {

}

void memory_model_sc_expand::addSpecialWS(symex_target_equationt &equation, partial_order_concurrencyt::event_it r,
                                          partial_order_concurrencyt::event_it w) {

}

bool
memory_model_sc_expand::isSpecialWS(partial_order_concurrencyt::event_it w1, partial_order_concurrencyt::event_it w2) {
    return false;
}

bool
memory_model_sc_expand::isSpecialFR(partial_order_concurrencyt::event_it r, partial_order_concurrencyt::event_it w1,
                                    partial_order_concurrencyt::event_it w2) {
    return false;
}

void memory_model_sc_expand::addSpecialFR(symex_target_equationt &equation, partial_order_concurrencyt::event_it r,
                                          partial_order_concurrencyt::event_it w1,
                                          partial_order_concurrencyt::event_it w2,exprt& rf_symbol,exprt& ws) {

}
memory_model_sc_expand::thread_idt
memory_model_sc_expand::get_thread_id(event_it it) {
    return it->source.thread_nr;
}
std::vector<memory_model_sc_expand::event_it> &memory_model_sc_expand::get_thread_events(thread_idt id) {
    #ifndef NDEBUG
    assert(!per_thread_map.empty());
    auto idt = per_thread_map.find(id);
    assert(idt!=per_thread_map.end());
    return idt->second;
    #else
    return per_thread_map.find(id)->second;
    #endif
}

void memory_model_sc_expand::make_constraint(symex_target_equationt &equation, message_handlert &) {
    program_order(equation);
    read_from(equation);
    write_serialization_external(equation);
    from_read(equation);
}

void memory_model_sc_expand::add_constraint(symex_target_equationt &equation, const exprt &cond, const std::string &msg,
                                            const symex_targett::sourcet &source) const {
    partial_order_concurrencyt::add_constraint(equation, cond, msg, source);
#ifdef debugger
    cons_nums[msg]++;
#endif
}
#ifdef debugger
void memory_model_sc_expand::show_per_thread_map() {
    for(auto& p:per_thread_map){
        std::cout<<"*********************************"<<p.second.front()->source.function_id<<"线程开始*********************************"<<std::endl;
        for(auto& v:p.second){
            std::cout<<"------------------------------event----------------------------------"<<std::endl;
            if(v->is_shared_read()){
                std::cout<<"read:"<<id(v)<<std::endl;
            } else if(v->is_shared_write()){
                std::cout<<"write:"<<id(v)<<std::endl;
            }else if(v->is_spawn()){
                std::cout<<"spawn"<<id(v)<<std::endl;
            }
            std::cout<<"---------------------------------------------------------------------"<<std::endl;
        }
        std::cout<<"**********************************线程结束********************************"<<std::endl;
    }
}
#endif
partial_order_concurrencyt::event_it memory_model_sc_expand::front(thread_idt id) {
#ifdef debugger
    auto& events = get_thread_events(id);
    assert(!events.empty());
    return events.front();
#else
    return get_thread_events(id).front();
#endif
}
memory_model_baset::event_it memory_model_sc_expand::back(thread_idt id){
#ifdef debugger
    auto& events = get_thread_events(id);
    assert(!events.empty());
    return events.back();
#else
    return get_thread_events(id).back();
#endif
}
#ifndef NDEBUG
std::string memory_model_sc_expand::print_event(const event_it &e) {
//    std::string file = e->source.pc->source_location().get_file().c_str();
    std::string line = e->source.pc->source_location().get_line().c_str();
    return std::string("{")+e->ssa_lhs.get_l1_object_identifier().c_str()+" in "+" line:"+line+"}";
}
#endif
#endif
