//
// Created by luca on 07/02/19.
//

#ifndef S2E_FOLLOWTRACE_H
#define S2E_FOLLOWTRACE_H

#include <s2e/Plugins/ExecutionTracers/TestCaseGenerator.h>
#include <s2e/CorePlugin.h>
#include <s2e/Plugin.h>
#include <list>

#define ENFAL 0
#define CREATE_REMOTE_THREAD_ADDR 0x0402446

int DIFF_THRESHOLD = 3;

namespace s2e {
namespace plugins {

class ProcessExecutionDetector;

class Trace: public PluginState {
private:
    std::list<uint64_t> trace;
    bool follow = true;
    int diff_count = 0;
    int threshold = DIFF_THRESHOLD;
    uint64_t prev = 0;

public:
    Trace() {}
    Trace(const std::list<uint64_t>& _trace, int _diff_count, uint64_t _prev) {
        trace = _trace;
        diff_count = _diff_count;
        prev = _prev;
    }
    ~Trace() {}

    static PluginState *factory(Plugin*, S2EExecutionState*) {
        return new Trace();
    }

    Trace *clone() const {
        return new Trace(this->trace, this->diff_count, this->prev);
    }

    bool is_trace_empty() {
        return trace.empty();
    }

    void set_trace(const std::list<uint64_t>& _trace) {
        trace = _trace;
    }

    void set_threshold(int _threshold) {
        this->threshold = _threshold;
        DIFF_THRESHOLD = _threshold;
    }

    uint64_t get_front() {
        return trace.front();
    }

    void unfollow (){
        this->follow = false;
    }

    bool consume(uint64_t pc) {
        if (!follow) return true;
        if (trace.empty()) return false;

        if (trace.front() == pc) {
            trace.pop_front();
            diff_count = 0;
            return true;
        } else {
            int i = 0; int new_treshold = threshold;
            uint64_t prec = 0;
            for (uint64_t el: trace) {

                if (el == pc) {
                    for (int j=0; j<=i; ++j)
                        trace.pop_front();
                    diff_count = 0;
                    return true;
                }
                if (i > new_treshold) break;
                if (prec == el) new_treshold++;
                i++;
                prec = el;
            }
            if (diff_count >= threshold) {
                diff_count = 0;
                return false;
            } else {
                diff_count++;
                return true;
            }
        }
    }

};


class FollowTrace : public Plugin {
    S2E_PLUGIN

public:
    FollowTrace(S2E *s2e) : Plugin(s2e) {
    }

    void initialize();

private:
    ProcessExecutionDetector *m_procDetector;
    testcases::TestCaseGenerator* m_testGen;
    std::string log_filename;
    uint64_t begin_address;
    uint64_t end_address;

    int threshold;
    bool stop_on_target;

    bool initialized;

    void onTranslateInstruction(ExecutionSignal *signal, S2EExecutionState *state,
                                TranslationBlock *tb, uint64_t pc);

    void killState(S2EExecutionState *state, uint64_t pc, std::string mex);
    void follow(S2EExecutionState *state, uint64_t pc);
    void setTrace(Trace* t);
    void updateBeginEndENFAL(S2EExecutionState *state, uint64_t pc);
};
}
}


#endif //S2E_FOLLOWTRACE_H
