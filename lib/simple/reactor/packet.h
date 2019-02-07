#ifndef LIB_REACTOR_PACKET_H
#define LIB_REACTOR_PACKET_H

#include "buffer/buffer.h"

namespace simple::reactor {
    class connection;

    namespace packet {

        enum {
            ERROR_PACKET_READER_NO_SPACE,
        };

        class base {
            public:
                base(buffer::buffer<>& buffer): buffer_(buffer) {
                }
            protected:
                buffer::buffer<>& buffer_;
        };

        class reader: public base {
            friend class writer;
            public:
                reader(buffer::buffer<>& buffer): base(buffer) {
                    head_ = tail_ = 0;
                    buffer_.read(length_, head_);
                    tail_ += sizeof(uint32);
                }
                reader(reader& reader): base(reader.buffer_) {
                    head_ = tail_ = reader.tail_;
                    buffer_.read(length_, head_);
                    reader.tail_ += length_ + sizeof(uint32);
                    tail_ += sizeof(uint32);
                }
            public:
                template <class type> reader& operator >> (type& value) {
                    auto old_count = buffer_.get_count();
                    buffer_.read(value, tail_);
                    tail_ += buffer_.get_count() - old_count;
                    return *this;
                }
            public:
                uint32 length() const {
                    return length_;
                }

            private:
                uint32 head_;
                uint32 tail_;
                uint32 length_;
        };


        class writer: public base {
            friend class simple::reactor::connection;

            public:
                writer(buffer::buffer<>& buffer): base(buffer), length_(0), writer_(nullptr) {
                    head_ = buffer_.get_count();
                    buffer_.write(uint32(-1));
                }
                writer(writer& writer): base(writer.buffer_), length_(0), writer_(&writer) {
                    head_ = writer.head_ + writer.length_ + sizeof(uint32);
                    buffer_.write(uint32(-1));
                }
                writer(const writer& writer): base(writer.buffer_) {
                    head_ = writer.head_;
                    length_ = writer.length_;
                    writer_ = writer.writer_;
                }
            public:
                ~writer() {
                    uint32 length;
                    if (buffer_.get_count() > head_) {
                        buffer_.read(length, head_);
                        if (length == uint32(-1)) {
                            buffer_.remove_from_back(sizeof(uint32) + length_);
                        }
                    }
                }

            public:
                uint32 length() const {
                    return length_;
                }

                void flush() {
                    buffer_.write(length_, head_);
                    if (writer_) {
                        writer_->length_ += length_ + sizeof(uint32);
                    }
                }
            public:
                template <class type> writer& operator << (const type& value) {
                    auto old_count = buffer_.get_count();
                    buffer_.write(value);
                    length_ += buffer_.get_count() - old_count;
                    return *this;
                }

                writer& operator << (const reader& reader) {
                    const auto length = reader.length_ + sizeof(uint32) - reader.tail_;
                    buffer_.write(reader.buffer_, reader.tail_, length);
                    length_ += length;
                    return *this;
                }

            private:
                uint32 head_;
                uint32 length_;

            private:
                writer *writer_;
        };
    };
}


#endif
