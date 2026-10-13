#ifndef AVIONICS_RTE_RING_BUFFER_HPP
#define AVIONICS_RTE_RING_BUFFER_HPP

#include <cstdint>

template<typename T, uint32_t N>
class RingBuffer {
    static_assert((N & (N - 1)) == 0u, "Ring buffer size must be a power of two!");
    static constexpr uint32_t BUFFER_MASK = N - 1ul;

public:
    bool write(const T& data) {
        uint32_t next = (_head + 1) & BUFFER_MASK;
        if(next == _tail) {
            return false;
        }

        _buffer[_head] = data;
        _head = next;

        return true;
    }

    bool read(T& data) {
        if(_tail == _head) {
            return false;
        }

        data = _buffer[_tail];
        _tail = (_tail + 1) & BUFFER_MASK;

        return true;
    }

    bool is_empty() const { return _head == _tail;}
    bool is_full() const { return ((_head + 1) & BUFFER_MASK) == _tail;}

private:
    T _buffer[N]{};

    volatile uint32_t _head{0u};
    volatile uint32_t _tail{0u};
};

#endif // AVIONICS_RTE_RING_BUFFER_HPP
