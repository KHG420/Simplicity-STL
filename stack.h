#ifndef STACK_H
#define STACK_H

#include "deque.h"

// 定义一个模板类 Stack，用于实现栈的功能
// T 是栈中元素的类型，Sequence 是底层容器的类型，默认为 Deque<T>
template<typename T, typename Sequence = Deque<T> >
class Stack{

    // 友元函数，用于比较两个栈是否相等
    template<typename _Tp1, typename _Seq1>
	friend bool
	operator==(const Stack<_Tp1, _Seq1>&, const Stack<_Tp1, _Seq1>&);

    // 友元函数，用于比较两个栈是否相等
    template<typename _Tp1, typename _Seq1>
	friend bool
	operator<(const Stack<_Tp1, _Seq1>&, const Stack<_Tp1, _Seq1>&);

    // SFINAE检查：确保分配器适用于底层容器
    template<typename _Alloc>
	using _Uses = typename enable_if<uses_allocator<Sequence, _Alloc>::value>::type;

    // 静态断言：确保模板参数T与底层容器的元素类型一致
    static_assert(is_same<T, typename Sequence::value_type>::value,
	"value_type must be the same as the underlying container");

public:
    using value_type      = typename Sequence::value_type;// 元素类型
    using reference       = typename Sequence::reference; // 元素引用类型
    using const_reference = typename Sequence::const_reference;// 常量引用类型
    using size_type       = typename Sequence::size_type; // 大小类型
    using container_type  =	Sequence; // 底层容器类型

protected:
    Sequence c;// 底层容器实例

public:

    // 默认构造函数，当 Sequence 是可默认构造的时调用
    template<typename _Seq = Sequence, typename _Requires = typename
	       enable_if<is_default_constructible<_Seq>::value>::type>
	Stack() : c() {}

    // 默认构造函数，当 Sequence 是可默认构造的时调用
    explicit Stack(const Sequence& __c) : c(__c) {}

    // 默认构造函数，当 Sequence 是可默认构造的时调用
    explicit Stack(Sequence&& __c) : c(std::move(__c)) {}

    // 默认构造函数，当 Sequence 是可默认构造的时调用
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	explicit Stack(const _Alloc& __a) : c(__a) {}

    // 带一个底层容器和一个分配器的构造函数，使用底层容器的拷贝构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Stack(const Sequence& __c, const _Alloc& __a) : c(__c, __a) {}

    // 带一个底层容器和一个分配器的移动构造函数，使用底层容器的移动构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Stack(Sequence&& __c, const _Alloc& __a) : c(std::move(__c), __a) {}

    // 带一个栈和一个分配器的拷贝构造函数，使用底层容器的拷贝构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Stack(const Stack& __q, const _Alloc& __a) : c(__q.c, __a) {}

    // 带一个栈和一个分配器的移动构造函数，使用底层容器的移动构造函数和分配器
    template<typename _Alloc, typename _Requires = _Uses<_Alloc>>
	Stack(Stack&& __q, const _Alloc& __a) : c(std::move(__q.c), __a) {}

    // 判断栈是否为空
    bool empty() const{ 
        return c.empty(); 
    }

    // 返回栈中元素个数
    size_type size() const { 
        return c.size(); 
    }

    // 获取栈顶元素引用（非const）
    reference top(){
	    return c.back();
    }

    // 获取栈顶元素常量引用
    const_reference top() const {
	    return c.back();
    }
    
    // 向栈中压入一个元素，调用底层容器的 push_back 函数
    void push(const value_type& __x) { 
        c.push_back(__x); 
    }

    void push(value_type&& __x) { 
        c.push_back(std::move(__x)); 
    }

    // 向栈中压入一个元素，调用底层容器的 push_back 函数
    template<typename... _Args>
	void emplace(_Args&&... __args){ 
        return c.emplace_back(std::forward<_Args>(__args)...); 
    }

    // 弹出栈顶元素
    void pop(){
	    c.pop_back();
    }

    // 交换两个栈的内容
    void swap(Stack& __s) noexcept (__is_nothrow_swappable<Sequence>::value) {
	    std::swap(c, __s.c);
    }
};

// 类模板参数推导指南，用于根据容器类型推导 Stack 的模板参数
template<typename _Container, typename = _RequireNotAllocator<_Container>>
Stack(_Container) -> Stack<typename _Container::value_type, _Container>;

// 类模板参数推导指南，用于根据容器类型推导 Stack 的模板参数
template<typename _Container, typename _Allocator, typename = _RequireNotAllocator<_Container>>
Stack(_Container, _Allocator) -> Stack<typename _Container::value_type, _Container>;

// ------------------ 非成员函数 ------------------
/// 比较两个栈（通过底层容器比较）
template<typename _Tp, typename _Seq>
inline bool operator==(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return __x.c == __y.c;
}

template<typename _Tp, typename _Seq>
inline bool operator<(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return __x.c < __y.c; 
}

template<typename _Tp, typename _Seq>
inline bool operator!=(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return !(__x == __y); 
}

template<typename _Tp, typename _Seq>
inline bool operator>(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return __y < __x; 
}

template<typename _Tp, typename _Seq>
inline bool operator<=(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return !(__y < __x); 
}

template<typename _Tp, typename _Seq>
inline bool operator>=(const Stack<_Tp, _Seq>& __x, const Stack<_Tp, _Seq>& __y){ 
    return !(__x < __y); 
}

template<typename _Tp, typename _Seq>
inline typename enable_if<__is_swappable<_Seq>::value>::type
swap(stack<_Tp, _Seq>& __x, stack<_Tp, _Seq>& __y) noexcept( noexcept(__x.swap(__y))) { 
    __x.swap(__y); 
}

#endif //STACK_H