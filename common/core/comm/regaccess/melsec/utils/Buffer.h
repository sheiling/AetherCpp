#ifndef MELSEC_UTILS_BUFFER_H
#define MELSEC_UTILS_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace common::regaccess {
namespace mc {
	class Buffer {
	public:

		explicit Buffer(size_t capacity);
		explicit Buffer(const std::vector<uint8_t>& data);
		Buffer();

		Buffer& operator << (uint8_t val);
		Buffer& operator >> (uint8_t& val);

		Buffer& operator << (int8_t val);
		Buffer& operator >> (int8_t& val);

		Buffer& operator << (uint16_t val);
		Buffer& operator >> (uint16_t& val);

		Buffer& operator << (int16_t val);
		Buffer& operator >> (int16_t& val);

		Buffer& operator << (uint32_t val);
		Buffer& operator >> (uint32_t& val);

		Buffer& operator << (int32_t val);
		Buffer& operator >> (int32_t& val);

		Buffer& operator << (uint64_t val);
		Buffer& operator >> (uint64_t& val);

		Buffer& operator << (int64_t val);
		Buffer& operator >> (int64_t& val);

		Buffer& operator << (float val);
		Buffer& operator >> (float & val);

		Buffer& operator << (double val);
		Buffer& operator >> (double& val);

		Buffer& operator << (const std::vector<uint8_t>& val);
		Buffer& operator >> (std::vector<uint8_t>& val);

		Buffer& operator << (const std::vector<uint16_t>& val);
		Buffer& operator >> (std::vector<uint16_t>& val);

		std::vector<uint8_t> data() const { return _buffer; }
		size_t size() const { return _buffer.size(); }
		size_t pos() const { return _position; }
		bool isValid() const { return _position <= _buffer.size(); }
		bool empty() const  { return  _position >= _buffer.size(); }
	private:
		std::vector<uint8_t> _buffer;
		size_t _position;
	};
} // end of namespace mc
} // end of namespace common::regaccess

#endif 
