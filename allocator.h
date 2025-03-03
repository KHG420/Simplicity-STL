#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include<memory>
using namespace std;

template <typename T>
class Allocator{
public:
    using value_type = T;
    // 定义传播特性
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap            = std::true_type;

    // 默认构造函数
    Allocator() = default;

    // 支持 rebind
    template <typename U>
    struct rebind {
        using other = Allocator<U>;
    };

    template<typename U>
    Allocator(const Allocator<U>&) noexcept {}

    //分配内存
    T* allocate(size_t n){
        return static_cast<T*>(operator new(n * sizeof(T)));//利用operator new分配
    }

    T* allocate(size_t n) const {
        return static_cast<T*>(operator new(n * sizeof(T)));//利用operator new分配
    }

    //释放内存
    void deallocate(T* p, size_t n){
        operator delete(p);
    }

    //构造对象
    template <typename... Args>//表示Args为模板参数包，可以包含零个或多个类型参数
    void construct(T* p, Args&&... args){
        new(p) T(forward<Args>(args)...);
    }

    template <typename... Args>
    void construct(T* p, Args&&... args) const {
        new(p) T(forward<Args>(args)...);
    }

    //销毁对象
    void destroy(T* p){
        p->~T();
    }

    // 比较操作
    bool operator==(const Allocator&) const noexcept { return true; }
    bool operator!=(const Allocator&) const noexcept { return false; }
};

#endif // ALLOCATOR_H