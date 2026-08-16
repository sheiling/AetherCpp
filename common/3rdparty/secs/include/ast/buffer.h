#ifndef _SECS_BUFFER_HH
#define _SECS_BUFFER_HH

#include <cstddef>
#include <cstdint>
#include <vector>

namespace SECS {
	class Buffer {
	public:
		explicit Buffer(size_t capacity);
		explicit Buffer(const std::vector<uint8_t>& data);
		Buffer();

		Buffer& operator << (bool val);
		Buffer& operator >> (bool& val);

		Buffer& operator << (char val);
		Buffer& operator >> (char& val);

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
		void replace(size_t begin, size_t end, const std::vector<uint8_t>& val);
	private:
		std::vector<uint8_t> _buffer;
		size_t _position;
	};
}

#endif 
