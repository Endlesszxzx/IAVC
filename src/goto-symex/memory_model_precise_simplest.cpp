//
// Created by liu-ye-ye on 2023/9/10.
//

#include "memory_model_precise_simplest.h"

void memory_model_precise_simplest::nested_isr(symex_target_equationt &equation) {
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
                //1个大小就返回
                if(t2->second.size()<2)continue;
                for(auto e:t1->second)
                {
                    if(e->is_spawn()) continue;
                    exprt cond = implies_exprt(
                            and_exprt(before(b2, e), b2->guard, e->guard), before(e2, e));
                    add_constraint(equation, cond, "nested-isr", b2->source);
                }
            }else if(h1 > h2){
                //1个大小就返回
                if(t1->second.size()<2)continue;
                for(auto e:t2->second)
                {
                    if(e->is_spawn()) continue;
                    exprt cond = implies_exprt(
                            and_exprt(before(b1, e), b1->guard, e->guard), before(e1, e));
                    add_constraint(equation, cond, "nested-isr", b1->source);
                }
            }
        }
}
