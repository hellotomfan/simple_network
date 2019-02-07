#ifndef LIB_REACTOR_BUFFER_BUFFER_H
#define LIB_REACTOR_BUFFER_BUFFER_H

#include "../../../common.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <cassert>


namespace simple::reactor::buffer {

enum {
    ERROR_READ_NO_ENOUGH_SPACE,
    ERROR_WRITE_INVALID_OFFSET,
    ERROR_WRITE_NO_SPACE,
    ERROR_READ_INVALID_OFFSET,
    ERROR_REMOVE_FROM_BACK_NO_SPACE,
    ERROR_REMOVE_NO_SPACE,
};

struct data {
    data(size_t _max_size = 4096, size_t default_size = 4096):
        mem(new uint8[default_size]),
        start(mem.get()),
        end(start + default_size),
        max_size(_max_size) {
        }
    data& operator= (data& data) {
        mem.reset(data.mem.release());
        start = data.start;
        end = data.end;
        max_size = data.max_size;
        return *this;
    }
    size_t get_size() const { 
        return end - start;
    }
    std::unique_ptr<uint8[]> mem;
    uint8* start;
    uint8* end;
    size_t max_size;
};

}

#include "bip.h"
#include "circle.h"
#include "array.h"

namespace simple::reactor::buffer {

template <class type=circle> class buffer {
    friend class bip;
    friend class circle;
    friend class array;

    public:
        buffer(size_t max_size = 409600, size_t default_size = 1024): 
            type_(max_size, default_size) {
        }

    public:
        // >> / << 
        template <class value_type> buffer& operator>> (value_type& value) {
            read(&value, sizeof(value));
            return *this;
        }
        template <class value_type> buffer& operator<< (const value_type& value) {
            write(&value, sizeof(value));
            return *this;
        }

        // read
        void read(uint8 *data, size_t count, size_t offset) {
            return type_.read(data, count, offset);
        }
        template <class data_type> void read(data_type& data) {
            return read((uint8*)&data, sizeof(data), -1);
        }
        template <class data_type> void read(data_type& data, size_t offset) {
            return read((uint8*)&data, sizeof(data), offset);
        }

        // write
        void write(const uint8* data, size_t count, size_t offset) {
            return type_.write(data, count, offset);
        }   
        void write(uint8 *data, size_t count) {
            return write(data, count, -1);
        }
        void write(const uint8 *data, size_t count) {
            return write(data, count, -1);
        }
        template <class data_type> void write(const data_type& data) {
            return write((const uint8*)&data, sizeof(data), -1);
        }
        template <class data_type> void write(const data_type& data, size_t offset) {
            return write((const uint8*)&data, sizeof(data), offset);
        }
        void write(const buffer<type>& buffer, size_t offset, size_t count) {
            return type_.write(buffer.type_, offset, count);
        }

    public:
        uint8 *get_start() {
            return type_.get_start();
        }
        uint8 *get_end() {
            return type_.get_end();
        }
        size_t get_count() {
            return type_.get_count();
        }
        size_t get_space() {
            return type_.get_space();
        }
        size_t get_continguious_count() {
            return type_.get_continguious_count();
        }
        size_t get_continguious_space() {
            return type_.get_continguious_space();
        }
        void remove(size_t count) {
            return type_.remove(count);
        }
        void written(size_t count) {
            return type_.written(count);
        }
        void remove_from_back(size_t count) {
            return type_.remove_from_back(count);
        }

    public:
        void reset() {
            return type_.reset();
        }
        bool operator == (const buffer<type>& buffer) const {
            if (type_.data_.mem.get() == buffer.type_.data_.mem.get())
                return true;
            return false;
        }

    private:
        type type_;
};

}
#endif
