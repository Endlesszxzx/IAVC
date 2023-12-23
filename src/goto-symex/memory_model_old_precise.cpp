//
// Created by liu-ye-ye on 2023/8/2.
//

#include "memory_model_old_precise.h"
void memory_model_old_precise::nested_isr(symex_target_equationt &equation) {
    FOR_EVERY_TWO_THREAD(t1,t2,per_thread_map){
        for(auto e1: t1->second){
            for(auto e2:t2->second){
                if(DEPTH(e1)>= DEPTH(e2)){
                    exprt cond=implies_exprt(and_exprt(before(e1, e2), e1->guard, e2->guard),last(e1, e2));
                    add_constraint(equation, cond, "nested-isr", e1->source);
                }
                if(DEPTH(e1)<= DEPTH(e2)){
                    exprt cond=implies_exprt(and_exprt(before(e2, e1), e1->guard, e2->guard),last(e2, e1));
                    add_constraint(equation, cond, "nested-isr", e2->source);
                }
            }
        }
    }
}

void memory_model_old_precise::make_constraint(symex_target_equationt &equation, message_handlert &mes) {
    program_order(equation);
    read_from(equation);
    write_serialization_external(equation);
    from_read(equation);
    nested_isr(equation);
}
exprt memory_model_old_precise::last(const event_it &from, const event_it &to){
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