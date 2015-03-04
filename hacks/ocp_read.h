#pragma once

#include <rw_api/hacks/ocp_pop_read_query.h>

namespace rw_api {
	namespace flyspi {

		inline FlyspiCom::Data ocpRead(FlyspiCom& com,
				FlyspiCom::Locator const& loc,
				FlyspiCom::Address const& addr) {
			SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> q(com, loc);

			q.iread(addr);
			auto r = q.commit();
			r.wait();

			do {
				OcpPopReadQuery s(com, loc);
				auto t = s.commit();
				t.wait();
				if( t.isGood() )
					return t[0];
			} while( true );
		}

	}
}

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
