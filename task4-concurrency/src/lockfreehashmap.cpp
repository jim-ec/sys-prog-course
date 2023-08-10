#include "../chashmap.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <atomic>
#include <utility>
#include <tuple>


/// Implementing a lock-free hash map, using the lock-free linked list proposed here:
/// https://www.cl.cam.ac.uk/research/srg/netos/papers/2001-caslists.pdf


/// "Compare-and-swap" function.
/// Compares contents of `self` to `expectedPast`, and if they match,
/// copy `desiredFuture` into it, and return `true`.
/// If they do not match, simply return `false`.
template<class T>
bool cas(std::atomic<T>& self, T expectedPast, T desiredFuture) {
    return self.compare_exchange_strong(expectedPast, desiredFuture);
}


template<class T>
static T* get_marked_reference(T* pointer) {
    return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pointer) | 0x1);
}


template<class T>
static T* get_unmarked_reference(T* pointer) {
    return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pointer) & ~0x1);
}


template<class T>
static bool is_marked_reference(T* pointer) {
    return (reinterpret_cast<intptr_t>(pointer) & 0x1) != 0;
}


struct Node {
    long key = 0;
    std::atomic<Node*> next = nullptr;
};


struct List {
    Node* head = nullptr;
    Node* tail = nullptr;


    List() {
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->next = tail; // An iteration over the node chain can never derail, because next is never null
    }


    ~List() {
        for (Node* node = head; node != tail; ) {
            Node* deletion = node;
            node = node->next;
            // delete deletion;
        }
        // delete tail;
    }


    std::pair<Node*, Node*> search(long key) {
        for (;;) {
            // 1: Find left and right
            Node* node = head;
            Node* nodeNext = head->next;
            Node* leftNext = nullptr;
            Node* left = nullptr;
            do {
                if (!is_marked_reference(nodeNext)) {
                    left = node;
                    leftNext = nodeNext;
                }
                node = get_unmarked_reference(nodeNext);
                if (node == tail) {
                    break;
                }
                nodeNext = node->next;
            } while (is_marked_reference(nodeNext) || (node->key < key));
            Node* right = node;

            // 2: Check nodes are adjacent
            if (leftNext == right) {
                if ((right != tail) && is_marked_reference(right->next.load())) {
                    continue;
                }
                else {
                    return {left, right};
                }
            }

            // 3: Remove one or more marked nodes
            if (cas(left->next, leftNext, right)) {
                // delete leftNext;
                if ((right != tail) && is_marked_reference(right->next.load())) {
                    continue;
                }
                else {
                    return {left, right};
                }
            }
        }
    }


    bool insert(long key) {
        Node* insertion = new Node();
        insertion->key = key;

        for (;;) {
            auto [left, right] = search(key);
            if ((right != tail) && (right->key == key)) {
                return false;
            }
            insertion->next = right;
            if (cas(left->next, right, insertion)) {
                return true;
            }
        }
    }


    bool remove(long key) {
        for (;;) {
            auto [left, right] = search(key);

            if ((right == tail) || (right->key != key)) {
                return false;
            }

            Node* rightNext = right->next;
            if (!is_marked_reference(rightNext) && cas(right->next, rightNext, get_marked_reference(rightNext))) {
                if (cas(left->next, right, rightNext)) {
                    // delete right;
                }
                else {
                    std::tie(left, right) = search(right->key);
                }
                return true;
            }
        }
    }


    bool find(long key) {
        auto [left, right] = search(key);
        return (right != tail) && (right->key == key);
    }

    
    void print() {
        for (Node* node = head->next; node != tail; node = node->next) {
            std::cout << node->key << ' ';
            while (is_marked_reference(node->next.load())) {
                node = node->next;
            }
        }
    }
};


struct Map {
    List** buckets;
    size_t length;
};


Map* alloc_hashmap(size_t n_buckets) {
    Map* map = new Map;
    if (map == nullptr) {
        return nullptr;
    }
    map->length = n_buckets;
    map->buckets = new List* [n_buckets];
    if (map->buckets == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < n_buckets; ++i) {
        map->buckets[i] = new List;
        if (map->buckets[i] == nullptr) {
            return nullptr;
        }
    }
    return map;
}


void free_hashmap(Map* map) {
    for (size_t i = 0; i < map->length; ++i) {
        // delete map->buckets[i];
    }
    // delete map->buckets;
    // delete map;
}


static int getIndex(Map* map, long key) {
    size_t index = key % map->length;
    if (index < 0) {
        index += map->length;
    }
    return index;
}


int insert_item(Map* map, long key) {
    auto index = getIndex(map, key);
    return map->buckets[index]->insert(key) ? 0 : 1;
}


int remove_item(Map* map, long key) {
    auto index = getIndex(map, key);
    return map->buckets[index]->remove(key) ? 0 : 1;
}


int lookup_item(Map* map, long key) {
    auto index = getIndex(map, key);
    return map->buckets[index]->find(key) ? 0 : 1;
}


void print_hashmap(Map* map) {
    for (size_t i = 0; i < map->length; ++i) {
        std::cout << "Bucket " << i << " - ";
        map->buckets[i]->print();
        std::cout << '\n';
    }
    std::cout << std::flush;
}
