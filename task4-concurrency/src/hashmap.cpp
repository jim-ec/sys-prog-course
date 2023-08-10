#include "../chashmap.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>


struct Node {
    long value = 0;
    Node* next = nullptr;
};


struct List {
    Node* sentinel = nullptr;
    std::mutex lock;
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
        map->buckets[i]->sentinel = new Node;
        if (map->buckets[i]->sentinel == nullptr) {
            return nullptr;
        }
    }
    return map;
}


void free_hashmap(Map* map) {
    for (size_t i = 0; i < map->length; ++i) {
        map->buckets[i]->lock.lock();
        Node* node = map->buckets[i]->sentinel;
        while (node != nullptr) {
            Node* next = node->next;
            delete node;
            node = next;
        }
        map->buckets[i]->lock.unlock();
        delete map->buckets[i];
    }
    delete map->buckets;
    delete map;
}


static int getIndex(Map* map, long value) {
    size_t index = value % map->length;
    if (index < 0) {
        index += map->length;
    }
    return index;
}


int insert_item(Map* map, long value) {
    auto index = getIndex(map, value);
    std::lock_guard lock(map->buckets[index]->lock);

    Node* insertion = new Node;
    if (insertion == nullptr) {
        return 1;
    }
    insertion->value = value;

    Node* node = map->buckets[index]->sentinel;
    insertion->next = node->next;
    node->next = insertion;
    return 0;
}


int remove_item(Map* map, long value) {
    auto index = getIndex(map, value);
    std::lock_guard lock(map->buckets[index]->lock);
    Node* node = map->buckets[index]->sentinel;
    while (node->next != nullptr) {
        if (node->next->value == value) {
            Node* deletion = node->next;
            node->next = node->next->next;
            delete deletion;
            return 0;
        }
        node = node->next;
    }
    return 1;
}


int lookup_item(Map* map, long value) {
    auto index = getIndex(map, value);
    std::lock_guard lock(map->buckets[index]->lock);
    Node* node = map->buckets[index]->sentinel->next;
    while (node != nullptr) {
        if (node->value == value) {
            return 0;
        }
        node = node->next;
    }
    return 1;
}


void print_hashmap(Map* map) {
    for (size_t i = 0; i < map->length; ++i) {
        std::lock_guard lock(map->buckets[i]->lock);
        std::cout << "Bucket " << i << " - ";
        Node* node = map->buckets[i]->sentinel->next;
        while (node != nullptr) {
            std::cout << node->value << ' ';
            node = node->next;
        }
        std::cout << '\n';
    }
    std::cout << std::flush;
}
