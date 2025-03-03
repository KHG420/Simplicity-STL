#ifndef LIST_H
#define LIST_H

//#include <memory>
#include"allocator.h"
#include <initializer_list>
#include <type_traits> 

// 节点结构
template <typename T>
struct ListNode {
    T data;
    ListNode* prev;
    ListNode* next;

    ListNode(const T& value) : data(value), prev(nullptr), next(nullptr) {}
};

// 迭代器
template <typename T, bool IsConst>
class ListIterator {
public:
    using value_type = T;
    using const_pointer = const T*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer  = std::conditional_t<IsConst, const T*, T*>;  // 根据IsConst的类型判断指针类型
    using reference = std::conditional_t<IsConst, const T&, T&>;

    ListIterator(ListNode<T>* node) : current(node) {}

    ListIterator& operator++() {
        current = current->next;
        return *this;
    }

    ListIterator operator++(int) {
        ListIterator temp = *this;
        ++(*this);
        return temp;
    }

    ListIterator& operator--() {
        current = current->prev;
        return *this;
    }

    ListIterator operator--(int) {
        ListIterator temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const ListIterator& other) const {
        return current == other.current;
    }

    bool operator!=(const ListIterator& other) const {
        return !(*this == other);
    }

    T& operator*() const { return this->current->data; }
    T* operator->() const { return &(this->current->data); }

    // 获取当前节点指针
    ListNode<T>* getNode() const {
        return current;
    }

protected:
    ListNode<T>* current;
};

// List 容器类
template <typename T, typename Alloc = Allocator<ListNode<T>>>
class List {

private:
    ListNode<T>* head;  // 头部哨兵节点
    ListNode<T>* tail;  // 尾部哨兵节点
    Alloc allocator;
    size_t size_;

    // 初始化哨兵节点
    void initialize_sentinel_nodes() {
        head = allocator.allocate(1);
        tail = allocator.allocate(1);
        allocator.construct(head, ListNode<T>(T()));
        allocator.construct(tail, ListNode<T>(T()));
        head->next = tail;
        tail->prev = head;
    }

    // 创建默认初始化的元素
    void resize_default(size_t count) {
        for (size_type i = 0; i < count; ++i) {
            emplace_back(T());
        }
    }

public:

    using allocator_type         = Alloc;
    using iterator               = ListIterator<T, false>;
    using const_iterator         = ListIterator<T, true>;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type        = std::ptrdiff_t;
    using value_type             = T;
    using size_type              = size_t;

    // 默认构造函数，使用默认分配器
    List() : size_(0), allocator() {
        initialize_sentinel_nodes();
    }

    // 使用指定分配器创建空列表
    explicit List(const Alloc& al) : allocator(al), size_(0) {
        initialize_sentinel_nodes();
    }

    // 创建Count个默认初始化的元素
    explicit List(size_type count) : List() {
        resize_default(count);
    }

    // 创建Count个Val的副本
    List(size_type count, const T& val) : List() {
        assign(count, val);
    }

    // 创建Count个Val的副本，使用指定分配器
    List(size_type count, const T& val, const Alloc& al) : allocator(al), size_(0) {
        initialize_sentinel_nodes();
        assign(count, val);
    }

    // 拷贝构造函数
    List(List& other) : allocator(other.get_allocator()), size_(0) {
        initialize_sentinel_nodes();
        for (auto it = other.begin(); it != other.end(); it++) {
            push_back(*it);
        }
    }

    // 移动构造函数
    List(List&& other) noexcept 
        : head(other.head), tail(other.tail), allocator(std::move(other.allocator)), size_(other.size_) {
        // 重置原对象的哨兵节点
        other.head = other.allocator.allocate(1);
        other.tail = other.allocator.allocate(1);
        other.allocator.construct(other.head, ListNode<T>(T()));
        other.allocator.construct(other.tail, ListNode<T>(T()));
        other.head->next = other.tail;
        other.tail->prev = other.head;
        other.size_ = 0;
    }

    // 初始化列表构造函数
    List(std::initializer_list<T> IList, const Alloc& al = Alloc()) : allocator(al), size_(0) {
        initialize_sentinel_nodes();
        for (const auto& val : IList) {
            push_back(val);
        }
    }

    // 迭代器范围构造函数
    template <class InputIterator, typename = std::enable_if_t<!std::is_integral<InputIterator>::value>>
    List(InputIterator First, InputIterator Last, const Alloc& al = Alloc()) : allocator(al), size_(0) {
        initialize_sentinel_nodes();
        assign(First, Last);
    }

    //用另一个列表的副本替换列表中的元素
    List& operator=(const List& right){
            if (this != &right) {  // 避免自我赋值
                clear();

                for (const_iterator it = right.cbegin(); it != right.cend(); it++) {
                    push_back(*it);
                }
            }
            return *this;
    }

    List& operator=(List&& right){
        if (this != &right) {  // 避免自我赋值
            clear();
            head = std::move(right.head);
            tail = std::move(right.tail);
            size_ = std::move(right.size_);
            // 将 right 置为空列表
            right.head = right.allocator.allocate(1);
            right.tail = right.allocator.allocate(1);
            right.head->next = right.tail;
            right.tail->prev = right.head;
            right.size_ = 0;
        }
        return *this;
    }

    ~List() {
        clear();
        allocator.destroy(head);
        allocator.deallocate(head, 1);
        allocator.destroy(tail);
        allocator.deallocate(tail, 1);
    }

    // 在尾部插入元素
    void push_back(const T& value) {
        ListNode<T>* newNode = allocator.allocate(1);
        allocator.construct(newNode, ListNode<T>(value));
        newNode->prev = tail->prev;
        newNode->next = tail;
        tail->prev->next = newNode;
        tail->prev = newNode;
        ++size_;
    }

    // 在头部插入元素
    void push_front(const T& value) {
        // 分配新节点的内存
        ListNode<T>* newNode = allocator.allocate(1);
        allocator.construct(newNode, ListNode<T>(value));

        newNode->prev = head;
        newNode->next = head->next;

        // 如果列表为空（头哨兵节点的下一个节点是尾哨兵节点）
        if (head->next == tail) {
            tail->prev = newNode;
        } else {
            head->next->prev = newNode;
        }

        head->next = newNode;
        ++size_;
    }

    // 删除尾部元素
    void pop_back() {
        if (empty()) return;
        ListNode<T>* temp = tail->prev;
        tail->prev->prev->next = tail;
        tail->prev = tail->prev->prev;
        allocator.destroy(temp);
        allocator.deallocate(temp, 1);
        --size_;
    }

    // 删除头部元素
    void pop_front() {
        if (empty()) return;
        ListNode<T>* temp = head->next;
        head->next->next->prev = head;
        head->next = head->next->next;
        allocator.destroy(temp);
        allocator.deallocate(temp, 1);
        --size_;
    }

    //清除列表中的元素，并将一组新元素复制到目标列表
    void assign(size_t n, const T& val) {
        clear();
        for (size_t i = 0; i < n; ++i) {
            push_back(val);
        }
    }

    void assign(std::initializer_list<T> IList) {
        clear();
        for (const auto& it : IList) {
            push_back(it);
        }
    }

    template <typename InputIt,
              typename = std::enable_if_t<!std::is_integral<InputIt>::value>> //进行 SFINAE。当 InputIt 是整数类型时，这个模板会因为替换失败而被排除，避免错误匹配。
    void assign(InputIt first, InputIt last) {
        clear();
        for (InputIt it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    iterator begin() {
        return iterator(head->next);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_iterator cbegin() const {
        return const_iterator(head->next);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    iterator end() {
        return iterator(tail);
    }

    const_iterator cend() const {
        return const_iterator(tail);
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    reference back(){
        return tail->prev->data;
    }

    const_reference back() const{
        return tail->prev->data;
    }

    reference front(){
        return head->next->data;
    }

    const_reference front() const{
        return head->next->data;
    }

    //将构造的元素插入到列表中的指定位置
    void emplace(iterator Where, const T& val) {
        ListNode<T>* newNode = allocator.allocate(1);
        allocator.construct(newNode, val);

        newNode->prev = Where.getNode()->prev;
        newNode->next = Where.getNode();
        Where.getNode()->prev->next = newNode;
        Where.getNode()->prev = newNode;
        ++size_;
    }

    void emplace_back(value_type&& val){
        push_back(std::move(val));
    }

    void emplace_front(value_type&& val){
        push_front(std::move(val));
    }

    // 判断容器是否为空
    bool empty() const {
        return size_ == 0;
    }

    // 获取容器大小
    size_t size() const {
        return size_;
    }

    // 清除容器
    void clear() {
        ListNode<T>* current = head->next;
        while (current != tail) {
            if(!current || !current->next) return;
            ListNode<T>* next = current->next;
            allocator.destroy(current);
            allocator.deallocate(current, 1);
            current = next;
        }
        head->next = tail;
        tail->prev = head;
        size_ = 0;
    }

    //从列表中的指定位置移除一个或一系列元素
    iterator erase(iterator Where){
        ListNode<T>* node = Where.getNode();
        if(node == nullptr || node == tail || node == head) return iterator(nullptr);
        //为头时
        if(node == tail->prev) {
            pop_back();
            return iterator(nullptr);
        }
        //为尾时
        if(node == head->next){
            pop_front();
            return iterator(head->next);
        }
        //中间
        ListNode<T>* temp = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        allocator.destroy(node);
        allocator.deallocate(node, 1);
        size_--;
        return iterator(temp);
    }

    iterator erase(iterator first, iterator last){
        while(first != last){
            iterator temp = first;
            ++first;
            erase(temp);
        }
        return first;
    }

    //返回用于构造列表的分配器对象的一个副本
    allocator_type get_allocator() const{
        return allocator_type();
    }

    //将一个、几个或一系列元素插入列表中的指定位置
    iterator insert(iterator Where, const value_type& Val){

        ListNode<T>* newNode = allocator.allocate(1);
        allocator.construct(newNode, ListNode<T>(Val));
        Where.getNode()->prev->next = newNode;
        newNode->prev = Where.getNode()->prev;
        Where.getNode()->prev = newNode;
        newNode->next = Where.getNode();

        size_++;

        return iterator(newNode);
    }

    iterator insert(iterator Where, value_type&& Val){

        ListNode<T>* newNode = allocator.allocate(1);
        allocator.construct(newNode, move(Val));
        Where.getNode()->prev->next = newNode;
        newNode->prev = Where.getNode()->prev;
        Where.getNode()->prev = newNode;
        newNode->next = Where.getNode();

        size_++;

        return iterator(newNode);
    }

    void insert(iterator Where, size_type Count, const value_type& Val){
        while(Count--){
            Where = insert(Where, Val);
        }
    }

    iterator insert(iterator Where, initializer_list<value_type> IList){
        size_t i = 0;
        iterator temp(nullptr);
        for(auto Val : IList){
            Where = insert(Where, Val);
            if(!i++) temp = Where;
            Where++;
        }
        return temp;
    }

    template <class InputIterator, typename = std::enable_if_t<!std::is_integral<InputIterator>::value>>
    void insert(iterator Where, InputIterator First, InputIterator Last){
        while(First != Last){
            Where = insert(Where, *First++);
            Where++;
        }
    }

    //合并两个链表
    void merge(List& other) {
        if (this == &other || other.empty()) return;

        if (empty()) {
            // 当前链表为空，直接接管other的节点
            head->next = other.head->next;
            other.head->next->prev = head;
            tail->prev = other.tail->prev;
            other.tail->prev->next = tail;
            size_ = other.size_;
            other.head->next = other.tail;
            other.tail->prev = other.head;
            other.size_ = 0;
            return;
        }

        ListNode<T>* this_current = head->next;
        ListNode<T>* other_current = other.head->next;
        ListNode<T>* merged_tail = head;

        while (this_current != tail && other_current != other.tail) {
            if (this_current->data <= other_current->data) {
                merged_tail->next = this_current;
                this_current->prev = merged_tail;
                merged_tail = this_current;
                this_current = this_current->next;
            } else {
                ListNode<T>* next_other = other_current->next;
                // 从other中移除节点
                other_current->prev->next = next_other;
                next_other->prev = other_current->prev;

                // 插入到当前链表
                merged_tail->next = other_current;
                other_current->prev = merged_tail;
                other_current->next = this_current;
                this_current->prev = other_current;

                merged_tail = other_current;
                other_current = next_other;
                --other.size_;
                ++size_;
            }
        }

        // 处理剩余的other节点
        if (other_current != other.tail) {
            merged_tail->next = other_current;
            other_current->prev = merged_tail;

            // 更新当前链表的尾节点
            tail->prev = other.tail->prev;
            other.tail->prev->next = tail;

            size_ += other.size_;
            other.size_ = 0;
        }

        // 重置other的哨兵节点和大小
        other.head->next = other.tail;
        other.tail->prev = other.head;
        other.size_ = 0;
    }

    void sort() {
        if (size_ <= 1) return;

        List<T> tempList;
        tempList.initialize_sentinel_nodes();

        size_t blockSize = 1;
        while (blockSize < size_) {
            // 将原链表内容转移到临时链表
            tempList.clear();
            ListNode<T>* current = head->next;
            while (current != tail) {
                // 分割块并合并到临时链表
                ListNode<T>* left = current;
                ListNode<T>* leftEnd = split(left, blockSize);
                ListNode<T>* right = leftEnd;
                ListNode<T>* rightEnd = split(right, blockSize);

                // 合并到临时链表
                mergeToTemp(tempList, left, leftEnd, right, rightEnd);
                current = rightEnd;
            }

            // 将临时链表内容移回原链表
            clear();
            for (auto& val : tempList) {
                push_back(val);
            }

            blockSize *= 2;
        }
    }

        //清除列表中与指定值匹配的元素
    void remove(const value_type& val){
        for(ListNode<T>* node = head->next; node != tail && node; node = node->next){
            if(node->data == val){
                erase(iterator(node));
            }
        }
    }

    //将满足指定谓词的元素从列表中消除
    template <class Predicate>
    void remove_if(Predicate pred){
        for(ListNode<T>* node = head->next; node != tail && node; node = node->next){
            if(pred(node->data)){
                erase(iterator(node));
            }
        }
    }

    //为列表指定新的大小
    void resize(size_type _Newsize){
        while(size_ > _Newsize){
            pop_back();
        }
        while(size_ < _Newsize){
            push_back(T());
        }
    }

    void resize(size_type _Newsize, value_type val){
        while(size_ > _Newsize){
            pop_back();
        }
        while(size_ < _Newsize){
            push_back(T(val));
        }
    }

    //反转列表中元素的顺序
    void reverse(){
        ListNode<T>* temp = nullptr;
        for(ListNode<T>* node = head; node != nullptr;){
            temp = node->prev;
            node->prev = node->next;
            node->next = temp;
            node = node->prev;
        }
        temp = head;
        head = tail;
        tail = temp;
    }

    //从源列表中删除元素并将其插入到目标列表中
    // insert the entire source list
    void splice(iterator Where, List<value_type, Alloc>& Source) {
        if (this == &Source || Source.empty()) return;

        ListNode<T>* whereNode = Where.getNode();
        ListNode<T>* sourceFirst = Source.head->next;  // 源链表的第一个实际节点
        ListNode<T>* sourceLast = Source.tail->prev;   // 源链表的最后一个实际节点

        // 从源链表断开节点
        Source.head->next = Source.tail;
        Source.tail->prev = Source.head;
        size_ += Source.size_;
        Source.size_ = 0;

        // 将源链表的节点插入目标链表
        sourceFirst->prev = whereNode->prev;
        sourceLast->next = whereNode;
        whereNode->prev->next = sourceFirst;
        whereNode->prev = sourceLast;
    }

    // 插入整个源列表（右值引用）
    void splice(iterator Where, List<value_type, Alloc>&& Source) {
        splice(Where, Source);
    }


    // insert one element of the source list
    void splice(iterator Where, List<value_type, Alloc>& Source, iterator Iter) {
        if (this == &Source || Iter == Source.end()) return;

        ListNode<T>* whereNode = Where.getNode();
        ListNode<T>* sourceNode = Iter.getNode();

        // 从源链表断开该节点
        sourceNode->prev->next = sourceNode->next;
        sourceNode->next->prev = sourceNode->prev;
        Source.size_--;

        // 插入到目标链表
        sourceNode->prev = whereNode->prev;
        sourceNode->next = whereNode;
        whereNode->prev->next = sourceNode;
        whereNode->prev = sourceNode;

        size_++;
    }
    
    void splice(iterator Where, List<value_type, Alloc>&& Source, iterator Iter){
        splice(Where, Source, Iter);
    }

    // insert a range of elements from the source list
    void splice(iterator Where, List<value_type, Alloc>& Source, iterator First, iterator Last) {
        if (this == &Source || First == Last) return;

        ListNode<T>* whereNode = Where.getNode();
        ListNode<T>* firstNode = First.getNode();
        ListNode<T>* lastNode = Last.getNode()->prev;  // Last 是哨兵节点，需取前一个

        // 从源链表断开范围 [firstNode, lastNode]
        firstNode->prev->next = Last.getNode();
        Last.getNode()->prev = firstNode->prev;

        // 插入到目标链表
        firstNode->prev = whereNode->prev;
        lastNode->next = whereNode;
        whereNode->prev->next = firstNode;
        whereNode->prev = lastNode;

        // 更新大小
        size_t movedSize = 0;
        for (auto it = First; it != Last; ++it) movedSize++;
        size_ += movedSize;
        Source.size_ -= movedSize;
    }

    void splice(iterator Where, List<value_type, Alloc>&& Source, iterator First, iterator Last){
        splice(Where, Source, First, Last);
    }

    template<typename Type_1, typename Allocator_1>
    void swap(List<Type_1, Allocator_1>& right){    
        std::swap(head, right.head);
        std::swap(tail, right.tail);

        // 交换大小信息
        std::swap(size_, right.size_);

        // 交换分配器（如果分配器支持交换)
        if constexpr (std::allocator_traits<Allocator_1>::propagate_on_container_swap::value) {
            std::swap(allocator, right.allocator);
        }
    }

    //从列表中删除满足某些其他二元谓词的相邻重复元素或相邻元素
    void unique(){
        ListNode<T>* node = head->next->next;
        T temp = head->next->data;
        while(node != tail){
            if(temp == node->data){
                ListNode<T>* tp = node;
                node = node->prev;
                tp->prev->next = tp->next;
                tp->next->prev = tp->prev;
                allocator.destroy(tp);
                allocator.deallocate(tp, 1);
            }
            temp = node->data;
            node = node->next;
        }
    }

    template <class BinaryPredicate>
    void unique(BinaryPredicate pred){
        ListNode<T>* node = head->next->next;
        T temp = head->next->data;
        while(node != tail){
            if(pred(temp, node->data)){
                ListNode<T>* tp = node;
                node = node->prev;
                tp->prev->next = tp->next;
                tp->next->prev = tp->prev;
                allocator.destroy(tp);
                allocator.deallocate(tp, 1);
            }
            temp = node->data;
            node = node->next;
        }
    }

private:

    // 分割链表，返回分割后的下一个块起始节点
    ListNode<T>* split(ListNode<T>* start, size_t blockSize) {
        ListNode<T>* prev = nullptr;
        for (size_t i = 0; i < blockSize && start != tail; ++i) {
            prev = start;
            start = start->next;
        }
        // 不再修改原链表，仅返回下一个块的起始节点
        return start;
    }
        // 合并两个有序块
    // 合并两个有序块到临时链表
    void mergeToTemp(List<T>& tempList, 
                    ListNode<T>* left, ListNode<T>* leftEnd,
                    ListNode<T>* right, ListNode<T>* rightEnd) {
        ListNode<T>* tempTail = tempList.tail->prev;

        // 合并时创建新节点，而非移动原链表节点
        while (left != leftEnd && right != rightEnd) {
            if (left->data <= right->data) {
                tempList.push_back(left->data);
                left = left->next;
            } else {
                tempList.push_back(right->data);
                right = right->next;
            }
        }

        // 处理剩余节点
        while (left != leftEnd) {
            tempList.push_back(left->data);
            left = left->next;
        }
        while (right != rightEnd) {
            tempList.push_back(right->data);
            right = right->next;
        }
    }

    // 辅助函数：将节点链接到目标尾部
    void linkNode(ListNode<T>*& destTail, ListNode<T>* node) {
        node->prev = destTail;
        node->next = destTail->next;
        destTail->next->prev = node;
        destTail->next = node;
    }
};

#endif //LIST_H