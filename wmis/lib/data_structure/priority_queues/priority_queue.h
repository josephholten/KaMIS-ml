//
// Created by joseph on 4/26/22.
//

#ifndef KAMIS_PRIORITY_QUEUE_H
#define KAMIS_PRIORITY_QUEUE_H

#include <vector>
#include <cstddef>
#include <random>
#include <cassert>
#include <algorithm>
#include <iostream>

// priority_t must be integer-like type

enum class priority_direction { MIN, MAX };
enum order_t { up = 0b100, down = 0b010, equal = 0b001};

template<typename priority_t, priority_direction direction>
class priority_queue {
public:
    priority_queue() = default;

    explicit priority_queue(size_t n) {
        heap.resize(n);
        std::iota(heap.begin(), heap.end(), 0);

        indices.resize(n);
        std::iota(indices.begin(), indices.end(), 0);

        priorities.resize(n, 0);

        _size = n;
    };

    explicit priority_queue(std::vector<priority_t> priorities)
        : priority_queue(priorities.size())
    {
        build(std::move(priorities));
    }

    // keys are the indices
    void build(std::vector<priority_t> priorities) {
        this->priorities = std::move(priorities);

        // print();

        if constexpr(direction == priority_direction::MIN) {
            for (size_t i = 1; i < _size; ++i)
                sieve(order_t::up, i);
        }
        if constexpr(direction == priority_direction::MAX) {
            for (size_t i = _size / 2; i > 0; --i)
                sieve(order_t::down, i);
            sieve(order_t::down, 0);
        }
    };

    size_t pop() {
        assert(_size >= 1);
        auto min = heap.front();
        _size -= 1;

        swap(0, _size);

        sieve(order_t::down, 0);

        return min;
    }

    void set(size_t key, priority_t new_priority) {
        size_t index = indices[key];
        assert(index < _size && "setting index which was removed");

        auto o = order(new_priority, priorities[key]);
        priorities[key] = new_priority;
        sieve(o, index);
    };

    size_t size() {
        return _size;
    }

    bool empty() {
        return _size == 0;
    }

    size_t top() {
        return heap.front();
    };

    priority_t priority(size_t key) {
        return priorities[key];
    }

    order_t order(priority_t p1, priority_t p2) {
        if constexpr(direction == priority_direction::MIN) {
            if (p1 < p2)
                return order_t::up;
            else if (p1 > p2)
                return order_t::down;
            else
                return order_t::equal;
        }
        if constexpr(direction == priority_direction::MAX) {
            if (p1 > p2)
                return order_t::up;
            else if (p1 < p2)
                return order_t::down;
            else
                return order_t::equal;
        }
    }

    void check_correctness() {
        for (size_t i = 0; i < _size; ++i) {
            size_t l_child = 2*i + 1;
            size_t r_child = 2*i + 2;

            if (l_child < _size)
                assert(order(priorities[heap[l_child]], priorities[heap[i]]) & (down | equal));
            if (r_child < _size)
                assert(order( priorities[heap[r_child]], priorities[heap[i]]) & (down | equal));

            assert(indices[heap[i]] == i);
        }
    }

    void print(size_t depth = 0, size_t index = 0) {
        if (index == 0) {
            std::cout << "indices: ";
            for (int i = 0; i < _size; ++i) {
                std::cout << i << " (" << indices[i] << ")";
                if (i != _size - 1)
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }

        if (index < _size)
            std::cout << "(" << index << ") " << heap[index] << ": " << priorities[heap[index]] << std::endl;

        size_t l_child = 2*index + 1;
        size_t r_child = 2*index + 2;
        depth += 2;

        if (l_child < _size) {
            for (int i = 0; i < depth; ++i)
                std::cout << " ";

            print(depth, l_child);
        }

        if (r_child < _size) {
            for (int i = 0; i < depth; ++i)
                std::cout << " ";
            print(depth, r_child);
        }
    }

private:
    void sieve(order_t o, size_t index) {
        if (o == order_t::up) {
            size_t parent = (index - 1) / 2;
            while (index > 0 && order(priorities[heap[parent]], priorities[heap[index]]) != up) {
                swap(index, parent);
                index = parent;
                parent = (index - 1) / 2;

                // print();
            }

        } else if (o == order_t::down) {
            while(!is_leaf(index)) {
                size_t top_child = get_top_child(index);
                if (order(priorities[heap[top_child]], priorities[heap[index]]) == down)
                    break;
                swap(index, top_child);
                index = top_child;

                // print();
            }
        }
    }

    void swap(size_t id1, size_t id2) {
        std::swap(heap[id1], heap[id2]);
        indices[heap[id1]] = id1;
        indices[heap[id2]] = id2;
    }

    bool is_leaf(size_t index) {
        return 2*index + 1 >= _size;
    }

    size_t get_top_child(size_t index) {
        size_t l_child = 2*index + 1;
        size_t r_child = 2*index + 2;

        if (r_child >= _size || order(priorities[heap[l_child]], priorities[heap[r_child]]) == up)
            return l_child;
        else
            return r_child;
    }

    std::vector<size_t> heap;
    std::vector<size_t> indices;
    size_t _size = 0;
    std::vector<priority_t> priorities;
};

#if 0
void test_min_priority_queue() {
    std::random_device rd;
    std::mt19937 rng(1);
    std::uniform_real_distribution<float> _uni_real(0, 1);
    std::uniform_int_distribution<int> _uni_int(0, 9);

    auto prioritiy = [&]{ return _uni_real(rng); };
    auto key = [&]{ return _uni_int(rng); };

    std::vector<float> priorities(10);
    for (auto& prio : priorities)
        prio = prioritiy();

    priority_queue<float, priority_direction::MIN> empty;
    assert(empty.empty());

    priority_queue<float, priority_direction::MIN> q(priorities);
    q.check_correctness();

    for (int i = 0; i < 100; ++i) {
        auto k = key();
        auto new_priority = prioritiy();
        q.set(k, new_priority);
        assert(q.priority(k) == new_priority);
        q.check_correctness();
    }

    std::vector<float> maybe_sorted;
    while (!q.empty()) {
        auto top = q.top();
        maybe_sorted.push_back(q.priority(top));
        q.pop();
    }

    for (int i = 0; i < maybe_sorted.size() - 1; ++i)
        assert(maybe_sorted[i] <= maybe_sorted[i+1]);

    std::cout << "\nmax heap\n\n";

    priority_queue<float, priority_direction::MAX> q2(priorities);
    q2.print();

    q2.check_correctness();

    for (int i = 0; i < 100; ++i) {
        auto k = key();
        auto new_priority = prioritiy();
        q2.set(k, new_priority);
        assert(q2.priority(k) == new_priority);
        q2.check_correctness();
    }

    std::vector<float> maybe_sorted2;
    while (!q2.empty()) {
        auto top = q2.top();
        maybe_sorted2.push_back(q2.priority(top));
        q2.pop();
    }

    for (int i = 0; i < maybe_sorted2.size() - 1; ++i)
        assert(maybe_sorted2[i] >= maybe_sorted2[i+1]);
}
#endif

#endif //KAMIS_PRIORITY_QUEUE_H
