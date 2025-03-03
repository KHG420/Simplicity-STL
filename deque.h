#ifndef DEQUE_H
#define DEQUE_H

#include "allocator.h"
#include "vector.h"
#include <cstring>
#include <algorithm>

template <typename T, typename Alloc = Allocator<T>>
class Deque;

// 迭代器类
template <typename T, bool IsConst>
class DequeIterator {
private:
    using ElementPtr  = std::conditional_t<IsConst, const T*, T*>;  // 根据IsConst的类型判断指针类型
    using BlockPtrPtr = std::conditional_t<IsConst, const T* const*, T**>;
    BlockPtrPtr block;      // 指向当前元素所在块的指针的指针（可能是const的）
    ElementPtr  cur;        // 当前元素指针（可能是const的）
    ElementPtr  last;       // 后边界指针（可能是const的）
    ElementPtr  first;      // 前边界指针（可能是const的）
    size_t      block_size; // 缓冲区大小

    // 跳转缓冲区
    void set_buf(DequeIterator<T, IsConst>::BlockPtrPtr new_block) {
        block = new_block;
        first = *block;
        last = first + this->deque_buf_size();
    }

    size_t deque_buf_size() const {
        return block_size < sizeof(T) ? 1 : block_size / sizeof(T);
    }

    // 声明所有 DequeIterator 实例为友元类
    template <typename U, bool OtherConst>
    friend class DequeIterator;

public:
    friend class Deque<T>;

    // 迭代器标签，这里需要使用标准库定义的迭代器标签
    using iterator_category = std::random_access_iterator_tag;  // 修改为随机访问迭代器标签
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = std::conditional_t<IsConst, const T*, T*>;
    using reference         = std::conditional_t<IsConst, const T&, T&>;

    // 构造函数，用于初始化迭代器
    DequeIterator(BlockPtrPtr b, ElementPtr x, size_t block_size) : block(b), cur(x), block_size(block_size) {
        first = *block;
        last = first + this->deque_buf_size();
    }

    // const转换构造函数
    // const转换构造函数
    template<bool OtherConst, typename = std::enable_if_t<IsConst || !OtherConst>>
    DequeIterator(const DequeIterator<T, OtherConst>& other)
        : block(other.block), 
        cur(other.cur),
        first(other.first),
        last(other.last),
        block_size(other.block_size) {}

    DequeIterator() {}

    ~DequeIterator() {}

    // operator+ 重载
    DequeIterator operator+(difference_type n) const {
        DequeIterator tmp = *this;
        return tmp += n;
    }

    // operator+= 重载
    DequeIterator& operator+=(difference_type n) {
        difference_type offset = n + (cur - first);
        // 在同一块内
        if (offset >= 0 && offset < difference_type(this->deque_buf_size())) {
            cur += n;
        }
        else {
            difference_type block_offset = offset > 0 ? offset / difference_type(this->deque_buf_size()) : -difference_type((-offset - 1) / this->deque_buf_size());
            // 切换至正确块
            set_buf(block + block_offset);
            // 切换至正确偏移量
            cur = first + (offset - block_offset * difference_type(this->deque_buf_size()));
        }
        return *this;
    }

    // operator- 重载（迭代器减整数）
    DequeIterator operator-(difference_type n) const {
        return *this + (-n);
    }

    // operator-= 重载
    DequeIterator& operator-=(difference_type n) {
        return *this += (-n);
    }

    // operator- 重载（迭代器间距离）
    difference_type operator-(const DequeIterator& other) const {
        return (block - other.block - 1) * this->deque_buf_size() + (cur - first) + (other.last - other.cur);
    }

    // 前置自增运算符重载
    DequeIterator& operator++() {
        ++cur;
        if (cur == last && *(block + 1) != nullptr) {
            set_buf(block + 1);
            cur = first;
        }
        return *this;
    }

    // 后置自增运算符重载
    DequeIterator operator++(int) {
        DequeIterator temp = *this;
        ++(*this);
        return temp;
    }

    // 前置自减运算符重载
    DequeIterator& operator--() {
        if (cur == first && *(block - 1) != nullptr) {
            set_buf(block - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    // 后置自减运算符重载
    DequeIterator operator--(int) {
        DequeIterator temp = *this;
        --(*this);
        return temp;
    }

    // 解引用运算符重载，返回当前元素的引用
    reference operator*() const {
        return *cur;
    }

    // 箭头运算符重载，用于访问当前元素的成员
    pointer operator->() const {
        return cur;
    }

    // 相等运算符重载，判断两个迭代器是否相等
    bool operator==(const DequeIterator& other) const {
        return other.cur == cur;
    }

    // 不等运算符重载，判断两个迭代器是否不等
    bool operator!=(const DequeIterator& other) const {
        return !(*this == other);
    }

    // 小于运算符重载
    bool operator<(const DequeIterator& other) const {
        return block < other.block || (block == other.block && cur < other.cur);
    }

    // 大于运算符重载
    bool operator>(const DequeIterator& other) const {
        return other < *this;
    }

    // 小于等于运算符重载
    bool operator<=(const DequeIterator& other) const {
        return !(other < *this);
    }

    // 大于等于运算符重载
    bool operator>=(const DequeIterator& other) const {
        return !(*this < other);
    }

    // 下标运算符重载，支持随机访问
    reference operator[](difference_type n) const {
        return *(*this + n);
    }
};


template <typename T, typename Alloc>
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
    Alloc allocator_;

public:
    friend class DequeIterator<T, true>;
    friend class DequeIterator<T, false>;

    // 迭代器类型定义
    using iterator               = DequeIterator<T, false>;
    using const_iterator         = DequeIterator<T, true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator_type         = Alloc;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using difference_type        = std::ptrdiff_t;
    using value_type             = T;
    using size_type              = size_t;

    // 默认构造函数
    Deque(){
        initialize();
    }

    // 带分配器的构造函数
    explicit Deque(const Alloc &Al) : allocator_(Al){
        initialize();
    }

    // 带元素数量的构造函数
    explicit Deque(size_type Count){
        initialize_count_val(Count, value_type());
    }
                                                                          
    // 带元素数量和初始值的构造函数
    explicit Deque(size_type Count, const T &Val){
        initialize_count_val(Count, Val);
    }

    // 带元素数量、初始值和分配器的构造函数
    Deque(size_type Count, const T &Val, const Alloc &Al) : allocator_(Al){
        initialize_count_val(Count, Val);
    }

    // 拷贝构造函数
    Deque(const Deque &Right) : allocator_(Right.allocator_), block_size(Right.block_size) {
    size_type buf_size = deque_buf_size();
    blocks_.resize(Right.blocks_.size());
    size_t blocks_index = 0;
    for (const T* it : Right.blocks_)   {  // 使用 const 引用 
        if (it != nullptr) {
            blocks_[blocks_index] = allocator_.allocate(buf_size);
            // 使用 memcpy（注意：可能不适用于非平凡类型，建议改用元素拷贝构造）
            memcpy(blocks_[blocks_index], it, buf_size * sizeof(T));
        } else {
            blocks_[blocks_index] = nullptr;
        }
        ++blocks_index;
    }

    // 计算原块索引 
    T* const* right_blocks_start = &Right.blocks_[0]; 
    size_t head_block_idx = Right.head.block - right_blocks_start;
    size_t tail_block_idx = Right.tail.block - right_blocks_start;

    // 初始化 head 和 tail 迭代器 
    head = iterator(
        &blocks_[head_block_idx],
        blocks_[head_block_idx] + (Right.head.cur - Right.head.first),
        block_size 
        );
    tail = iterator(
        &blocks_[tail_block_idx],
        blocks_[tail_block_idx] + (Right.tail.cur - Right.tail.first),
        block_size  
        );
    }

    // 拷贝赋值运算符 
    Deque& operator=(const Deque& other) {
        if (this != &other) {
            if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::true_type) {
                allocator_ = other.allocator_;  
            }
            assign(other.begin(),  other.end()); 
        }   
        return *this;
    }
 
    // 移动赋值运算符 
    Deque& operator=(Deque&& other) noexcept {
        if (this != &other) {
            clear();
            for (auto& block : blocks_) {
                if (block) {
                    allocator_.deallocate(block, block_size);
                    block = nullptr;
                }
            }
            blocks_.clear();
 
            if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value) {
                allocator_ = std::move(other.allocator_); 
            }
 
            blocks_ = std::move(other.blocks_); 
            head = other.head;  
            tail = other.tail; 
            block_size = other.block_size;
 
            other.blocks_.clear(); 
            other.initialize(); 
        }
        return *this;
    }
 
    // 初始化列表赋值运算符 
    Deque& operator=(std::initializer_list<T> ilist) {
        assign(ilist.begin(),  ilist.end()); 
        return *this;
    }

    // 迭代器范围构造函数
    template <class InputIterator,
            typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag,
            typename std::iterator_traits<InputIterator>::iterator_category>>>
    Deque(InputIterator First, InputIterator Last) : Deque(First, Last, Allocator<T>()) {}

    // 迭代器范围和分配器构造函数
    template <class InputIterator,
            typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag,
            typename std::iterator_traits<InputIterator>::iterator_category>>>
    Deque(InputIterator First, InputIterator Last, const Alloc &Al) : allocator_(Al){
        initialize();
        for (InputIterator it = First; it != Last; ++it)
        {
            push_back(*it);
        }
    }

    // 初始化列表和分配器构造函数
    Deque(std::initializer_list<T> IList, const Alloc &Al) : allocator_(Al){
        initialize();
        for (const auto &elem : IList){
            push_back(elem);
        }
    }

    Deque(std::initializer_list<T> IList){
        initialize();
        for (const auto &elem : IList){
            push_back(elem);
        }
    }

    // 析构函数
    ~Deque(){
        clear();
        for(auto it = blocks_.begin(); it != blocks_.end(); ++it){
            if(*it != nullptr){
                allocator_.deallocate(*it, block_size);
            }
        }
    }

    // 清空容器
    void clear(){
        while(head != tail){
            allocator_.destroy(head.cur);
            head++;
        }
        head = tail = iterator();
    }

    reference back(){
        return *(tail - 1);
    }

    const_reference back() const {
        return *(tail - 1);
    }

    reference front(){
        return *head;
    }

    const_reference front() const {
        return *head;
    }

    // 插入元素到头部
    void push_front(const T &value){
        emplace_front(value);
    }

    // 移动语义的 push_front 函数
    void push_front(T&& value){
        emplace_front(std::move(value));
    }

    //删除 deque 开头的元素
    void pop_front(){
        if(head == tail) return;

        allocator_.destroy(head.cur);
        //元素为最后一块时
        if(head.cur == head.last){ 
            auto temp = head.block + 1;
            allocator_.deallocate(*head.block, block_size); //回收内存
            *head.block = nullptr;
            head.block = temp;   //head重新赋值
            head.cur = head.first = *(tail.block);
            head.last = head.first + deque_buf_size();
        }
        else{
            ++head;
        }

    }

    // 插入元素到尾部
    void push_back(const T& value) {
        emplace_back(value);
    }

    // 移动语义的 push_back 函数
    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    //删除 deque 末尾处的元素
    void pop_back(){
        if(head == tail) return;
        //元素为最后一块时
        if(tail.cur == tail.first){
            auto temp = tail.block - 1;
            allocator_.deallocate(*tail.block, block_size);
            *tail.block = nullptr;
            tail.block = temp;   // tail重新赋值
            tail.cur = tail.last = *(tail.block) + deque_buf_size();
            tail.first = *(tail.block);
        }
        --tail;
        allocator_.destroy(tail.cur);
    }

    // 容量操作
    size_t size() const { 
        return tail - head; 
    }
    
    bool empty() const { 
        return tail == head;
    }

    //元素访问
    // 非const版本下标运算符重载
    reference operator[](size_type index) {
        if (index >= size()) {
            throw std::out_of_range("Deque::operator[] index out of range");
        }
        return *(head + index);
    }

    // const版本下标运算符重载
    const_reference operator[](size_type index) const {
        if (index >= size()) {
            throw std::out_of_range("Deque::operator[] index out of range");
        }
        return *(head + index);
    }

    reference at(size_type index){
        if(index >= tail - head){
            throw std::out_of_range("Deque : at");
        }
        return *(head + index);
    }

    const_reference at (size_type index) const {
        if(index >= tail - head){
            throw std::out_of_range("Deque : at");
        }
        return *(head + index);
    }

    // 迭代器
    iterator begin() noexcept { 
        return head; 
    }
    const_iterator cbegin() const noexcept { 
        return const_iterator(head.block, head.cur, block_size); 
    }
    reverse_iterator rbegin() noexcept { 
        return reverse_iterator(end()); 
    }
    const_reverse_iterator crbegin() const noexcept { 
        return const_reverse_iterator(cend()); 
    }
    iterator end() noexcept { 
        return tail; 
    }
    const_iterator cend() const noexcept { 
        return const_iterator(tail.block, tail.cur, block_size); 
    }
    reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }
    const_reverse_iterator crend() const noexcept { 
        return const_reverse_iterator(cbegin()); 
    }

    //将元素从 deque 中清除并将一组新的元素序列复制到目标 deque
    template <class InputIterator, typename = std::enable_if_t<!std::is_integral<InputIterator>::value>>
    void assign(InputIterator First, InputIterator Last) {
        clear();

        using IteratorCategory = typename std::iterator_traits<InputIterator>::iterator_category;
        // 为随机访问迭代器时，预先计算所需的块数，分配内存，然后批量复制元素，提高效率
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag, IteratorCategory>) {
            
            size_type n = Last - First;
            if (n == 0) return;

            size_type buf_size = deque_buf_size();
            size_type required_blocks = (n + buf_size - 1) / buf_size;
            size_type num_blocks = required_blocks + 2; // 前后哨兵块

            blocks_.resize(num_blocks, nullptr);
            for (size_t i = 0; i < num_blocks; ++i) {
                blocks_[i] = allocator_.allocate(buf_size);
            }

            // 设置头尾到中间块
            size_type mid_block = num_blocks / 2;
            size_type head_block_idx = mid_block - (required_blocks / 2);
            head = iterator(&blocks_[head_block_idx], blocks_[head_block_idx], block_size);
            tail = head;

            // 批量复制元素
            size_type remaining = n;
            while (remaining > 0) {
                size_type to_copy = std::min(static_cast<size_type>(tail.last - tail.cur), remaining);
                std::uninitialized_copy_n(First, to_copy, tail.cur);
                First += to_copy;
                tail += to_copy;
                remaining -= to_copy;
            }
        } else {
            // 非随机访问迭代器，笨蛋式填充
            while(First != Last){
                push_back(*First++);
            }
        }
    }

    void assign(size_type Count, const value_type& Val){
        clear();
        initialize_count_val(Count, Val);
    }

    void assign(initializer_list<value_type> IList){
        assign(IList.begin(), IList.end());
    }

    //将就地构造的元素插入到指定位置的 deque 中
    template <typename... Args>
    void emplace_front(Args&&... args) {
        // 检查是否需要扩展头部块
        if (head.cur == head.first) {
            size_t old_cap = blocks_.size();
            try{
                //缓冲区不足时扩充
                if(head.block == &blocks_[0]){
                    expand_blocks();
                    *(head.block - 1) = allocator_.allocate(deque_buf_size());
                    head.set_buf(head.block - 1);
                    head.cur = head.last;
                }
                //跳转至前一个缓冲区
                else if(*(head.block - 1) == nullptr){
                    *(head.block - 1) = allocator_.allocate(deque_buf_size());
                    head.set_buf(head.block - 1);
                    head.cur = head.last;
                }
            }catch(...){
                blocks_.resize(old_cap); // 回滚容量
                throw;
            }
        }
        --head; // 移动指针到新位置
        allocator_.construct(head.cur, std::forward<Args>(args)...); // 直接构造元素
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        // 检查是否需要扩展尾部块
        if (tail.cur == tail.last) {
            size_t old_cap = blocks_.size();
            try {
                if (tail.block == &blocks_.back()) {
                    expand_blocks();
                    *(tail.block + 1) = allocator_.allocate(deque_buf_size());
                    tail.set_buf(tail.block + 1);
                    tail.cur = tail.first;
                } else if (*(tail.block + 1) == nullptr) {
                    *(tail.block + 1) = allocator_.allocate(deque_buf_size());
                    tail.set_buf(tail.block + 1);
                    tail.cur = tail.first;
                }
            } catch (...) {
                blocks_.resize(old_cap);
                throw;
            }
        }
        allocator_.construct(tail.cur, std::forward<Args>(args)...); // 直接构造元素
        ++tail; // 移动尾指针到下一个位置
    }

    template <typename... Args>
    iterator emplace(const_iterator _Where, Args&&... args) {
        // 将const_iterator转换为普通iterator
        iterator where = head + (_Where - cbegin());
        if (where == tail) {    
            // 插入到末尾
            emplace_back(std::forward<Args>(args)...);
            return tail - 1;
        } else if (where == head) {
            // 插入到开头
            emplace_front(std::forward<Args>(args)...);
            return head;
        }

        // 中间插入，需要移动元素
        size_type index = where - head;
        if (index < size() / 2) {
            // 前半部分，移动前面的元素
            emplace_front(T());
            std::move(head + 1, where + 1, head);
        } else {
            // 后半部分，移动后面的元素
            emplace_back(T());
            std::move_backward(where, tail - 1, tail);
        }

        // 在腾出的位置构造元素
        allocator_.destroy(&*where);
        allocator_.construct(&*where, std::forward<Args>(args)...);
        return where;
    }

    //从指定位置删除 deque 中一个或一系列元素
    iterator erase(iterator where) {
        if (where == end()) return where;

        // 销毁目标元素
        allocator_.destroy(&*where);

        // 判断删除位置在前半还是后半
        size_type index = where - head;
        if (index < size() / 2) {
            // 前半部分：将 [head, where) 的元素向后移动一位，覆盖where
            std::move_backward(head, where, where + 1);
            pop_front(); // 调整头部
        } else {
            // 后半部分：将 [where+1, tail) 的元素向前移动一位，覆盖where
            std::move(where + 1, tail, where);
            pop_back(); // 调整尾部
        }
        return where;
    }

    iterator erase(iterator first, iterator last) {
        if (first == last) return first;

        // 计算删除范围长度
        size_type n = last - first;

        // 判断删除范围在前半还是后半
        size_type index = first - head;
        if (index < size() / 2) {
            // 前半部分：将 [head, first) 的元素向后移动n位
            std::move_backward(head, first, last);
            // 销毁并调整头部
            for (auto it = head; it != head + n; ++it)
                allocator_.destroy(&*it);
            head += n;
        } else {
            // 后半部分：将 [last, tail) 的元素向前移动n位
            std::move(last, tail, first);
            // 销毁并调整尾部
            for (auto it = tail - n; it != tail; ++it)
                allocator_.destroy(&*it);
            tail -= n;
        }
        return first;
    }

    //返回用于构造 deque 的分配器对象的一个副本
    allocator_type get_allocator() const {
        return Alloc();
    }

    //将一个、多个或一系列元素插入 deque 中的指定位置
    iterator insert(const_iterator Where, const value_type& Val){
        return emplace(Where, Val);
    }

    iterator insert(const_iterator Where, value_type&& Val){
        return emplace(Where, std::move(Val));
    }

    iterator insert(iterator Where, size_type Count, const value_type& Val) {
        auto offset = Where - begin();
        if (Count == 0) return begin() + offset;

        // 计算插入位置的偏移量
        size_type index = Where - begin();

        // 根据插入位置决定移动方向
        if (index < size() / 2) {
            // 前半部分：扩展头部并移动元素
            for (size_type i = 0; i < Count; ++i)
                emplace_front(Val);
            std::rotate(begin(), begin() + Count, begin() + index + Count);
        } else {
            // 后半部分：扩展尾部并移动元素
            for (size_type i = 0; i < Count; ++i)
                emplace_back(Val);
            std::rotate(begin() + index, end() - Count, end());
        }

        return begin() + offset;
    }

    template <class InputIterator>
    void insert(iterator Where, InputIterator First, InputIterator Last) {
        using IteratorCategory = typename std::iterator_traits<InputIterator>::iterator_category;

        if constexpr (std::is_base_of_v<std::random_access_iterator_tag, IteratorCategory>) {
            // 随机访问迭代器：批量处理
            size_type n = Last - First;
            if (n == 0) return;

            size_type index = Where - begin();
            if (index < size() / 2) {
                // 前半部分插入，逐个插入最前端后翻转元素，下同
                for (size_type i = 0; i < n; ++i)
                    emplace_front(*(Last-- - 1));
                std::rotate(begin(), begin() + n, begin() + index + n);
            } else {
                // 后半部分插入
                for (size_type i = 0; i < n; ++i)
                    emplace_back(*(First++));
                std::rotate(begin() + index, end() - n, end());
            }
        } else {
            // 输入迭代器：逐个插入
            Vector<T> temp(First, Last); // 缓存元素
            for (auto rit = temp.rbegin(); rit != temp.rend(); ++rit)
                insert(Where, *rit);
        }
    }

    iterator insert(iterator Where, initializer_list<value_type>IList){
        auto offset = Where - begin();
        insert(Where, IList.begin(), IList.end());
        return begin() + offset;
    }

    //为 deque 指定新的大小
    void resize(size_type _Newsize){
        resize(_Newsize, T());
    }

    void resize(size_type _Newsize, const value_type& val) {
        size_type current_size = size();
        if (_Newsize > current_size) {
            // 扩展并填充新元素
            insert(end(), _Newsize - current_size, val);
        } else if (_Newsize < current_size) {
            // 缩小容器，删除末尾多余元素
            erase(begin() + _Newsize, end());
        }
        // 如果 _Newsize == current_size，无需操作
    }

    //放弃额外容量
    void shrink_to_fit(){
        size_t block_count = 0; //有元素的缓冲区个数
        size_t block_front_IsNullptr_count = 0; //前导为空的元素块个数
        for(auto it = blocks_.begin(); it != blocks_.end(); ++it){
            if(*it != nullptr){
                ++block_count;
            }
            if(!block_count && *it == nullptr){
                ++block_front_IsNullptr_count;
            }
        }
        // 有元素的块移动到前面
        std::rotate(head, head + block_front_IsNullptr_count, head + block_front_IsNullptr_count + block_count);
        // 清除后面为空的块
        blocks_.resize(block_count);
    }
    
    void swap(Deque &other) noexcept;

    //重载比较运算符
    template<typename _Tp, typename _Alloc>
    bool operator==(const Deque<_Tp, _Alloc>& __y) const { 
        return size() == __y.size() && std::equal(cbegin(), cend(), __y.cbegin()); 
    }

    template<typename _Tp, typename _Alloc>
    bool operator!=(const Deque<_Tp, _Alloc>& __y) const { 
        return !(*this == __y); 
    }

    template<typename _Tp, typename _Alloc>
    bool operator<(const Deque<_Tp, _Alloc>& __y) const { 
        return std::lexicographical_compare(cbegin(), cend(), __y.cbegin(), __y.cend()); 
    }

    template<typename _Tp, typename _Alloc>
    bool operator>(const Deque<_Tp, _Alloc>& __y) const { 
        return __y < *this; 
    }

    template<typename _Tp, typename _Alloc>
    bool operator<=(const Deque<_Tp, _Alloc>& __y) const { 
        return !(__y < *this); 
    }

    template<typename _Tp, typename _Alloc>
    bool operator>=(const Deque<_Tp, _Alloc>& __y) const { 
        return !(*this < __y); 
    }

private:
    // 初始化
    void initialize(){
        blocks_.resize(2, nullptr);
        size_t buf_size = deque_buf_size();
        blocks_[0] = allocator_.allocate(buf_size);
        blocks_[1] = allocator_.allocate(buf_size);
        head = iterator(&blocks_[1], blocks_[1], block_size);
        tail = iterator(&blocks_[1], blocks_[1], block_size);
    }

    // 扩充blocks_数组
    void expand_blocks() {
        size_t old_size = blocks_.size();
        size_t new_size = old_size * 2;
        Vector<T*> new_blocks(new_size, nullptr);
        
        // 将原块移动到新数组中间
        size_t mid = new_size / 4;
        std::copy(blocks_.begin(), blocks_.end(), new_blocks.begin() + mid);
        
        // 更新头尾块指针
        head.block = new_blocks.data() + mid + (head.block - blocks_.data());
        tail.block = new_blocks.data() + mid + (tail.block - blocks_.data());
        
        blocks_.swap(new_blocks);
    }

    //初始化为count个val
    void initialize_count_val(size_type Count, const T &Val){
        size_type buf_size = deque_buf_size();

        // 计算实际需要的块数（包括头尾哨兵块）
        size_type required_blocks = (Count > 0) ? (Count + buf_size - 1) / buf_size : 0;
        size_type num_blocks = required_blocks + 2; // 前后各留一个哨兵块

        try {
            // 预分配块并初始化
            blocks_.resize(num_blocks, nullptr);
            for (size_t i = 0; i < num_blocks; ++i) {
                blocks_[i] = allocator_.allocate(buf_size);
            }

            // 初始化头尾
            size_type head_offset = Count / 2;
            size_type head_buf_offset = head_offset / buf_size;
            head_offset -= head_buf_offset * buf_size;
            head = iterator(&blocks_[num_blocks / 2 - head_buf_offset], 
                            blocks_[num_blocks / 2 - head_buf_offset], block_size);
            tail = head;

            // 批量填充元素
            size_type elements_remaining = Count;
            while (elements_remaining > 0) {
                size_type available = static_cast<size_type>(tail.last - tail.cur);
                size_type to_construct = std::min(available, elements_remaining);

                // 构造元素
                std::uninitialized_fill_n(tail.cur, to_construct, T());
                tail += to_construct;
                elements_remaining -= to_construct;
            }

        } catch (...) {
            // 异常安全：释放所有已分配块
            for (auto ptr : blocks_) {
                if (ptr) allocator_.deallocate(ptr, buf_size);
            }
            blocks_.clear();
            throw;
        }
    }

};

template <typename Type, typename other_Alloc>
    void Deque<Type, other_Alloc>::swap(Deque& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(block_size, other.block_size);
        blocks_.swap(other.blocks_);
        if (std::allocator_traits<other_Alloc>::propagate_on_container_swap::value) {
            std::swap(allocator_, other.allocator_);
        }
    }

template <class Type, class Allocator>
void swap(Deque<Type, Allocator>& left, Deque<Type, Allocator>& right) noexcept(noexcept(left.swap(right))) {
    left.swap(right);
}

#endif // DEQUE_H
