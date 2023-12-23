//
// Created by liu-ye-ye on 2023/6/10.
//

#ifndef CBMC_MEMORY_MODEL_UTIL_PROCEDURE_H
#define CBMC_MEMORY_MODEL_UTIL_PROCEDURE_H
#define create_memory_model(class_name,parent_class) static_assert(std::is_base_of<memory_model_baset,parent_class>::value||std::is_same<parent_class,memory_model_baset>::value,"parent_class must be the Derived class of memory_model_baset"); class class_name: public parent_class{public:class_name(const namespacet &_ns,abstract_goto_modelt& goto_model):parent_class(_ns,goto_model){} private:
#define create_memory_model_end };
#define override_run(class_name) public:virtual void operator()(symex_target_equationt &equation, message_handlert &) override
#define run_start(equation,message_handler,class_name) void class_name::operator()(symex_target_equationt &equation, message_handlert &message_handler){messaget log{message_handler};log.statistics() << "Adding constraints" << messaget::eom;build_event_lists(equation, message_handler);build_clock_type();
#define run_end }

#endif //CBMC_MEMORY_MODEL_UTIL_PROCEDURE_H
