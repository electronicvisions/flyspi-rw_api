#pragma once

//#include <iostream>

namespace rw_api {
	// forward declarations
	template<typename Com, typename Channel> class Request;
}


namespace std {

	// forward declarations
	template<typename Com, typename Channel>
	typename Com::BufferPtr begin(rw_api::Request<Com, Channel>& req);

	template<typename Com, typename Channel>
	typename Com::BufferPtr end(rw_api::Request<Com, Channel>& req);

}


namespace rw_api {


	/* XXX if the user is not storing the Request object returned by 
	 * Query::commit() the response buffer may get freed before the Com 
	 * implementation has written to it (for asynchronous communication).
	 * In this case BufferPtr should be ref-counted or Com::free() smart enough
	 * to recognize the situation. */
	template<typename Com, typename Channel>
	class Request {
		friend typename Com::BufferPtr std::begin<Com, Channel>(Request& req);
		friend typename Com::BufferPtr std::end<Com, Channel>(Request& req);

		Com& com;
		typename Com::BufferPtr buf;
		Channel channel;
		typename Com::RequestHandle handle;

		public:
		unsigned int const length;

		Request(Com& com,
				typename Com::BufferPtr const& buf,
				unsigned int const length,
				typename Com::RequestHandle& handle)
			:	com(com),
				buf(buf),
				handle(std::move(handle)),
				length(length)
		{
			//std::cout << "Request constructor " << this << std::endl;
		}

		Request(Request&& tmp)
			:	com(tmp.com),
				buf(tmp.buf),
				handle(std::move(tmp.handle)),
				length(tmp.length) {
			tmp.buf = nullptr;
			//std::cout << "Request move constructor " << this << std::endl;
		}

		Request(Request const&) = delete;
		Request& operator = (Request const&) = delete;

		Request& operator = (Request&& tmp) {
			if( this != &tmp ) {
				if( !com.hasCompleted(handle) )
					wait();

				if( buf != nullptr )
					com.free(buf);

				com = tmp.com;
				buf = tmp.buf;
				channel = tmp.channel;
				handle = std::move(tmp.handle);
				const_cast<unsigned int&>(length) = tmp.length;

				tmp.buf = nullptr;
				const_cast<unsigned int&>(tmp.length) = 0;
			}

			return *this;
		}

		~Request() {
			if( !com.hasCompleted(handle) )
				wait();

			//std::cout << "Request destructor " << this << std::endl;

			if( buf != nullptr )  // could have been moved away
				com.free(buf);
		}

		void wait() {
			//std::cout << "Request wait " << this << std::endl;
			com.waitRequest(handle);
		}

		typename Com::Data& operator [] (unsigned int const& index) {
			return channel.extract(buf, index);	
		}

		typename Com::Data const& operator [] (unsigned int const& index) const {
			return channel.extract(buf, index);	
		}

		bool isGood() {
			return channel.isGood(buf);
		}

		unsigned int size() const {
			return length;
		}
	};


}  /* namespace rw_api */


namespace std {

	template<typename Com, typename Channel>
	typename Com::BufferPtr begin(rw_api::Request<Com, Channel>& req) {
		return req.buf + req.channel.header_size;
	}

	template<typename Com, typename Channel>
	typename Com::BufferPtr end(rw_api::Request<Com, Channel>& req) {
		return req.buf + req.channel.header_size + req.length;
	}

}

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
