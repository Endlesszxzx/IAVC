//
// Created by liu-ye-ye on 2023/9/10.
//

#include "memory_model_spawn_reduction.h"


bool memory_model_spawn_reduction::spawns_before(partial_order_concurrencyt::event_it i1,
                                                 partial_order_concurrencyt::event_it i2) {
    return spawns_before_set.find({i1, get_thread_id(i2)})!=spawns_before_set.end();
}

void memory_model_spawn_reduction::register_thread_spawn(symex_target_equationt &equation) {
    thread_idt next_thread_id=0;
    FOR_EVERY_EVENT(e_it, equation){
        if(e_it->is_spawn()){
            auto next_thread=per_thread_map.find(++next_thread_id);
            if(next_thread==per_thread_map.end()) {continue;}
            //添加一个映射关系spawn和thread_id
            spawns_mapping.emplace(e_it, next_thread_id);
        }
    }
    scan_thread(0,{});
    spawns_mapping.clear();
    std::cout<<spawns_before_set.size()<<std::endl;
}

void memory_model_spawn_reduction::nested_isr(symex_target_equationt &equation) {
    FOR_EVERY_TWO_THREAD(t1,t2,per_thread_map){
            if(t1->second.empty()||t2->second.empty()) continue;
            const auto b1 = *(t1->second.begin());
            const auto b2 = *(t2->second.begin());
            const auto e1 = (t1->second.back());
            const auto e2 = (t2->second.back());
            auto h1 = DEPTH(b1);
            auto h2 = DEPTH(b2);
            if(h1 == h2){
                symbol_exprt s = nondet_bool_symbol("ets");
                add_constraint(
                        equation,
                        implies_exprt{s, before(b1, b2)},
                        "ni_eq",
                        b1->source);
                if(t1->second.size()>1) {
                    add_constraint(
                            equation,
                            implies_exprt{s, before(e1, b2)},
                            "ni_eq",
                            b1->source);
                }
                add_constraint(
                        equation,
                        implies_exprt{not_exprt(s), before(b2, b1)},
                        "ni_eq",
                        b2->source);
                if(t2->second.size()>1) {
                    add_constraint(
                            equation,
                            implies_exprt{not_exprt(s), before(e2, b1)},
                            "ni_eq",
                            b2->source);
                }
            }else if(h1 < h2){
                if(t2->second.size()<2)continue;
                for(auto e:t1->second)
                {
                    if(e->is_spawn()) continue;
                    if(spawns_before(e,b2)){
                        relaxing_nums++;
                        if (address(e)== address(b1)){
                            add_constraint(equation, before(e,b2), "nested-isr", b2->source);
                        }
                        continue;
                    }
                    else if(spawns_before(b2,e)){
                        add_constraint(equation, before(e2,e), "nested-isr", b2->source);
                    }
                    else{
                        exprt cond = implies_exprt(
                                and_exprt(before(b2, e), b2->guard, e->guard), before(e2, e));
                        add_constraint(equation, cond, "nested-isr", b2->source);
                    }

                }
            }else if(h1 > h2){
                if(t1->second.size()<2)continue;
                for(auto e:t2->second)
                {
                    if(e->is_spawn()) continue;
                    if(spawns_before(e,b1)) {
                        relaxing_nums++;
                        //9.11:改进 不省去isr 而是替换为明确的方向
//                        add_constraint(equation, before(e,b1), "nested-isr", b2->source);
                        //9.12:改进 改为省区非偏序的
                        if (address(e)== address(b1)){
                            add_constraint(equation, before(e,b1), "nested-isr", b2->source);
                        }
                        continue;
                    }
                    else if(spawns_before(b1,e)){
                        add_constraint(equation, before(e1,e), "nested-isr", b1->source);
                    }
                    else{
                        exprt cond = implies_exprt(
                                and_exprt(before(b1, e), b1->guard, e->guard), before(e1, e));
                        add_constraint(equation, cond, "nested-isr", b2->source);
                    }
                }
            }
        }
}

bool memory_model_spawn_reduction::isSpecialRF(partial_order_concurrencyt::event_it r,
                                               partial_order_concurrencyt::event_it w) {
    relaxing_nums++;
    //基于spawn_before的缩减
    return spawns_before(r,w);
}

void
memory_model_spawn_reduction::addSpecialRF(symex_target_equationt &equation, partial_order_concurrencyt::event_it r,
                                           partial_order_concurrencyt::event_it w) {

}

bool memory_model_spawn_reduction::isSpecialWS(partial_order_concurrencyt::event_it w1,
                                               partial_order_concurrencyt::event_it w2) {
    relaxing_nums++;
    return spawns_before(w1,w2)|| spawns_before(w2,w1);
}

void
memory_model_spawn_reduction::addSpecialWS(symex_target_equationt &equation, partial_order_concurrencyt::event_it w1,
                                           partial_order_concurrencyt::event_it w2) {
    if(spawns_before(w1,w2)){
        add_constraint(equation,before(w1,w2),"spawn-before-ws",w1->source);
        //直接使用before关系就ok
        ws_relations.insert({{w1, w2},true_exprt()});
    }else{
        add_constraint(equation,before(w2,w1),"spawn-before-ws",w2->source);
        ws_relations.insert({{w2, w1},true_exprt()});
    }
}

bool memory_model_spawn_reduction::isSpecialFR(partial_order_concurrencyt::event_it r,
                                               partial_order_concurrencyt::event_it w1,
                                               partial_order_concurrencyt::event_it w2) {
    relaxing_nums++;
    //return spawns_before(w2,r);
    //因为fr是用来排除的
    return spawns_before(r,w2)||spawns_before(w2,r);
}

void
memory_model_spawn_reduction::addSpecialFR(symex_target_equationt &equation, partial_order_concurrencyt::event_it r,
                                           partial_order_concurrencyt::event_it w1,
                                           partial_order_concurrencyt::event_it w2, exprt &rf_symbol, exprt &ws) {
    if(spawns_before(r,w2)){
        //这里显示的表明这个before关系
        add_constraint(equation,
                       before(r,w2),
                       "spawn-before-fr-true",
                       r->source);
        fr_relations.insert({{r,w2},true_exprt()});
    }else{
        //这个fr毫无疑问一定为false
        //w1->w2 => !rf
        add_constraint(equation,
                       implies_exprt(ws,not_exprt(rf_symbol)),
                       "spawn-before-fr-false",
                       r->source);
    }
}

void memory_model_spawn_reduction::program_order(symex_target_equationt &equation) {
    memory_model_sc_expand::program_order(equation);
    register_thread_spawn(equation);
}

void memory_model_spawn_reduction::scan_thread(partial_order_concurrencyt::thread_idt thread_id,
                                               const std::vector<event_it> &before_es) {
    std::vector<event_it> local_before_es;
    for(auto& be:before_es){
        spawns_before_set.emplace(be,thread_id);
    }
    for(auto e: get_thread_events(thread_id)){
        if(e->is_spawn()){
            std::vector<event_it> all_before_es(before_es);
            all_before_es.insert(all_before_es.begin(), local_before_es.begin(), local_before_es.end());
            auto t_id_t = spawns_mapping.find(e);
            if(t_id_t== spawns_mapping.end()) continue;
            scan_thread(t_id_t->second,all_before_es);
        }
        local_before_es.emplace_back(e);
    }
}
