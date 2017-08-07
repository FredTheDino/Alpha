// Kudos to some guy on reddit.
// https://www.reddit.com/r/ProgrammerTIL/comments/58c6dx/til_how_to_defer_in_c/
template <typename F>
struct saucy_defer {
    F f;
    saucy_defer(F f) : f(f) {}
    ~saucy_defer() { f(); }
};

template <typename F>
saucy_defer<F> defer_func(F f) {
    return saucy_defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) =     defer_func([&](){code;})
