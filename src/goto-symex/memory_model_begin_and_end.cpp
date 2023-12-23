//
// Created by liu-ye-ye on 2023/9/10.
//

#include "memory_model_begin_and_end.h"

void memory_model_begin_and_end::read_from(symex_target_equationt &equation) {
    memory_model_sc_expand::read_from(equation);
    FOR_EVERY_RF_RELATION(rf){
        GET_MES_FROM_RELATION(r,w,rf_symbol,rf);
        assert(!po(r, w));
        if(IS_IN_SAME_THREAD(r,w)) continue;
        if(DEPTH(w)>DEPTH(r)){
            // must use before(w, r) instead of c_it.second
            exprt cond=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    last(w, r));
            add_constraint(
                    equation, cond, "rf-end", r->source);
            exprt cond2=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    first(w, r));
            add_constraint(equation, cond2, "rf-begin", r->source);
        }else if(DEPTH(w)<DEPTH(r)){
            // must use before(w, r) instead of c_it.second
            exprt cond=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    before(w, front(get_thread_id(r))));
            add_constraint(
                    equation, cond, "rf-end", r->source);
            exprt cond2=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    before(w, back(get_thread_id(r))));
            add_constraint(equation, cond2, "rf-begin", r->source);
        }else{
            exprt cond=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    before(front(get_thread_id(w)), front(get_thread_id(r))));
            add_constraint(
                    equation, cond, "rf-end", r->source);
            exprt cond2=implies_exprt(
                    and_exprt(rf_symbol, w->guard, r->guard),
                    before(back(get_thread_id(w)), front(get_thread_id(r))));
            add_constraint(equation, cond2, "rf-begin", r->source);
        }
    }
}

void memory_model_begin_and_end::write_serialization_external(symex_target_equationt &equation) {
    memory_model_sc_expand::write_serialization_external(equation);
    FOR_EVERY_WS_RELATION(ws_relation){
        GET_MES_FROM_RELATION(w1,w2,ws,ws_relation);
        if(DEPTH(w1)> DEPTH(w2))
        {
            add_constraint(
                    equation,
                    implies_exprt(
                            and_exprt(ws, w1->guard, w2->guard),
                            last(w1, w2)),
                    "ws-end",
                    w1->source);
            add_constraint(
                    equation,
                    implies_exprt(
                            and_exprt(ws, w1->guard, w2->guard),
                            first(w1,w2)),
                    "ws-begin",
                    w1->source);
        }else if(DEPTH(w2)> DEPTH(w1)){
            add_constraint(
                    equation,
                    implies_exprt(
                            and_exprt(not_exprt(ws), w1->guard, w2->guard),
                            last(w2, w1)),
                    "ws-end",
                    w1->source);
            add_constraint(
                    equation,
                    implies_exprt(
                            and_exprt(not_exprt(ws), w1->guard, w2->guard),
                            first(w2,w1)),
                    "ws-begin",
                    w1->source);
        } else{
            add_constraint(
                    equation,
                    and_exprt(
                            implies_exprt(
                                    and_exprt(ws, w1->guard, w2->guard),
                                    before(front(get_thread_id(w1)),front(get_thread_id(w2)))),
                            implies_exprt(
                                    and_exprt(not_exprt(ws), w1->guard, w2->guard),
                                    before(front(get_thread_id(w2)),front(get_thread_id(w1)))
                                    )
                              ),
                    "ws-end",
                    w1->source);
            add_constraint(
                    equation,
                    and_exprt(
                            implies_exprt(
                                    and_exprt(ws, w1->guard, w2->guard),
                                    before(back(get_thread_id(w1)),front(get_thread_id(w2)))),
                            implies_exprt(
                                    and_exprt(not_exprt(ws), w1->guard, w2->guard),
                                    before(back(get_thread_id(w2)),front(get_thread_id(w1)))
                            )
                    ),
                    "ws-begin",
                    w1->source);
        }
    }
}

void memory_model_begin_and_end::from_read(symex_target_equationt &equation) {
    memory_model_sc_expand::from_read(equation);
    FOR_EVERY_FR_RELATION(fr_relation){
        GET_MES_FROM_RELATION(r,w,fr,fr_relation);
        if(IS_IN_SAME_THREAD(r,w)) continue;
        if(DEPTH(r)> DEPTH(w)){
            auto cond=implies_exprt(
                    fr,
                    last(r, w));
            auto cond2=implies_exprt(
                    fr,
                    first(r,w));
            add_constraint(equation, cond, "fr-end", r->source);
            add_constraint(equation, cond2, "fr-begin", r->source);
        }else if(DEPTH(r)< DEPTH(w)){
            auto cond=implies_exprt(
                    fr,
                    before(r, front(get_thread_id(w))));
            auto cond2=implies_exprt(
                    fr,
                    before(r, back(get_thread_id(w))));
            add_constraint(equation, cond, "fr-end", r->source);
            add_constraint(equation, cond2, "fr-begin", r->source);
        }else{
            auto cond=implies_exprt(
                    fr,
                    before(front(get_thread_id(r)), back(get_thread_id(w))));
            auto cond2=implies_exprt(
                    fr,
                    before(back(get_thread_id(r)), front(get_thread_id(w))));
            add_constraint(equation, cond, "fr-end", r->source);
            add_constraint(equation, cond2, "fr-begin", r->source);
        }
    }
}
