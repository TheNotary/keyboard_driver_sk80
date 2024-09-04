#include <algorithm> // `std::sort`
#include <cmath>     // `std::pow`
#include <cstdint>   // `int32_t`
#include <cstdlib>   // `std::rand`
#include <execution> // `std::execution::par_unseq`
#include <new>       // `std::launder`
#include <random>    // `std::mt19937`
#include <vector>    // `std::algorithm`
#include <benchmark/benchmark.h>

using namespace std;

namespace bm = benchmark;


static void i32_addition(bm::State& state) {
    int32_t a = 0, b = 0, c = 0;
    for (auto _ : state)
        c = a + b;

    // Silence "variable ‘c’ set but not used" warning
    (void)c;
}

// The compiler will just optimize everything out.
// After the first run, the value of `c` won't change.
// The benchmark will show 0ns per iteration.

//BENCHMARK(i32_addition);

static void i32_addition_random(bm::State& state) {
    int32_t c = 0;
    for (auto _ : state)
        c = std::rand() + std::rand();

    // Silence "variable ‘c’ set but not used" warning
    (void)c;
}

// This run in 25ns, or about 100 CPU cycles.
// Is integer addition really that expensive?

//BENCHMARK(i32_addition_random);


static void i32_addition_semi_random(bm::State& state) {
    int32_t a = std::rand(), b = std::rand(), c = 0;
    for (auto _ : state)
        bm::DoNotOptimize((++a) + (++b));
}

BENCHMARK(i32_addition_semi_random);

// i32_addition_semi_random       1.08 ns        0.460 ns   2240000000
// i32_addition_semi_random       1.10 ns        0.516 ns   1120000000
// i32_addition_semi_random       1.08 ns        0.396 ns   2640842105
// i32_addition_semi_random      0.858 ns        0.254 ns   3136000000

// An optimized version of Bubble Sort
static void GoodBubbleSort(int arr[], int n)
{
    int i, j;
    bool swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }

        // If no two elements were swapped
        // by inner loop, then break
        if (swapped == false)
            break;
    }
}

//static void UseGoodBubbleSort(bm::State& state) {
//    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 90 };
//
//    for (auto _ : state) {
//        std::vector<int> copyArr = arr;
//        GoodBubbleSort(copyArr, sizeof(copyArr));
//        RandomizeArray(arr);
//    }
//}


static void BubbleSort(std::vector<int>& arr) {
    size_t n = arr.size();
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
            }
        }
    }
}


static void RandomizeArray(std::vector<int>& arr) {
    size_t n = arr.size();
    for (size_t i = 0; i < n; i++) {
        ++arr[0];
    }
}


static void UseBubbleSort(bm::State& state) {

    // 3 elements
    //std::vector<int> arr = { 64, 34, 25 };


    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 90 };
    // std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    // 10ish elements
    //std::vector<int> arr = { 64, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12 };
    
    // 190 elements
    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    std::vector<int> copyArr = arr;

    // Warm CPU cache
    for (int i = 0; i < 1000; ++i) {
        copyArr = arr;
        BubbleSort(copyArr);
        //RandomizeArray(arr);
    }


    for (auto _ : state) {
        copyArr = arr;
        BubbleSort(copyArr);
        //RandomizeArray(arr);
    }
}

BENCHMARK(UseBubbleSort);
// UseBubbleSort                  15.3 ns         8.23 ns    112000000
// UseBubbleSort                  15.3 ns         8.23 ns    112000000
// UseBubbleSort                  16.4 ns         6.72 ns    100000000


static void UseRandomizeArray(bm::State& state) {

    // 3 elements
    //std::vector<int> arr = { 64, 34, 25 };

    // 190 elements
    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };

    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    std::vector<int> copyArr = arr;

    // Warm CPU cache
    for (int i = 0; i < 1000; ++i) {
        copyArr = arr;
        RandomizeArray(arr);
    }

    for (auto _ : state) {
        copyArr = arr;
        RandomizeArray(arr);
    }
}


BENCHMARK(UseRandomizeArray);
// UseRandomizeArray              1.56 ns        0.903 ns    640000000


static int Partition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}


static void QuickSort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = Partition(arr, low, high);
        QuickSort(arr, low, pi - 1);
        QuickSort(arr, pi + 1, high);
    }
}


static void UseQuickSort(bm::State& state) {
    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    // 3 elements
    //std::vector<int> arr = { 64, 34, 25 };

    // 10 elements
    //std::vector<int> arr = { 64, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12 };

    // 190 elements
    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    std::vector<int> copyArr = arr;

    // Warm CPU cache
    for (int i = 0; i < 1000; ++i) {
        copyArr = arr;
        int begin = 0;
        int end = arr.size() - 1;
        QuickSort(copyArr, begin, end);
        //RandomizeArray(arr);
    }

    for (auto _ : state) {
        copyArr = arr;
        int begin = 0;
        int end = arr.size() - 1;
        QuickSort(copyArr, begin, end);
        //RandomizeArray(arr);
    }
}

BENCHMARK(UseQuickSort);
//UseQuickSort                   48.0 ns         23.5 ns     29866667

static void UseStdSort(bm::State& state) {
    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    // 3 elements
    //std::vector<int> arr = { 64, 34, 25 };

    // 10 elements
    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12 };

    // 190 elements
    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    
    std::vector<int> copyArr = arr;

    for (int i = 0; i < 1000; ++i) {
        copyArr = arr;
        /*int begin = 0;
        int end = arr.size() - 1;*/
        std::sort(copyArr.begin(), copyArr.end());
        //RandomizeArray(arr);
    }

    for (auto _ : state) {
        copyArr = arr;
        /*int begin = 0;
        int end = arr.size() - 1;*/
        std::sort(copyArr.begin(), copyArr.end());
        //RandomizeArray(arr);
    }
}
BENCHMARK(UseStdSort);

void swapValue(int* a, int* b)
{
    int* temp = a;
    a = b;
    b = temp;
    return;
}

/* Function to sort an array using insertion sort*/
void InsertionSort(int arr[], int* begin, int* end)
{
    // Get the left and the right index of the subarray
    // to be sorted
    int left = begin - arr;
    int right = end - arr;

    for (int i = left + 1; i <= right; i++) {
        int key = arr[i];
        int j = i - 1;

        /* Move elements of arr[0..i-1], that are
                greater than key, to one position ahead
                of their current position */
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }

    return;
}

// A function to partition the array and return
// the partition point
int* Partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element

    for (int j = low; j <= high - 1; j++) {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot) {
            // increment index of smaller element
            i++;

            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (arr + i + 1);
}

// A function that find the middle of the
// values pointed by the pointers a, b, c
// and return that pointer
int* MedianOfThree(int* a, int* b, int* c)
{
    if (*a < *b && *b < *c)
        return (b);

    if (*a < *c && *c <= *b)
        return (c);

    if (*b <= *a && *a < *c)
        return (a);

    if (*b < *c && *c <= *a)
        return (c);

    if (*c <= *a && *a < *b)
        return (a);

    if (*c <= *b && *b <= *a)
        return (b);
}


void IntrosortUtil(int arr[], int* begin, int* end,
    int depthLimit)
{
    // Count the number of elements
    int size = end - begin;

    // If partition size is low then do insertion sort
    if (size < 16) {
        InsertionSort(arr, begin, end);
        return;
    }

    // If the depth is zero use heapsort
    if (depthLimit == 0) {
        make_heap(begin, end + 1);
        sort_heap(begin, end + 1);
        return;
    }

    // Else use a median-of-three concept to
    // find a good pivot
    int* pivot
        = MedianOfThree(begin, begin + size / 2, end);

    // Swap the values pointed by the two pointers
    swapValue(pivot, end);

    // Perform Quick Sort
    int* partitionPoint
        = Partition(arr, begin - arr, end - arr);
    IntrosortUtil(arr, begin, partitionPoint - 1,
        depthLimit - 1);
    IntrosortUtil(arr, partitionPoint + 1, end,
        depthLimit - 1);

    return;
}

/* Implementation of introsort*/
void Introsort(int arr[], int* begin, int* end)
{
    int depthLimit = 2 * log(end - begin);

    // Perform a recursive Introsort
    IntrosortUtil(arr, begin, end, depthLimit);

    return;
}

static void UseIntroSort(bm::State& state) {
    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };

    // 3 elements
    //std::vector<int> arr = { 64, 34, 25 };

    // 10 elements
    //std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12 };

    // 190 elements
    std::vector<int> arr = { 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 11, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 64, 34, 25, 12, 22, 11, 90 };
    std::vector<int> copyArr = arr;

    for (int i = 0; i < 1000; ++i) {
        
    }

    for (auto _ : state) {
        copyArr = arr;
        // int arr[190] = copyArr.arr
        int* a = &copyArr[0]; // cast vector to buffer

        Introsort(a, a, a + 189);
    }
}
BENCHMARK(UseIntroSort);


// Notes:
// Bubble Sort: O(n^2)
// Quick  Sort: O(n log(n))
//
//   n = 10
// 10^2 = 100
// 10 * log(10) = 10
// 
//   n = 11
// 11^2 = 121
// 11 * log(11) = 11.45
// 
// 
// 
// so at n = 10, quickSort should be an order of magnitude faster than bubble sort
// The below measurements n = 10
//UseBubbleSort           71.5 ns         36.0 ns     18666667
//UseRandomizeArray       6.05 ns         3.51 ns    235789474
//UseQuickSort            65.3 ns         32.2 ns     20363636
//UseStdSort              61.3 ns         31.4 ns     22400000

// n = 11
//UseBubbleSort           68.4 ns         29.8 ns     29866667
//UseRandomizeArray       6.48 ns         3.52 ns    448000000
//UseQuickSort            72.5 ns         48.1 ns     29866667
//UseStdSort              63.5 ns         33.8 ns     20363636


// After tweaking bios, disabling CPU C states, turbo boost, and mostly fixing the multiplier/ frontside bus:

/*
i32_addition_semi_random       1.32 ns         1.62 ns    560000000
UseBubbleSort                  65.6 ns         71.9 ns     10000000
UseRandomizeArray              1.30 ns         1.42 ns    640000000
UseQuickSort                   64.7 ns         57.2 ns     11200000
UseStdSort                     62.0 ns         55.5 ns     12389136

i32_addition_semi_random       1.36 ns         1.40 ns    448000000
UseBubbleSort                  64.0 ns         66.3 ns      8960000
UseRandomizeArray              1.30 ns         1.32 ns    640000000
UseQuickSort                   62.7 ns         60.9 ns     10000000
UseStdSort                     68.9 ns         61.4 ns     11200000

i32_addition_semi_random       1.29 ns         1.29 ns    896000000
UseBubbleSort                  59.7 ns         58.0 ns     15346542
UseRandomizeArray              1.21 ns         1.17 ns    896000000
UseQuickSort                   61.6 ns         58.6 ns     11200000
UseStdSort                     70.4 ns         57.8 ns     10000000

i32_addition_semi_random       1.25 ns         1.08 ns    462451613
UseBubbleSort                  59.0 ns         59.4 ns     10000000
UseRandomizeArray              1.23 ns         1.12 ns    640000000
UseQuickSort                   65.8 ns         57.8 ns     10000000
UseStdSort                     64.0 ns         57.2 ns     11200000

i32_addition_semi_random       1.33 ns         1.23 ns    560000000
UseBubbleSort                  58.2 ns         55.8 ns     11200000
UseRandomizeArray              1.22 ns         1.11 ns    746666667
UseQuickSort                   59.6 ns         55.8 ns     11200000
UseStdSort                     61.6 ns         64.2 ns     11200000

*/

/* 
* 
* Better (or worse?) memory utilization * note n = 3

i32_addition_semi_random       1.38 ns         1.26 ns    448000000
UseBubbleSort                  9.80 ns         8.79 ns     74666667
UseRandomizeArray              5.14 ns         5.16 ns    100000000
UseQuickSort                   13.2 ns         10.9 ns     56000000
UseStdSort                     17.9 ns         18.6 ns     32000000

i32_addition_semi_random       1.35 ns         1.30 ns    746666667
UseBubbleSort                  9.05 ns         8.54 ns     89600000
UseRandomizeArray              4.79 ns         5.30 ns    144516129
UseQuickSort                   13.2 ns         12.9 ns     44800000
UseStdSort                     18.1 ns         18.0 ns     37333333

i32_addition_semi_random       1.29 ns         1.19 ns    497777778
UseBubbleSort                  9.55 ns         8.79 ns     74666667
UseRandomizeArray              4.81 ns         4.87 ns    144516129
UseQuickSort                   12.8 ns         11.6 ns     49777778
UseStdSort                     18.4 ns         15.0 ns     40727273

i32_addition_semi_random       1.41 ns         1.19 ns    448000000
UseBubbleSort                  9.16 ns         10.1 ns     89600000
UseRandomizeArray              4.72 ns         5.19 ns    144516129
UseQuickSort                   13.0 ns         11.7 ns     56000000
UseStdSort                     17.4 ns         17.0 ns     49777778

i32_addition_semi_random       1.34 ns         1.31 ns    512000000
UseBubbleSort                  9.07 ns         8.89 ns     89600000
UseRandomizeArray              4.74 ns         4.60 ns    112000000
UseQuickSort                   13.3 ns         11.8 ns     59379881
UseStdSort                     18.3 ns         21.1 ns     40727273

i32_addition_semi_random       1.35 ns         1.48 ns    560000000
UseBubbleSort                  9.46 ns         10.7 ns     74666667
UseRandomizeArray              4.78 ns         4.71 ns    165925926
UseQuickSort                   13.4 ns         12.6 ns     44800000
UseStdSort                     18.3 ns         19.2 ns     40727273

i32_addition_semi_random       1.38 ns         1.54 ns    640000000
UseBubbleSort                  9.28 ns         8.89 ns     89600000
UseRandomizeArray              5.22 ns         5.31 ns    100000000
UseQuickSort                   13.1 ns         13.1 ns     89600000
UseStdSort                     17.6 ns         16.6 ns     32000000

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~ After adding cache warming to quicksort and randomize array tests ~ 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

i32_addition_semi_random       1.64 ns         1.95 ns    497777778
UseBubbleSort                  9.36 ns         8.79 ns     74666667
UseRandomizeArray              5.01 ns         4.43 ns    165925926
UseQuickSort                   13.1 ns         11.7 ns     74666667
UseStdSort                     16.6 ns         15.6 ns     64000000

i32_addition_semi_random       1.59 ns         1.64 ns    640000000
UseBubbleSort                  9.33 ns         8.79 ns     64000000
UseRandomizeArray              5.16 ns         5.31 ns    100000000
UseQuickSort                   13.0 ns         12.3 ns     74666667
UseStdSort                     17.7 ns         16.7 ns     37333333

i32_addition_semi_random       1.65 ns         1.63 ns    498024814
UseBubbleSort                  8.80 ns         8.91 ns    100000000
UseRandomizeArray              4.79 ns         5.27 ns    165925926
UseQuickSort                   13.0 ns         11.9 ns     49777778
UseStdSort                     17.4 ns         18.1 ns     32000000

i32_addition_semi_random       1.63 ns         1.67 ns    560000000
UseBubbleSort                  9.36 ns         9.84 ns     74666667
UseRandomizeArray              4.81 ns         4.60 ns    112000000
UseQuickSort                   13.6 ns         12.9 ns     64000000
UseStdSort                     18.2 ns         16.5 ns     40727273

i32_addition_semi_random       1.56 ns         1.72 ns    535210667
UseBubbleSort                  8.75 ns         9.00 ns     74666667
UseRandomizeArray              5.15 ns         5.51 ns    167253333
UseQuickSort                   13.0 ns         13.4 ns     56000000
UseStdSort                     17.1 ns         16.3 ns     37333333

There's something fishy about UseStdSort being slower than UseQuickSort, they should be identical if not faster in StdSort... maybe array size throws it off?

*/

