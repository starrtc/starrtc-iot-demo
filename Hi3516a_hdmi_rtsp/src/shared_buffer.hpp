#ifndef _SHARED_BUFFER_HPP
#define _SHARED_BUFFER_HPP

typedef unsigned char byte;

namespace axis{

class buffer{
private:
	byte * data_;
	size_t capacity_;
	size_t used_size_;

private:
	buffer(const buffer&);
	buffer& operator=(const buffer&);
	void reserve(size_t new_size){
		byte * tmp = (byte *)malloc(new_size);
		if(data_){
			memcpy(tmp, data_, used_size_ > new_size ? new_size : used_size_);
			free(data_);
		}
		capacity_ = new_size;
		data_ = tmp;
	}

public:
	buffer()
		: used_size_(0)
		, capacity_(0)
		, data_(0)
	{
	}

	~buffer()
	{
		if(data_)free(data_);
	}
 
	byte* raw_data(){
		if(data_ == 0){
			return 0;
		}
		return data_;
	}

	size_t size() const{
		return used_size_;
	}

	void resize(size_t new_size){
		if(new_size > capacity_){
			reserve(new_size);
		}
		used_size_ = new_size;
	}
};

class shared_buffer
{
private:
	typedef boost::shared_ptr<buffer> shared_data_ptr;
	shared_data_ptr shared_data_;

public:
	shared_buffer()
		: shared_data_(new buffer())
	{
	}

	byte* raw_data() const{
		return shared_data_->raw_data();
	}

	unsigned size() const{
		return shared_data_->size();
	}

	void resize(size_t new_size){
		shared_data_->resize(new_size);
	}

	void reset(){
		shared_data_.reset(new buffer());
	}
};

};

#endif // _SHARED_BUFFER_HPP
