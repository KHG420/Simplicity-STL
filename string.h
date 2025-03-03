#ifndef DEQUE_H
#define DEQUE_H

#include "allocator.h"
#include "vector.h"
#include <algorithm>

template <typename T, typename alloc = Allocator<T>>
class Deque;

// 迭代器类
template <typename T, bool IsConst>
class DequeIterator{
private:
    T **block; // 指向当前元素所在块的指针
    T *cur;    // 当前元素指针
    T *last;   // 后边界指针
    T *first;  // 前边界指针
    size_t block_size; //缓冲区大小

    // 跳转缓冲区
    void set_buf(T **new_block){
        block = new_block;
        first = *block;
        last = first + this->deque_buf_size();
    }

    size_t deque_buf_size() const{
        return block_size < sizeof(T) ? 1 : block_size / sizeof(T);
    }

public:

    friend class Deque<T>;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<IsConst, const T *, T *>;
    using reference = std::conditional_t<IsConst, const T &, T &>;

    // 构造函数，用于初始化迭代器
    DequeIterator(T** b, T* x, size_t block_size) : block(b), cur(x), block_size(block_size){
        first = *block;
        last = first + this->deque_buf_size();
    }

    DequeIterator() {}

    // operator+ 重载
    DequeIterator operator+(difference_type n) const{
        DequeIterator tmp = *this;
        return tmp += n;
    }

    // operator+= 重载
    DequeIterator &operator+=(difference_type n){
        difference_type offset = n + (cur - first);
        // 在同一块内
        if (offset >= 0 && offset < difference_type(this->deque_buf_size())){
            cur += n;
        }
        else{
            difference_type block_offset = offset > 0 ? offset / difference_type(this->deque_buf_size()) : -difference_type((-offset - 1) / this->deque_buf_size());
            // 切换至正确块
            set_buf(block + block_offset);
            // 切换至正确偏移量
            cur = first + (offset - block_offset * difference_type(this->deque_buf_size()));
        }
        return *this;
    }

    // operator- 重载（迭代器减整数）
    DequeIterator operator-(difference_type n) const{
        return *this + (-n);
    }

    // operator-= 重载
    DequeIterator &operator-=(difference_type n){
        return *this += (-n);
    }

    // operator- 重载（迭代器间距离）
    difference_type operator-(const DequeIterator &other) const{
        return (block - other.block - 1) * this->deque_buf_size() + (cur - first) + (other.last - other.cur);
    }
    
    // 前置自增运算符重载
    DequeIterator &operator++(){
        ++cur;
        if (cur == last){
            set_buf(block + 1);
            cur = first;
        }
        return *this;
    }

    // 后置自增运算符重载
    DequeIterator operator++(int){
        DequeIterator temp = *this;
        ++(*this);
        return temp;
    }

    // 前置自减运算符重载
    DequeIterator &operator--(){
        if (cur == first){
            set_buf(block - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    // 后置自减运算符重载
    DequeIterator operator--(int){
        DequeIterator temp = *this;
        --(*this);
        return temp;
    }

    // 解引用运算符重载，返回当前元素的引用
    reference operator*(){
        return *cur;
    }

    // 箭头运算符重载，用于访问当前元素的成员
    pointer operator->(){
        return cur;
    }

    // 相等运算符重载，判断两个迭代器是否相等
    bool operator==(const DequeIterator &other) const{
        return other.cur == cur;
    }

    // 不等运算符重载，判断两个迭代器是否不等
    bool operator!=(const DequeIterator &other) const{
        return !(*this == other);
    }

};

template <typename T, typename alloc>
class Deque{
private:
    // 缓冲区容纳元素个数， 元素大小超出缓冲区容量时为1
    size_t deque_buf_size(){
        return block_size < sizeof(T) ? 1 : block_size / sizeof(T);
    }
    
    DequeIterator<T, false> head;
    DequeIterator<T, false> tail;
    size_t block_size = 512; // 元素块大小
    Vector<T*> blocks_;     // 块指针数组
    alloc allocator_;

public:
    friend class DequeIterator<T, true>;
    friend class DequeIterator<T, false>;
    // 迭代器类型定义
    using iterator = DequeIterator<T, false>;
    using const_iterator = DequeIterator<T, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    typedef alloc allocator_type;
    typedef const T *const_pointer;
    typedef const T &const_reference;
    typedef T *pointer;
    typedef T &reference;
    typedef std::ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef T value_type;

    // 默认构造函数
    Deque(){
        initialize();
    }

    // 带分配器的构造函数
    explicit Deque(const alloc &Al) : allocator_(Al){
        initialize();
    }

    // 带元素数量的构造函数
    explicit Deque(size_type Count){
        initialize();
        for (size_type i = 0; i < Count; ++i)
        {
            push_back(T());
        }
    }

    // 带元素数量和初始值的构造函数
    Deque(size_type Count, const T &Val){
        initialize();
        for (size_type i = 0; i < Count; ++i)
        {
            push_back(Val);
        }
    }

    // 带元素数量、初始值和分配器的构造函数
    Deque(size_type Count, const T &Val, const alloc &Al) : allocator_(Al){
        initialize();
        for (size_type i = 0; i < Count; ++i)
        {
            push_back(Val);
        }
    }

    // 拷贝构造函数
    Deque(Deque &Right) : allocator_(Right.allocator_){
        initialize();
        for (const auto &elem : Right)
        {
            push_back(elem);
        }
    }

    // 迭代器范围构造函数
    template <class InputIterator,
              typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag,
                                                            typename std::iterator_traits<InputIterator>::iterator_category>>>
    Deque(InputIterator First, InputIterator Last){
        initialize();
        for (InputIterator it = First; it != Last; ++it)
        {
            push_back(*it);
        }
    }

    // 迭代器范围和分配器构造函数
    template <class InputIterator,
              typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag,
                                                            typename std::iterator_traits<InputIterator>::iterator_category>>>
    Deque(InputIterator First, InputIterator Last, const alloc &Al) : allocator_(Al){
        initialize();
        for (InputIterator it = First; it != Last; ++it)
        {
            push_back(*it);
        }
    }

    // 初始化列表和分配器构造函数
    Deque(std::initializer_list<T> IList, const alloc &Al) : allocator_(Al){
        initialize();
        for (const auto &elem : IList)
        {
            push_back(elem);
        }
    }

    Deque(std::initializer_list<T> IList){
        initialize();
        for (const auto &elem : IList)
        {
            push_back(elem);
        }
    }

    // 析构函数
    ~Deque(){
        clear();
    }

    // 清空容器
    void clear(){
        
    }

    // 插入元素到头部
    void push_front(const T &value){
        //前面没有块时扩充
        if (head.cur == head.first ){
            expand_blocks();
            *(head.block - 1) = allocator_.allocate(deque_buf_size());
            head.set_buf(head.block - 1);
            head.cur = head.last;
        }
        --head;
        allocator_.construct(head.cur, value);
    }

    // 插入元素到尾部
    void push_back(const T &value){
        if (tail.cur == tail.last){
            //内存块不足时扩充
            expand_blocks();
            *(tail.block + 1) = allocator_.allocate(deque_buf_size());
            tail.set_buf(tail.block + 1);
            tail.cur = tail.first;
        }
        allocator_.construct(tail.cur++, value);
    }

    // 容量操作
    size_t size() const { return tail - head; }
    bool empty() const { return tail == head; }

    // 迭代器
    iterator begin() noexcept { return head; }
    const_iterator cbegin() noexcept { return const_iterator(head); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(tail); }
    const_reverse_iterator crbegin() noexcept { return const_reverse_iterator(tail); }
    iterator end() noexcept { return tail; }
    const_iterator cend() noexcept { return const_iterator(tail); }
    reverse_iterator rend() noexcept { return reverse_iterator(head); }
    const_reverse_iterator crend() noexcept { return const_reverse_iterator(head); }

private:
    // 初始化
    void initialize(){
        blocks_.resize(2);
        size_t buf_size = deque_buf_size();
        blocks_[0] = allocator_.allocate(buf_size);
        blocks_[1] = allocator_.allocate(buf_size);
        head = iterator(&blocks_[1], blocks_[1], block_size);
        tail = iterator(&blocks_[1], blocks_[1], block_size);
    }

    // 扩充blocks_数组
    void expand_blocks(){
        //记录头尾在各自缓冲区的偏移量
        size_t head_buf_offset = head.block - &blocks_[0];
        size_t head_offset = head.cur - head.first;
        size_t tail_buf_offset = tail.block - &blocks_[0];
        size_t tail_offset = tail.cur - tail.first;
        blocks_.resize(blocks_.size() * 2);
        // 原元素移动到中间
        for (long long i = blocks_.size() / 2; i >= 0; --i){
            blocks_[i + blocks_.size() / 4] = blocks_[i];
        }
        //其他设置为nullptr
        // for(size_t i = 0; i < blocks_.size() / 4; i++){
        //     blocks_[i] = nullptr;
        //     blocks_[blocks_.size() - 1 - i] = nullptr;
        // }
        //更新头尾
        head.block = &blocks_[head_buf_offset + blocks_.size() / 4];
        tail.block = &blocks_[tail_buf_offset + blocks_.size() / 4];
    }

};

#endif // DEQUE_H
