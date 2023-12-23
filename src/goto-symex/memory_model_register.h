//
// Created by liu-ye-ye on 2023/6/10.
//
#include "make_unique.h"
#include "memory_model_pso.h"
#include "memory_model_interrupt.h"
#include "memory_model_precise_simplest.h"
#include "memory_model_begin_and_end.h"
#include "memory_model_spawn_reduction.h"
#include "memory_model_old_precise.h"
#ifndef CBMC_MEMORY_MODEL_REGISTER_H
#define CBMC_MEMORY_MODEL_REGISTER_H
using memory_model_maker=std::function<std::unique_ptr<memory_model_baset>(const namespacet &ns,abstract_goto_modelt& goto_model)>;
static std::map<std::string,memory_model_maker> memory_model_handlers;
#define memory_model_register_start void load_memory_model(){
#define memory_model_register_end }
#define register_memory_model(argus,class_name) \
    static_assert(std::is_base_of<memory_model_baset,class_name>::value||std::is_same<class_name,memory_model_baset>::value,"error class name"); \
    memory_model_handlers.insert(               \
      {#argus,[](const namespacet &ns,abstract_goto_modelt& goto_model)->std::unique_ptr<memory_model_baset>{return util_make_unique<class_name>(ns,goto_model);}})
#define get_memory_model_from_argus(argus) (memory_model_handlers.find(argus)==memory_model_handlers.end()?nullptr:memory_model_handlers.find(argus)->second)

memory_model_register_start
//place register memory model there
//    register_memory_model(sc,memory_model_sct);
//    register_memory_model(tso,memory_model_tsot);
//    register_memory_model(pso,memory_model_psot);
//    register_memory_model(irq,memory_model_interruptt);
//    register_memory_model(irq-all,memory_model_interruptAll);
//    register_memory_model(irq-spawn,memory_model_interrupt_spawn_algorithm);
//    register_memory_model(irq-spawn-simplest,memory_model_interrupt_simplest);
//    register_memory_model(irq-spawn-middle,memory_model_interrupt_spawn_middle);
//    register_memory_model(irq-relaxing-rf1,memory_model_interrupt_relaxing_rf1);
//    register_memory_model(irq-relaxing-rf2,memory_model_interrupt_relaxing_rf2);
    //现在需要测试的
    register_memory_model(not_precise,memory_model_interruptt);
    register_memory_model(old_precise,memory_model_old_precise);
    register_memory_model(precise_simplest_module,memory_model_precise_simplest);
    register_memory_model(begin_and_end_module,memory_model_begin_and_end);
    register_memory_model(spawn_reduction_module,memory_model_spawn_reduction);
memory_model_register_end

#endif //CBMC_MEMORY_MODEL_REGISTER_H
