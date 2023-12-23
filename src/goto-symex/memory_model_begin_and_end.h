//
// Created by liu-ye-ye on 2023/9/10.
//

#ifndef CBMC_MEMORY_MODEL_BEGIN_AND_END_H
#define CBMC_MEMORY_MODEL_BEGIN_AND_END_H

#include "memory_model_precise_simplest.h"
create_memory_model(memory_model_begin_and_end,memory_model_precise_simplest)
void read_from(symex_target_equationt &equation) override;
void write_serialization_external(symex_target_equationt &equation) override;
exprt first(partial_order_concurrencyt::event_it e1, partial_order_concurrencyt::event_it e2) {
    return before(get_thread_events(get_thread_id(e1)).front(),e2);
}
void from_read(symex_target_equationt &equation) override;
create_memory_model_end
#endif //CBMC_MEMORY_MODEL_BEGIN_AND_END_H
