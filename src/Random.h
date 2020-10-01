#pragma once

#include <random>

namespace hiraishi {
    struct Random {
        std::mt19937 engine;
        std::uniform_real_distribution<double> dist;
        std::uniform_int_distribution<> intDist;
        Random() {};
        Random(int seed) {
            engine.seed(seed);
            dist.reset();
            intDist.reset();
        }
        double next() { return dist(engine); }
        int intNext(long min, long max) { return intDist(engine, std::uniform_int_distribution<>::param_type(min, max)); }
    };
}