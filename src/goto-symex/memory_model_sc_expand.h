//
// Created by liu-ye-ye on 2023/6/15.
//

#ifndef CBMC_MEMORY_MODEL_SC_EXPAND_H
#define CBMC_MEMORY_MODEL_SC_EXPAND_H
#include "memory_model_sc.h"
create_memory_model(memory_model_sc_expand,memory_model_sct)
protected:
    override_run(memory_model_sc_expand);
    void read_from(symex_target_equationt &equation) override;
    void write_serialization_external(symex_target_equationt &equation) override;
    void from_read(symex_target_equationt &equation) override;
#ifndef NDEBUG
    virtual void print_other_mes(symex_target_equationt &equation, messaget& mes) {}
#endif
    //可以扩展的部分
    virtual bool isSpecialRF(event_it r, event_it w);
    virtual void addSpecialRF(symex_target_equationt &equation,event_it r,event_it w);
    virtual bool isSpecialWS(event_it w1, event_it w2);
    virtual void addSpecialWS(symex_target_equationt &equation,event_it r,event_it w);
    virtual bool isSpecialFR(event_it r, event_it w1, event_it w2);
    virtual void addSpecialFR(symex_target_equationt &equation,event_it r,event_it w1,event_it w2,exprt& rf_symbol,exprt& ws);
    virtual void make_constraint(symex_target_equationt &equation,message_handlert &);

    static thread_idt  get_thread_id(event_it it);
    std::vector<event_it>& get_thread_events(thread_idt id);
    per_thread_mapt per_thread_map;
#ifndef NDEBUG
    mutable std::map<std::string,int> cons_nums;
    void show_per_thread_map();
    std::string print_event(const event_it& e);
#endif
    void add_constraint(symex_target_equationt &equation, const exprt &cond, const std::string &msg, const symex_targett::sourcet &source) const override;
    virtual event_it front(thread_idt id);
    virtual event_it back(thread_idt id);

create_memory_model_end

    void show_per_thread_map();

const memory_model_sc_expand::event_it null_ptr{};
#endif //CBMC_MEMORY_MODEL_SC_EXPAND_H
