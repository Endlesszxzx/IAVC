//
// Created by liu-ye-ye on 2023/6/9.
//
/**
 * 包含该文件获取更好的体验
 * */
#include "memory_model_util_procedure.h"
#ifndef CBMC_MEMORY_MODEL_UTIL_H
#define CBMC_MEMORY_MODEL_UTIL_H
#define FOR_EVERY_EVENT(__e_it,__equation)\
    for(auto __e_it=__equation.SSA_steps.begin();__e_it!=__equation.SSA_steps.end(); __e_it++)
#define FOR_EVERY_THREAD(__thread,__per_thread_map)\
    for(auto __thread=__per_thread_map.begin(); __thread!=__per_thread_map.end(); __thread++)
#define FOR_EVERY_TWO_THREAD(__thread1,__thread2,__per_thread_map)\
for(auto __thread1=per_thread_map.begin();__thread1!=per_thread_map.end();++__thread1) for(auto __thread2=++(std::map<unsigned, std::vector<event_it>>::iterator{__thread1});__thread2!=per_thread_map.end();++__thread2)

#define GET_THREAD(__thread_id,__per_thread_map) (__per_thread_map.find(__thread_id))
#define GET_THREAD_EVENTS(__thread) ((__thread)->second)
#define RECORD_RELATIONS(__e1,__e2,__symbol,__type) \
    __type##_relations.insert({{__e1,__e2},__symbol})
#define FOR_RELATION(__relation,__types)\
    for(auto __relation=__types##_relations.begin();__relation!=__types##_relations.end();__relation++)
#define FOR_EVERY_RF_RELATION(__relation) FOR_RELATION(__relation,rf)
#define FOR_EVERY_WS_RELATION(__relation) FOR_RELATION(__relation,ws)
#define FOR_EVERY_FR_RELATION(__relation) FOR_RELATION(__relation,fr)
#define GET_MES_FROM_RELATION(__read_e,__write_e,__rf_symbol,__relation)\
    __attribute__((unused)) auto& __read_e = __relation->first.first;\
    __attribute__((unused)) auto& __write_e = __relation->first.second;\
    __attribute__((unused)) auto& __rf_symbol = __relation->second
#define RECORD_RF_RELATION(__read_e,__write_e,__rf_symbol)\
    RECORD_RELATIONS(__read_e,__write_e,__rf_symbol,rf)
#define RECORD_FR_RELATION(__read_e,__write_e,__fr_symbol)\
    RECORD_RELATIONS(__read_e,__write_e,__fr_symbol,fr)
#define IS_MEMORY_MODEL_RELATED(__e_it)\
     ((__e_it)->is_shared_read()||(__e_it)->is_shared_write()||(__e_it)->is_spawn()||(__e_it)->is_memory_barrier())
#define EVENT_THREAD_ID(__e_it) \
      ((__e_it)->source.thread_nr)
#define IS_IN_SAME_THREAD(__e_1,__e_2)  (EVENT_THREAD_ID(__e_1)==EVENT_THREAD_ID(__e_2))
#define FOR_EVERY_VARIABLE(__variable) \
    for(address_mapt::const_iterator __variable=address_map.begin();__variable!=address_map.end();__variable++)
#define GET_VARIABLE_WRITES(__variable) (__variable->second.writes)
#define GET_VARIABLE_READS(__variable) (__variable->second.reads)
#define GET_VARIABLE_ADDRESS(__variable) (__variable->first)
#define GET_VARIABLE(__address)  (address_map.find(__address))
#define VARIABLE_IS_VALID(__variable) (__variable!=address_map.end())
#define CREATE_VARIABLE(__address) (address_map.emplace(__address,decltype(address_map.find("")->second){}))
#define FOR_EVERY_WRITE_PAIR(__w_1,__w_2,__writes) \
        for(auto __w_1=__writes.begin(); __w_1!=__writes.end(); ++__w_1) for(auto __w_2=__w_1+1;__w_2!=__writes.end();++__w_2)
#define DEPTH(__e_it) ((__e_it)->source.priority)
//need equtation
#define CREATE_THREAD_MAP(__name)\
        per_thread_mapt __name;\
        build_per_thread_map(equation,__name)
#endif //CBMC_MEMORY_MODEL_UTIL_H
