#pragma once

#include <endian.h>

namespace rw_api {

	inline uint16_t hostToBigEndian(uint16_t const& x) {
		return htobe16(x);
	}

	inline uint32_t hostToBigEndian(uint32_t const& x) {
		return htobe32(x);
	}

	inline uint64_t hostToBigEndian(uint64_t const& x) {
		return htobe64(x);
	}

	inline uint16_t bigEndianToHost(uint16_t const& x) {
		return be16toh(x);
	}

	inline uint32_t bigEndianToHost(uint32_t const& x) {
		return be32toh(x);
	}

	inline uint64_t bigEndianToHost(uint64_t const& x) {
		return be64toh(x);
	}


	template<typename T>
	class BigEndianReorder {
		T value;

		public:
		BigEndianReorder()
			:	value(0) {
		}

		BigEndianReorder(T const& v)
			:	value(hostToBigEndian(v)) {
		}

		void operator = (T const& v) {
			value = hostToBigEndian(v);
		}

		operator T () const {
			return bigEndianToHost(value);
		}
	};

}  /* namespace rw_api */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
