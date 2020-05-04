#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <random>
#include <ctime>

#include "double.h"
#include "linear.h"

using std::cout;
using std::endl;
using std::ifstream;

typedef uint8_t mykey_t;
typedef uint8_t myvalue_t;
typedef float mytime_t;

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif
// OS-specific timing
static double seconds()
{
#ifdef _WIN32
    LARGE_INTEGER frequency, now;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&now);
    return now.QuadPart / double(frequency.QuadPart);
#else
    timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec + now.tv_usec/1000000.0;
#endif
}

template <typename IndexType>
mytime_t put_throughput(IndexType &index, std::vector<mykey_t> keys) {
    auto start = seconds();
    //顺序插入
    for(int i = 0; i < keys.size(); i += 1) {
        mykey_t key = keys[i];
        //cout << (uint32_t)key << endl;
        index.insert((mykey_t)key, (myvalue_t)key); 
        //index.print();
    } 
    index.print();
    auto end = seconds();
    return mytime_t(end - start);
}

template <typename IndexType>
mytime_t get_throughput(IndexType &index, std::vector<mykey_t> keys) {
    auto start = seconds();
    myvalue_t val;
    for(int i = 0; i < keys.size(); i++) {
        mykey_t key = keys[i];
        if(index.find(key, val)) {
            //cout << (uint32_t)key << " "<< (uint32_t)val << endl;
        } else {
            cout << (uint32_t)key << " "<< 0 << endl;
        } 
    }
    auto end = seconds();
    return mytime_t(end - start);
}

int main(int argc, char ** argv) {
    int test_scale = 100;

    if(argc > 1) {
        test_scale = atoi(argv[1]);
    }
    
    std::vector<mykey_t> keys(test_scale);
    // workload 1: random workload
    // std::default_random_engine e1(99);
    // std::uniform_int_distribution<int> dist(0, UINT8_MAX);
    // for(int i = 0; i < test_scale; i++) {
    //     keys[i] = dist(e1);
    // }
    // int start_bit = 0;
    
    // workload 2:
    //      A   B  C  D   E  F  G   H   I   J   K   L   M  N
    keys = {25, 7, 5, 6, 20, 8, 3, 30, 17, 13, 21, 28, 12, 31};
    int start_bit = 3;

    cout << "========Doubly-extending hashtable========" << endl;
    
    extendible_hash::Double<mykey_t, myvalue_t> hashtable(start_bit); 
    put_throughput(hashtable, keys);
    get_throughput(hashtable, keys);


    cout << "========linearly-extending hashtable========" << endl;
    extendible_hash::Linear<mykey_t, myvalue_t> hashtable2(0.8);
    put_throughput(hashtable2, keys);
    get_throughput(hashtable2, keys);

    return 0;
}
