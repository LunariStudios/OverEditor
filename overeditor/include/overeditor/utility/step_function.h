#ifndef OVEREDITOR_STEPFUNCTION_H
#define OVEREDITOR_STEPFUNCTION_H

#include <functional>

namespace overeditor {
    template<typename... T>
    class Event {
    public:
        typedef std::function<void(T...)> EventListener;
    private:
        std::vector<EventListener *> listeners;
    public:
        void clear() {
            listeners.clear();
        }

        void operator+=(EventListener *listener) {
            listeners.push_back(listener);
        }

        void operator-=(EventListener *listener) {
            listeners.erase(std::remove(listeners.begin(), listeners.end(), listener));
        }

        void operator()(T... args) {
            for (EventListener *listener : listeners) {
                listener->operator()(args...);
            }
        }
    };

    template<typename ...T>
    class StepFunction {
    private:
        Event<T...> earlyStep;
        Event<T...> lateStep;
    public:
        StepFunction() : earlyStep(), lateStep() {

        }

        void clear() {
            earlyStep.clear();
            lateStep.clear();
        }

        Event<T...> &getEarlyStep() {
            return earlyStep;
        }

        Event<T...> &getLateStep() {
            return lateStep;

        }

        void operator()(T... params) {
            earlyStep(params...);
            lateStep(params...);
        }
    };
}
#endif //OVEREDITOR_STEPFUNCTION_H
