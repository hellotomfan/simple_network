#ifndef LIB_REACTOR_BUFFER_BIP_H
#define LIB_REACTOR_BUFFER_BIP_H


namespace simple::reactor::buffer {


class bip {
    public:
        template<class type> friend class buffer;

    public:
        bip(size_t max_size = 4096, size_t default_size = 4096): data_(max_size, default_size) {
            reset();
        }

    public:
        void reset() {
            region_a_start_ = nullptr;
            region_a_count_ = 0;
            region_b_start_ = nullptr;
            region_b_count_ = 0;
            region_a_start_ = data_.start;
        }

    public:
        size_t get_count() const  {
            return region_a_count_ + region_b_count_;
        }

        size_t get_space() {
            if (size_t space = data_.get_size()  - get_count()) {
                return space;
            }
            expand();
            return get_space();
        }

        size_t get_continguious_count() const {
            return region_a_count_ ? region_a_count_ : region_b_count_;
        }

        size_t get_continguious_space() {
            size_t continguious_space;
            if (region_b_count_) {
                continguious_space = get_b_space();
            } else {
                if (get_a_space() < get_space_before_a()) {
                    region_b_start_ = data_.start;
                    continguious_space = get_b_space();
                }
                continguious_space = get_a_space();
            }

            if (continguious_space) {
                return continguious_space;
            }
            expand();
            return get_continguious_space();
        }

    public:
        uint8* get_start() const {
            return region_a_count_ ? region_a_start_ : region_b_start_;
        }

        uint8* get_end() const {
            return region_b_start_ ? region_b_start_ + region_b_count_ : region_a_start_ + region_a_count_;
        }

    public:
        void written(size_t count) {
            if (region_b_start_)
                region_b_count_ += count;
            else
                region_a_count_ += count;
        }

    public:
        void remove(size_t count) {
            if (get_count() < count)
                throw ERROR_REMOVE_NO_SPACE;

            if (region_a_count_ > 0) {
                size_t region_count = std::min(region_a_count_, count);
                region_a_start_ += region_count;
                region_a_count_ -= region_count;
                count -= region_count;
            }
            if (count > 0) {
                region_b_start_ += count;
                region_b_count_ -= count;
            }

            if (region_a_count_ == 0) {
                if (region_b_count_ == 0) {
                    region_a_start_ = data_.start;
                } else {
                    region_a_start_ = region_b_start_;
                    region_a_count_ = region_b_count_;
                }
                region_b_start_ = nullptr;
                region_b_count_ = 0;
            }
        }

        void remove_from_back(size_t count) {
            if (get_count() < count) {
                return;
            }
            if (region_b_count_ > 0) {
                size_t region_count;
                region_count = std::min(region_b_count_, count);
                region_b_count_ -= region_count;
                count -= region_count;
            }
            if (count > 0) {
                region_a_count_ -= std::min(region_a_count_, count);
            }
            if (region_b_count_ == 0) {
                region_b_start_ = nullptr;
                if (region_a_count_ == 0) {
                    region_a_start_ = data_.start;
                }
            }
        }

    public:
        void read(uint8 *data, size_t count, size_t offset) {
            bool peek = true;
            if (offset == -1) {
                peek = false;
                offset = 0;
            }
            if (get_count() < offset + count) {
                throw ERROR_READ_INVALID_OFFSET;
            }

            size_t region_a_read_count = 0;
            if (region_a_count_ > 0) {
                if (region_a_count_ < offset) {
                    offset -= region_a_count_;
                } else {
                    region_a_read_count = std::min(region_a_count_ - offset, count);
                    memcpy(data, region_a_start_ + offset, region_a_read_count);
                    if (!peek) {
                        region_a_start_ += region_a_read_count;
                        region_a_count_ -= region_a_read_count;
                    }
                    count -= region_a_read_count;
                }
            }
            if (count > 0) {
                memcpy(data + region_a_read_count, region_b_start_, count);
                if (!peek) {
                    region_b_start_ += count;
                    region_b_count_ -= count;
                }
            }

            if (!peek) {
                if (region_a_count_ == 0) {
                    if (region_b_count_ == 0) {
                        region_a_start_ = data_.start;
                    } else {
                        region_a_start_ = region_b_start_;
                        region_a_count_ = region_b_count_;
                    }
                    region_b_start_ = nullptr;
                    region_b_count_ = 0;
                }
            }
        }

        void write(const uint8 *data, size_t count, size_t offset) {
            if (offset == -1) {
                offset = get_count();
            } else {
                if (get_count() < offset + count) {
                    throw ERROR_WRITE_INVALID_OFFSET;
                }
            }

            bool retry;
            do {
                retry = false;
                auto remain_count = count;
                if (region_a_count_ > offset) {
                    auto region_a_count = std::min(region_a_count_ - offset, count);
                    memcpy(region_a_start_ + offset, data, region_a_count);
                    remain_count -= region_a_count;
                }
                if (remain_count > 0) {
                    try {
                        if (region_b_start_) {
                            size_t region_b_count = std::max(region_b_count_, offset - region_a_count_ + remain_count);
                            if (get_b_space() < region_b_count - region_b_count_)
                                throw ERROR_WRITE_NO_SPACE;;

                            if (remain_count != count) {
                                memcpy(region_b_start_, &data[count - remain_count], remain_count);
                            } else {
                                memcpy(&region_b_start_[offset - region_a_count_], data, remain_count);
                            }
                            region_b_count_ = region_b_count;
                        } else {
                            if (get_a_space() > get_space_before_a()) {
                                if (get_a_space() < remain_count) {
                                    throw ERROR_WRITE_NO_SPACE;
                                }
                                memcpy(region_a_start_ + region_a_count_, &data[count - remain_count], remain_count);
                                region_a_count_ += remain_count;
                            }
                            else {
                                if (get_space_before_a() < remain_count) {
                                    throw ERROR_WRITE_NO_SPACE;;
                                }
                                region_b_start_ = data_.start;
                                memcpy(region_b_start_, data, remain_count);
                                region_b_count_ += remain_count;
                            }
                        }
                    } catch (...) {
                        expand();
                        retry = true;
                    }
                }
            } while (retry);
        }

        void write(const bip& bip, size_t offset, size_t count) {
            if (bip.get_count() < offset + count) {
                throw ERROR_READ_NO_ENOUGH_SPACE;
            }
            if (bip.region_a_count_ > 0) {
                if (bip.region_a_count_ < offset) {
                    offset -= bip.region_a_count_;
                } else {
                    size_t region_a_count = std::min(bip.region_a_count_ - offset, count);
                    write(bip.region_a_start_ + offset, region_a_count, -1);
                    count -= region_a_count;
                    offset = 0;
                }
            }
            if (count > 0) {
                write(bip.region_b_start_ + offset, count, -1);
            }
        }
    private:
        size_t get_a_space() {
            return data_.end - region_a_start_ - region_a_count_;
        }
        size_t get_b_space() {
            return !region_b_start_ ? 0 : region_a_start_ - region_b_start_ - region_b_count_;
        }
        size_t get_space_before_a() {
            return region_a_start_ - data_.start;
        }
        size_t get_space_after_a() {
            return data_.end - region_a_start_ - region_a_count_;
        }

    private:
        void expand() {
            size_t size = data_.get_size();
            if (size == data_.max_size)
                throw ERROR_WRITE_NO_SPACE;

            size <<= 2;
            size = size < data_.max_size ? size : data_.max_size;
            data data(data_.max_size, size);

            size_t count = get_count();
            if (region_a_count_ > 0) {
                memcpy(data.start, region_a_start_, region_a_count_);
                count -= region_a_count_;
            }
            if (count > 0) {
                memcpy(data.start + region_a_count_, region_b_start_, region_b_count_);
            }

            data_ = data;

            region_a_start_ = data_.start;
            region_a_count_ = get_count();
            region_b_start_ = nullptr;
            region_b_count_ = 0;
        } 


    private:
        data data_;

    private:
        uint8 *region_a_start_;
        size_t region_a_count_;

        uint8 *region_b_start_;
        size_t region_b_count_;
};


}

#endif
