// iterator_traits.h
#ifndef ITERATOR_TRAITS_H
#define ITERATOR_TRAITS_H

// 主模板定义
template <typename T>
struct iterator_traits {
    using value_type        = typename T::value_type;
    using difference_type   = typename T::difference_type;
    using iterator_category = typename T::iterator_category;
    using pointer           = typename T::pointer;
    using reference         = typename T::reference;
};

// 指针特化版本
template <typename T>
struct iterator_traits<T*> {
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using pointer           = T*;
    using reference         = T&;
};

// 常量指针特化版本
template <typename T>
struct iterator_traits<const T*> {
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using pointer           = const T*;
    using reference         = const T&;
};

#endif // ITERATOR_TRAITS_H
