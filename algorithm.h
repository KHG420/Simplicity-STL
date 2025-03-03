#include <iostream>

// 交换两个元素的值
template<typename T>
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

// 插入排序函数
template<typename RandomIt, typename Compare>
void insertionSort(RandomIt first, RandomIt last, Compare comp) {
    for (RandomIt i = first + 1; i != last; ++i) {
        for (RandomIt j = i; j > first && comp(*j, *(j - 1)); --j) {
            ::swap(*j, *(j - 1));
        }
    }
}

// 调整堆，使其满足最大堆性质
template<typename RandomIt, typename Compare>
void heapify(RandomIt first, int n, int i, Compare comp) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && comp(*(first + largest), *(first + left))) {
        largest = left;
    }

    if (right < n && comp(*(first + largest), *(first + right))) {
        largest = right;
    }

    if (largest != i) {
        ::swap(*(first + i), *(first + largest));
        heapify(first, n, largest, comp);
    }
}

// 堆排序函数
template<typename RandomIt, typename Compare>
void heapSort(RandomIt first, RandomIt last, Compare comp) {
    int n = last - first;
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(first, n, i, comp);
    }

    for (int i = n - 1; i > 0; --i) {
        ::swap(*first, *(first + i));
        heapify(first, i, 0, comp);
    }
}

// 快速排序分区函数
template<typename RandomIt, typename Compare>
RandomIt partition(RandomIt first, RandomIt last, Compare comp) {
    auto pivot = *(first + (last - first - 1) / 2);
    RandomIt left = first;
    RandomIt right = last - 1;

    while (true) {
        while (comp(*left, pivot)) ++left;
        while (comp(pivot, *right)) --right;

        if (left >= right) break;
        ::swap(*left, *right);
        ++left;
        --right;
    }
    return left;
}

// 计算以 2 为底的对数
int log2(int n) {
    int result = 0;
    while (n >>= 1) {
        ++result;
    }
    return result;
}

// 内省排序核心函数
template<typename RandomIt, typename Compare>
void introSort(RandomIt first, RandomIt last, int depthLimit, Compare comp) {
    int n = last - first;
    while (n > 16) {
        if (depthLimit == 0) {
            // 如果递归深度达到限制，切换到堆排序
            heapSort(first, last, comp);
            return;
        }
        --depthLimit;

        RandomIt cut = partition(first, last, comp);
        // 递归处理较小的子数组
        if (last - cut < cut - first) {
            introSort(cut, last, depthLimit, comp);
            last = cut;
        } else {
            introSort(first, cut, depthLimit, comp);
            first = cut;
        }
        n = last - first;
    }
    // 对于小规模数组，使用插入排序
    insertionSort(first, last, comp);
}

// 简化版的 std::sort 函数
template<typename RandomIt, typename Compare = std::less<>>
void sort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    if (first == last) return;
    int depthLimit = 2 * log2(last - first);
    introSort(first, last, depthLimit, comp);
}

