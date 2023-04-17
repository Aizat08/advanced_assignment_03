#include <iostream>
#include <memory>
#include <vector>
#include <map>

int factorial(int);

template <typename T, size_t N>
struct allocator {

    using value_type = T;
    std::shared_ptr<T> ptr = nullptr;
    std::size_t PoolSize;
    std::size_t offset;

    allocator() noexcept {
        offset = 0;
        PoolSize = N;
        ptr.reset(static_cast<T*>(::operator new(PoolSize  * sizeof(T))));
        } 

    template<typename U> allocator(const allocator<U, N>& lhs) noexcept{
        ptr = lhs.ptr; }

    T* allocate(std::size_t n) {
        
        std::size_t requiredSize = n;

        if(requiredSize > PoolSize - offset){
            throw std::bad_alloc(); }
           
        T* ptr_allocated = static_cast<T*>(static_cast<void*>(ptr.get())) + offset;

        offset+=requiredSize;

        return ptr_allocated;
    }

    void deallocate (T* p, std::size_t n){}

    template<typename U>
    struct rebind {
        typedef allocator<U, N> other;
    };
}; 

template <class T, class U, size_t N>
constexpr bool operator== (const allocator<T, N>& a1, const allocator<U, N>& a2) noexcept
{
    return a1.ptr == a2.ptr;
} 

template <class T, class U, size_t N>
constexpr bool operator!= (const allocator<T, N>& a1, const allocator<U, N>& a2) noexcept
{
    return a1.ptr != a2.ptr;
} 

template<typename T, typename Allocator=std::allocator<T>>
class List{
    
    struct Node{
        T value;
        Node* next;
        Node(const T& value) : value(value), next(nullptr) {};
    };

    public:
    List() : head(nullptr), tail(nullptr), size(0) {}

    void push_back(const T& value){

        Node* node = nodeAlloc.allocate(1);
        std::allocator_traits<NodeAlloc>::construct(nodeAlloc, node, value);
        if(size==0){head = tail = node;}
        else{
            tail->next = node;
            tail = node;}

        size++;
    }

    void print(){
        
        Node* current_node = head;
        while(current_node != nullptr){
            std::cout << current_node->value << " ";
            current_node = current_node->next; }
        std::cout << std::endl;
    }

    ~List(){
        Node* current_node = head;
        while(current_node != nullptr){
            Node* next = current_node->next;
            std::allocator_traits<NodeAlloc>::destroy(nodeAlloc, current_node);
            nodeAlloc.deallocate(current_node, 1); 
            current_node = next; }

        head = nullptr;
        tail = nullptr;
        size = 0;
    }

    private:
    Node* head;
    Node* tail;
    std::size_t size;
    using NodeAlloc = typename Allocator::template rebind<Node>::other;
    typename Allocator::template rebind<Node>::other nodeAlloc;
};

template<typename Key, typename T>
using Map = std::map<Key, T, std::less<Key>, allocator<std::pair<const Key, T>, 10>>;

int main(){

    //1. std::map with STL allocator and fill 
    std::map<int, int> map;
    for(int i=0; i<10; i++){
        map.insert({i, factorial(i)});}

    //2. std::map with custom allocator and fill
    Map<int, int> map1;
    for(int i=0; i<10; i++){
        map1.insert({i, factorial(i)});}

    //3. print maps
    for(auto it=map.begin(); it!=map.end(); ++it){
        std::cout << it->first << " " << it->second << std::endl;}

    for(auto it=map1.begin(); it!=map1.end(); ++it){
        std::cout << it->first << " " << it->second << std::endl;}

    //4. custom List with STL allocator and fill
    List<int> list;
    for(int i=0; i<10; i++){
        list.push_back(i); }

    //5. custom List with custom allocator and fill
    List<int, allocator<int, 10>> list1;
    for(int i=0; i<10; i++){
        list1.push_back(i); }

    //6. print lists
    list.print();
    list1.print();
}

int factorial(int number){

    int factorial=1;
    for (int a=1;a<=number;a++) {
        factorial=factorial*a;}

    return factorial;
}