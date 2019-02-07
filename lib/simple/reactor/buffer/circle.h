#ifndef LIB_REACTOR_BUFFER_CIRCLE_H
#define LIB_REACTOR_BUFFER_CIRCLE_H

namespace simple::reactor::buffer {

class circle {
    public:
        template<class type> friend class buffer;

    public:
        circle(size_t max_size = 4096, size_t default_size = 4096): data_(max_size, default_size) {
            reset();
        }

    public:
        void reset() {
            head_ = tail_ = 0;
        }

    public:
        size_t get_space() {
            if (size_t space = get_count(tail_ + 1, head_, data_.get_size())) {
                return space;
            }
            expand();
            return get_space();
        }
        size_t get_continguious_space() {
            if (size_t space = get_continguious_space(head_, tail_, data_.get_size())) {
                return space;
            }
            expand();
            return get_continguious_space();
        }

        size_t get_count() {
            return get_count(head_, tail_, data_.get_size());
        }

        size_t get_continguious_count() {
            return get_continguious_count(head_, tail_, data_.get_size());
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
            tail_ = (tail_ + count) % data_.get_size();
        }

    public:
        void remove(size_t count) {
            head_ = (head_ + count) % data_.get_size();
        }

        void remove_from_back(size_t count) {
            tail_ -= count;
            if (tail_ < 0) {
                tail_ += data_.get_size();
            }
        }

    public:
        void read(uint8* data, size_t count, size_t offset, bool peek = true) {
            if (offset == -1) {
                peek = false;
                offset = 0;
            }
            if (get_count() < offset + count) {
                /*
                expand();
                return read(data, count, offset, peek);
                */
                throw ERROR_READ_INVALID_OFFSET;
            }
            size_t head = (head_ + offset) % data_.get_size();
            size_t read_count = std::min(get_continguious_count(head, tail_, data_.get_size()), count);
            memcpy(data, data_.start + head, read_count);
            if (peek) {
                offset += read_count;
            } else {
                head_ = (head_ + read_count) % data_.get_size();
            }
            if (size_t remain_count = count - read_count) {
                read(data + read_count, remain_count, offset, peek);
            }
        }

        void write(const uint8* data, size_t count, size_t offset) {
            if (offset != -1) {
                if (get_count() < offset + count)
                    throw ERROR_WRITE_INVALID_OFFSET;
            } else {
                if (get_space() < count) {
                    expand();
                    return write(data, count, offset);
                }
                offset = get_count();
                written(count);
            }
            
            size_t head = (head_ + offset) % data_.get_size();
            size_t write_count = std::min(get_continguious_space(head, tail_, data_.get_size()), count);
            memcpy(data_.start + head, data, write_count);
            if (size_t remain_count = count - write_count) {
                write(data + write_count, remain_count, offset + write_count);
            }
        }

        void write(const circle& circle, size_t offset, size_t count) {
            size_t head = (circle.head_ + offset) % circle.data_.get_size();
            if (size_t read_count = std::min(circle.get_continguious_count(head, circle.tail_, circle.data_.get_size()), count)) {
                write(circle.data_.start + head, read_count, -1);
                if (size_t remain_count = count - read_count) {
                    return write(circle, offset + read_count, remain_count);
                }
            }
        }

    public:
        size_t get_count(size_t head, size_t tail, size_t size) const {
            return (((tail) - (head)) & ((size)-1));
        }

        size_t get_continguious_count(size_t head, size_t tail, size_t size) const {
            return ({int end = (size) - (head); \
             int n = ((tail) + end) & ((size)-1); \
             n < end ? n : end;});
        }
        size_t get_continguious_space(size_t head, size_t tail, size_t size) {
            return ({int end = (size) - 1 - (tail); \
             int n = (end + (head)) & ((size)-1); \
             n <= end ? n : end+1;});
        }

    private:
        void expand() {
            size_t size = data_.get_size();
            size <<= 2;
            if (size >= data_.max_size)
                throw ERROR_WRITE_NO_SPACE;

            data data(data_.max_size, size);
            size_t count = get_count();
            size_t read_count = std::min(get_continguious_count(), count);
            memcpy(data.start, data_.start + head_, read_count);
            if (size_t remain_count = count - read_count) {
                head_ = (head_ + read_count) % data_.get_size();
                memcpy(data.start + read_count, data_.start + head_, remain_count);
            }
            head_ = 0;
            tail_ = count;
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
