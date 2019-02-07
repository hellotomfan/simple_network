#ifndef LIB_REACTOR_BUFFER_ARRAY_H
#define LIB_REACTOR_BUFFER_ARRAY_H

namespace simple::reactor::buffer {

class array {
    public:
        template<class type> friend class buffer;

    public:
        array(size_t max_size = 4096, size_t default_size = 4096): data_(max_size, default_size) {
            reset();
        }

    public:
        void reset() {
            head_ = tail_ = 0;
        }

    public:
        size_t get_count() const {
            return tail_ - head_;
        }
        size_t get_continguious_count() {
            return get_count();
        }
        size_t get_space() {
            size_t space = data_.get_size() - get_count();
            if (space == 0) {
                expand();
                return get_space();
            }
            return space;
        }
        size_t get_continguious_space() {
            size_t continguious_space = data_.end - data_.start - tail_;
            if (continguious_space == 0) {
                expand();
                return get_continguious_count();
            }
            return continguious_space;
        }

    public:
        uint8* get_start() const {
            return data_.start + head_;
        }
        uint8* get_end() const {
            return data_.start + tail_;
        }

    public:
        void written(size_t count) {
            tail_ += count;
        }

    public:
        void remove(size_t count) {
            if (get_count() < count) {
                throw ERROR_REMOVE_NO_SPACE;
            }
            head_ += count;
            if (head_ == tail_) {
                head_ = tail_ = 0;
            } else {
                memmove(data_.start, data_.start + head_, get_count());
                tail_ -= head_;
                head_ = 0;
            }
        }

        void remove_from_back(size_t count) {
            if (get_count() < count) {
                throw ERROR_REMOVE_FROM_BACK_NO_SPACE;
            }
            tail_ -= count;
        }

    public:
        void read(uint8* data, size_t count, size_t offset) {
            bool peek = true;
            if (offset == -1) {
                peek = false;
                offset = 0;
            }
            if (get_count() < offset + count) {
                throw ERROR_READ_INVALID_OFFSET;
            }
            memcpy(data, data_.start + head_ + offset, count);
            if (!peek) {
                head_ += count;
            }
        }

        void write(const uint8* data, size_t count, size_t offset) {
            if (offset == -1) {
                offset = get_count();
                do {
                    try {
                        if (get_continguious_space() < count) {
                            if (head_ != 0) {
                                memmove(data_.start, data_.start + head_, get_count());
                                tail_ -= head_;
                                head_ = 0;
                            }
                            if (get_continguious_space() < count) {
                                throw ERROR_WRITE_NO_SPACE;
                            }
                        }
                        memcpy(data_.start + tail_, data, count);
                        tail_ += count;
                        return;
                    } catch (...) {
                        expand();
                    }
                } while (true);
            } else {
                if (get_count() < offset + count) {
                    throw ERROR_WRITE_INVALID_OFFSET;
                }
                memcpy(data_.start + head_ + offset, data, count);
            }
        }

        void write(const array& array, size_t offset, size_t count) {
            if (array.get_count() < offset + count) {
                throw ERROR_READ_NO_ENOUGH_SPACE;
            }
            write(array.data_.start + array.head_ + offset, count, -1);
        }

    public:
        void expand() {
            size_t size = data_.get_size();
            if (size == data_.max_size)
                throw ERROR_WRITE_NO_SPACE;

            size <<= 2;
            size = size < data_.max_size ? size : data_.max_size;
            data data(data_.max_size, size);

            memcpy(data.start, data_.start + head_, get_count());
            tail_ -= head_;
            head_ = 0;

            data_ = data;
        }

    private:
        data data_;

    private:
        size_t head_;
        size_t tail_;
};

}

#endif
