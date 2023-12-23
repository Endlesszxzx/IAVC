//
// Created by liu-ye-ye on 2023/8/2.
//

#ifndef CBMC_MEMORY_MODEL_OLD_PRECISE_H
#define CBMC_MEMORY_MODEL_OLD_PRECISE_H
#include "memory_model_sc_expand.h"

create_memory_model(memory_model_old_precise,memory_model_sc_expand)
public:
    void make_constraint(symex_target_equationt &equation, message_handlert &)override;
    virtual void nested_isr(symex_target_equationt &equation);
    exprt last(const event_it &from, const event_it &to);
create_memory_model_end

#endif //CBMC_MEMORY_MODEL_OLD_PRECISE_H
