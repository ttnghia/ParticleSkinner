This is the implementation code of the paper "A Level-set Method for Skinning Animated Particle Data", but has been modified by adding multi-threading library (Intel TBB).

# Benchmark

The code is not very scalable (memory access dominant rather than computation dominant) thus this modified version cannot gain much performance improvement. I have tested for one of my data set and produced the following result:

* Test on iMac 2013, Intel Core i7-4471 3.5Ghz 4-cores 8 threads: 544 seconds (parallelized) vs 1827 seconds (original code): 3.36X speedup
* Test on Windows, Intel Core i7-6700K 4.0Ghz 4-cores 8 threads: 135 seconds (parallelized) vs 443 seconds (original code): 3.28X speedup
* Test on Linux, Intel Xeon E5-2640 2.5Ghz 6-cores 24 threads: 95 seconds (parallelized) vs 470 seconds (original code): 4.95X speedup

Note: the data input file contains 655K particles and the output mesh contains 4.84 millions triangles.

# References
[https://cal.cs.umbc.edu/Papers/Bhattacharya-2015-ALM/](https://cal.cs.umbc.edu/Papers/Bhattacharya-2015-ALM/)