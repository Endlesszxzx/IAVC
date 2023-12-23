//
// Created by liu-ye-ye on 2023/9/10.
//

#ifndef CBMC_MEMORY_MODEL_SPAWN_REDUCTION_H
#define CBMC_MEMORY_MODEL_SPAWN_REDUCTION_H
#include "memory_model_begin_and_end.h"

create_memory_model(memory_model_spawn_reduction,memory_model_begin_and_end)
private:
    std::set<std::pair<event_it,thread_idt>> spawns_before_set;
    std::map<event_it,thread_idt> spawns_mapping;
public:
    /**
     * 用于注册spawn相关的信息
     * 如befores 和 spawns_mapping
     * */
    int relaxing_nums = 0;
    bool spawns_before(event_it i1,event_it i2);
    void register_thread_spawn(symex_target_equationt &equation);
    void nested_isr(symex_target_equationt &equation) override;
    /**
     * 可重写的方法  是针对SC模型的扩展
     * */
    bool isSpecialRF(event_it r, event_it w) override;
    void addSpecialRF(symex_target_equationt &equation,event_it r,event_it w) override;
    bool isSpecialWS(event_it w1, event_it w2) override;
    void addSpecialWS(symex_target_equationt &equation,event_it r,event_it w) override;
    bool isSpecialFR(event_it r, event_it w1, event_it w2) override;
    void addSpecialFR(symex_target_equationt &equation,event_it r,event_it w1,event_it w2,exprt& rf_symbol,exprt& ws) override;
    void program_order(symex_target_equationt &equation) override;
    void scan_thread(thread_idt thread_id, const std::vector<event_it>& before_es);
create_memory_model_end
#endif //CBMC_MEMORY_MODEL_SPAWN_REDUCTION_H
