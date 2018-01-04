#pragma once

#include <string>
#include <stdexcept>


namespace rw_api {

	class ErrorBase : public std::runtime_error {
		protected:
		std::string m_where;
		std::string m_what;

		public:
		ErrorBase() : runtime_error("") {
		};
		~ErrorBase() throw() {};

		ErrorBase(std::string const& where, std::string const& what)
		    :   runtime_error(what),
		        m_where(where),
		        m_what(what) {
		};

		/*
		const char* what() const {
		    return m_what.c_str();
		};
		*/

		std::string where() const {
		    return m_where;
		};
	};

	struct LogicError : public ErrorBase {
		LogicError(std::string const& where, std::string const& reason)
			:	ErrorBase(where, std::string("Logic error: ") + reason) {
		}
	};

	struct ImplementationError : public ErrorBase {
		ImplementationError(std::string const& where, std::string const& reason)
			:	ErrorBase(where, std::string("Implementation error: ") + reason) {
		}
	};

	struct DeviceError : public ErrorBase {
		DeviceError(std::string const& where, std::string const& reason)
			:	ErrorBase(where, std::string("Device error: ") + reason) {
		}
	};
	
}  /* namespace rw_api */

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
