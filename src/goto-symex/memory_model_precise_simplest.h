//
// Created by liu-ye-ye on 2023/9/10.
//

#ifndef CBMC_MEMORY_MODEL_PRECISE_SIMPLEST_H
#define CBMC_MEMORY_MODEL_PRECISE_SIMPLEST_H
#include "memory_model_old_precise.h"

create_memory_model(memory_model_precise_simplest,memory_model_old_precise)
void nested_isr(symex_target_equationt &equation) override;
create_memory_model_end

#endif //CBMC_MEMORY_MODEL_PRECISE_SIMPLEST_H
