README.file
Xu He
he275@purdue.edu
---------------------------------------------------------------------------------------------------------------------------------------
Part 2:
-----------
Output from step 3:
The expected output is "A*B*C" repeatedly being printed. However, the real output was not stable.
Sometimes there were multiple As or Bs before one C, sometimes mutiple Cs after A or B. Sometimes As were followed by Bs. 
Sample output as follows, 
"AAAAAACACACBBBBBBBBBBBBBBBBBBBCBCBCAAAAAAAAAAAAAAAAAACACACBBBBBBBBBBBBBBBBBBBCBCBCBABBBBBBBBBBBBBBBBBABABACCCCCCCCCCCCCCCCCC
ABBBBBBBBBBBBBBBBBABBBAAAAAAAAAAAAAAAAAAABCCCBCBCBCAAAAAAAAAAAAACCCCCCCCCCCCCCABAAAAAAAAAAAAAAAAAAAAABABABABCAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAACCCCCCCCCCCCCCCCCCCCBBBBBBBCBBCBCAAAAAAAAAAAAAAAAAACBCCBCAAAAAAAAAAAAAAAAAAAACCABAAAAAAAAAAAAAAABABABCCCCCC
CCCCCCCCCCCBCBCBABACCCCCCCCCCCCCCCCCCCCACACAACCCCCCCCCCCCCCCCCCCACABBBBBBBBBBBBBBBBBBBBABAABCBBBBBBBBBBBBBBBBCBBCBBCCCCCCCCCCCC"

This is because the threads are not synchoronized, the thread for writing A can take over control and writes "A". Same idea goes to the 
thread wrting "B". Even though printC is not a thread, but its CPU control time is affected by the threads t1 and t2. 

----------------------------------------------------------------------------------------------
Output from step 5:
Only "C" is being printed out constantly. 

This is because the threads are created after printf of C, which results no interrupt to the infinitely while loop.
i.e. The command lines of thread creations will never be executed. 
-----------------------------------------------------------------------------------------------------------------------------------------

PART 3:
-------------
Time below is given, following System(Kernel) Time, User Time, and Real Time order.
pthread_mutex (count): 2.616s, 3.164s, 2.950s
spin lock (count_spin with thr_yield): 0.315s, 0.896s, 0.624s
spin_lock (count_spin without thr_yield): 0.020s, 7.048s, 3.585s

1. Explain the differences in user time between count_spin with and without thr_yield.
The user time of count_spin without thr_yield is enormous compared to that of with thr_yield.
This is because with yield command, the program is willingly give up CPU whereas without yield command will hold on to CPU
until the system forces it to give it up. 

2. Explain the difference in system time between count and count_spin with thr_yield.
The sys time of count is way longer than that of with thr_yield. This is because the spin clock gives up CPU willingly whereas the count has a context
exchange and requires more CPU time.
