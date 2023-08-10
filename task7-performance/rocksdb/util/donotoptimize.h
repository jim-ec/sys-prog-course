#pragma once

#if !defined(doNotOptimize)
#define doNotOptimize(x) asm volatile("" ::"g"(x))
#endif //doNotOptimize
