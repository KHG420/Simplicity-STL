#ifndef QUEUE_H
#define QUEUE_H

#include "deque.h"
#include "allocator.h"
#include "vector.h"
#include <functional> // 用于默认比较器 std::less
#include <algorithm>

// 定义一个模板类 Queue，用于实现队列的功能
// T 是队列中元素的类型，Sequence 是底层容器的类型，默认为 Deque<T>
template<typename T, typename Sequence = Deque<T> >
class Queue{
private:
    Sequence c;//queue内置deque，方法也大多调用deque里的

    // 用于检查 Sequence 是否可以使用分配器 _Alloc 的类型别名
    template<typename _Alloc>
	using _Uses = typename enable_if<uses_allocator<Sequence, _Alloc>::value>::type;

public:
    using value_type      = typename Sequence::value_type;     //元素的类型
    using reference       = typename Sequence::reference;      //引用类型
    using const_reference = typename Sequence::const_reference;//常量引用类型
    using size_type       = typename Sequence::size_type;      //大小类型
    using container_type  = Sequence;                          // 底层容器的类型

    // 默认构造函数，当 Sequence 是可默认构造的时调用
    template<typename Seq = Sequence, typename _Requires = typename enable_if<is_default_constructible<Seq>::value>::type>
	Queue() : c() {}

    // 带一个底层容器的构造函数，使用底层容器的拷贝构造函数
    explicit Queue(const Sequence& __c) : c(__c) {}

    // 带一个底层容器的移动构造函数，使用底层容器的移动构造函数
    explicit Queue(Sequence&& __c) : c(std::move(__c)) {}

    // 带一个分配器的构造函数，当 Sequence 可以使用该分配器时调用
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>> explicit
	Queue(const _Alloc& __a) : c(__a) {}

    // 带一个底层容器和一个分配器的构造函数，使用底层容器的拷贝构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Queue(const Sequence& __c, const _Alloc& __a) : c(__c, __a) {}

    // 带一个底层容器和一个分配器的移动构造函数，使用底层容器的移动构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Queue(Sequence&& __c, const _Alloc& __a) : c(std::move(__c), __a) {}

    // 带一个队列和一个分配器的拷贝构造函数，使用底层容器的拷贝构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Queue(const Queue& __q, const _Alloc& __a = 0) : c(__q.c, __a) {}

    // 带一个队列和一个分配器的移动构造函数，使用底层容器的移动构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Queue(Queue&& __q, const _Alloc& __a = 0) : c(std::move(__q.c), __a) {}

    bool empty() const { 
        return c.empty(); 
    }

    size_type size() const { 
        return c.size();
    }

    reference front(){
	    return c.front();
    }

    const_reference front() const {
	    return c.front();
    }

    reference back() {
	    return c.back();
    }

    const_reference back() const {
	    return c.back();
    }

    void push(const value_type& __x) { 
        c.push_back(__x); 
    }

    void push(value_type&& __x) { 
        c.push_back(std::move(__x)); 
    }

    template<typename... _Args>
	decltype(auto) emplace(_Args&&... __args) { 
        return c.emplace_back(std::forward<_Args>(__args)...); 
    }

    void pop() {
	    c.pop_front();
    }

    template<typename _Tp, typename _Seq>
    bool operator==(const Queue<_Tp, _Seq>& __y){ 
        return c == __y.c; 
    }

    template<typename _Tp, typename _Seq>
    bool operator<(const Queue<_Tp, _Seq>& __y) { 
        return c < __y.c; 
    }

    template<typename _Tp, typename _Seq>
    bool operator!=(const Queue<_Tp, _Seq>& __y) { 
        return !(*this == __y); 
    }

    template<typename _Tp, typename _Seq>
    bool operator>(const Queue<_Tp, _Seq>& __y) { 
        return __y < *this; 
    }

    template<typename _Tp, typename _Seq>
    bool operator<=(const Queue<_Tp, _Seq>& __y) { 
        return !(__y < *this); 
    }


    template<typename _Tp, typename _Seq>
    bool operator>=(const Queue<_Tp, _Seq>& __y) { 
        return !(*this < __y); 
    }

    void swap(Queue& __q){
	    c.swap(__q.c);
    }

};

template<typename Type, typename Allocator>
void swap(Queue<Type, Allocator>& left, Queue<Type, Allocator>& right) noexcept {
    left.swap(right);
}


//优先队列
template<typename T, typename Container = vector<T>, typename Compare = std::less<typename Container::value_type>>
class Priority_Queue {
private:
    Container c;       // 底层容器
    Compare comp;      // 比较器

    // 上浮操作：将新插入的元素调整到正确位置
    void sift_up(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (comp(c[parent], c[index])) { // 如果父节点小于当前节点
                std::swap(c[parent], c[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    // 下沉操作：删除堆顶后调整堆结构
    void sift_down(size_t index) {
        size_t size = c.size();
        while (index < size) {
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;
            size_t largest = index;

            if (left < size && comp(c[largest], c[left])) {
                largest = left;
            }
            if (right < size && comp(c[largest], c[right])) {
                largest = right;
            }
            if (largest != index) {
                std::swap(c[index], c[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

    template<typename _Alloc>
	using _Uses = typename enable_if<uses_allocator<Container, _Alloc>::value>::type;
    
public:

    using value_type      = typename Container::value_type;
    using reference       = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using size_type       = typename Container::size_type;
    using container_type  = Container;
    using value_compare   = Compare;

    // 默认构造函数
    Priority_Queue() = default;

    // 默认构造函数（需要显式支持默认构造）
    template<typename Cont = Container, typename = typename std::enable_if<std::is_default_constructible<Cont>::value>::type>
    Priority_Queue() : c(), comp() {}

    // 带比较器和容器的构造函数
    explicit Priority_Queue(const Compare& cmp, const Container& cont = Container()) : c(cont), comp(cmp) { 
        std::make_heap(c.begin(), c.end(), comp); 
    }

    // 移动语义容器构造函数
    explicit Priority_Queue(const Compare& cmp, Container&& cont) : c(std::move(cont)), comp(cmp) { 
        std::make_heap(c.begin(), c.end(), comp); 
    }

    // 迭代器范围构造函数
    template<typename InputIterator, typename = typename std::enable_if<std::is_convertible<
             typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value>::type>
    Priority_Queue(InputIterator first, InputIterator last, const Compare& cmp = Compare()) : comp(cmp) {
        c.insert(c.end(), first, last);
        std::make_heap(c.begin(), c.end(), comp);
    }

    // 带分配器的构造函数组
    template<typename Alloc, typename = _Uses<Alloc>>
    explicit Priority_Queue(const Alloc& alloc) : c(alloc), comp() {}

    template<typename Alloc, typename = _Uses<Alloc>>
    Priority_Queue(const Compare& cmp, const Alloc& alloc) : c(alloc), comp(cmp) {}

    template<typename Alloc, typename = _Uses<Alloc>>
    Priority_Queue(const Compare& cmp, const Container& cont, const Alloc& alloc) : c(cont, alloc), comp(cmp) { 
        std::make_heap(c.begin(), c.end(), comp); 
    }

    template<typename Alloc, typename = _Uses<Alloc>>
    Priority_Queue(const Compare& cmp, Container&& cont, const Alloc& alloc) : c(std::move(cont), alloc), comp(cmp) { 
        std::make_heap(c.begin(), c.end(), comp);
    }

    // 拷贝/移动 + 分配器构造函数
    template<typename Alloc, typename = _Uses<Alloc>>
    Priority_Queue(const Priority_Queue& other, const Alloc& alloc) : c(other.c, alloc), comp(other.comp) {}

    template<typename Alloc, typename = _Uses<Alloc>>
    Priority_Queue(Priority_Queue&& other, const Alloc& alloc) : c(std::move(other.c), alloc), comp(std::move(other.comp)) {}

    template<typename InputIterator, typename Alloc, typename = typename std::enable_if<std::is_convertible<
             typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>::value>,
             typename = _Uses<Alloc>>
    Priority_Queue(InputIterator first, InputIterator last, const Compare& cmp, Container&& cont, const Alloc& alloc)
        : c(std::move(cont), alloc), comp(cmp) {
        c.insert(c.end(), first, last);
        std::make_heap(c.begin(), c.end(), comp);
    }

    // 访问堆顶元素（最大元素）
    const T& top() const {
        return c.front();
    }

    // 判断是否为空
    bool empty() const {
        return c.empty();
    }

    // 返回元素个数
    size_t size() const {
        return c.size();
    }

    // 插入元素并调整堆
    void push(const T& value) {
        c.push_back(value);
        sift_up(c.size() - 1);
    }

    // 移动语义插入
    void push(T&& value) {
        c.push_back(std::move(value));
        sift_up(c.size() - 1);
    }

    // 就地构造元素
    template<typename... Args>
    void emplace(Args&&... args) {
        c.emplace_back(std::forward<Args>(args)...);
        sift_up(c.size() - 1);
    }

    // 删除堆顶元素
    void pop() {
        if (empty()) return;
        std::swap(c.front(), c.back());
        c.pop_back();
        sift_down(0);
    }

    // 交换两个优先队列
    void swap(Priority_Queue& other) noexcept {
        std::swap(c, other.c);
        std::swap(comp, other.comp);
    }

    // 友元函数，支持非成员swap
    friend void swap(Priority_Queue& lhs, Priority_Queue& rhs) noexcept {
        lhs.swap(rhs);
    }
};

#endif //QUEUE_H