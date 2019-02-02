#ifndef OVEREDITOR_STEPFUNCTION_H
#define OVEREDITOR_STEPFUNCTION_H

#include <functional>

namespace overeditor::utility {
    template<typename ...T>
    class StepFunction {
    public:
        typedef std::function<void(T...)> Listener;
        typedef std::vector<Listener> ListenerHandler;
    private:
        static void notify(const ListenerHandler &handler, T... values);

        std::vector<Listener> earlyListeners, lateListeners;
    public:
        void operator()(T... values);
    };

    template<typename ...T>
    void StepFunction<T...>::operator()(T... values) {
        notify(earlyListeners, ...
        values);
        notify(lateListeners, ...
        values);
    }

    template<typename ...T>
    void StepFunction<T...>::notify(const StepFunction::ListenerHandler &handler, T... values) {
        for (Listener l : handler) {
            l(...
            values);
        }
    }
}
#endif //OVEREDITOR_STEPFUNCTION_H
