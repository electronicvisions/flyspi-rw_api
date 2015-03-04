#include <rw_api/flyspi_com.h>
//#include <rw_api/functional.h>

#include <iostream>
#include <array>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>
//#include <random>
//#include <algorithm>


inline std::string progress(int cur, int min, int max) {
    std::stringstream strm;
    strm << std::setprecision(2) << std::fixed << (float)cur/(float)(max - min) * 100.0 << " %";
    return strm.str();
}

void rate_test(rw_api::FlyspiCom& com,
        rw_api::FlyspiCom::Locator const& loc,
        rw_api::flyspi::Address const& base,
        unsigned int const size) {
    using namespace std;
    using namespace rw_api;
    using namespace rw_api::flyspi;

    unsigned int const block_size = 16*1024*1024 / sizeof(Data) - 3;

    vector<Data> data(size);
    mt19937 rng{};
    uniform_int_distribution<uint32_t> rand{0,0xffffffff};
    int fail_count = 0;

    cout << "\nFilling local memory (" << sizeof(Data) * data.size() / 1024 << " kiB) ..." << endl;
    for(auto& i : data) {
        i = rand(rng);
    }

    cout << "Transmitting data..." << endl;
    auto t_start = chrono::system_clock::now();
    int sent = 0;
    auto p = begin(data);
    while( p != end(data) ) {
        SdramBlockWriteQuery q{com, loc, block_size};
        q.addr(base + sent);
        
        for(unsigned int i=0; i<block_size && p != end(data); i++, p++) {
            q.iwrite(*p);
        }

        auto r = q.commit();
        r.wait();

        sent += q.counter;

        //cout << "transmitting: " << progress(sent, 0, size) << "   \r" << flush;
    }
    //cout << '\n';

    //auto t_stop = chrono::system_clock::now();
    //auto dur = chrono::duration_cast<chrono::milliseconds>(t_stop - t_start);
    //cout << "Transmitted " << sizeof(Data) * data.size() << " bytes in "
        //<< dur.count() << " ms  rate: " << (float)(sizeof(Data) * data.size())/((float)dur.count() / 1000.0) << " bytes/sec \n";
    
    //cout << "Recveiving data..." << endl;
    int received = 0;
    p = begin(data);
    while( p != end(data) ) {
        auto fetch_sz = ((size-received) < block_size) ? (size-received) : block_size;
        SdramBlockReadQuery q{com, loc, fetch_sz};
        q.addr(base + received);

        auto r = q.commit();
        r.wait();

        for(unsigned int i=0; i<fetch_sz && p != end(data); i++, p++) {
            if( (r[i] != *p) && (fail_count++ < 20) ) {
                cout << "Memory test failed at address " << base + i
                    << " (expected: 0x" << hex << setw(8) << setfill('0') << *p 
                    << " got: 0x" << setw(8) << setfill('0') << r[i] << ")\n" << dec;
            }
        }

        received += q.capacity;

        //cout << "receiving: " << progress(received, 0, size) << "   \r" << flush;
    }

    auto t_stop = chrono::system_clock::now();
    auto dur = chrono::duration_cast<chrono::milliseconds>(t_stop - t_start);
    cout << "Transmitted " << sizeof(Data) * data.size() << " bytes in "
        << dur.count() << " ms  rate (in + out): " << (float)(sizeof(Data) * data.size() * 2)/((float)dur.count() / 1000.0) << " bytes/sec \n";
    //cout << '\n';

    cout << fail_count << " errors occured during rate test" << endl;
}

void async_test(rw_api::FlyspiCom& com,
		rw_api::FlyspiCom::Locator const& loc,
		rw_api::FlyspiCom::Address const& base_addr,
		unsigned int const size) {
    using namespace std;
    using namespace rw_api;
    using namespace rw_api::flyspi;

    //unsigned int const block_size = 16*1024*1024 / sizeof(Data) - 3;
    unsigned int const block_size = 4*1024*1024 / sizeof(Data) - 3;

    vector<Data> data(size);
    vector<Data> res_data(size);
    mt19937 rng{};
    uniform_int_distribution<uint32_t> rand{0,0xffffffff};
    int fail_count = 0;

    cout << "\nFilling local memory (" << sizeof(Data) * data.size() / 1024 << " kiB) ..." << endl;
    for(auto& i : data) {
        i = rand(rng);
    }

	vector<SdramBlockWriteQuery> w_queries;
	vector<SdramBlockReadQuery> r_queries;
	vector<SdramRequest> w_reqs;
	vector<SdramRequest> r_reqs;

	w_queries.reserve(size / block_size +1);
	r_queries.reserve(size / block_size +1);
	w_reqs.reserve(size / block_size +1);
	r_reqs.reserve(size / block_size +1);

    auto t_start = chrono::system_clock::now();

	for(auto p = begin(data); p != end(data); ) {
		w_queries.emplace_back(com, loc, block_size);

		unsigned int d = distance(p, end(data));
		auto remaining = min(block_size, d);

		//cout << remaining << " remaining" << endl;

		auto addr = base_addr + distance(begin(data), p);
		w_queries.back().addr(addr);
		w_queries.back().resize(remaining);
		copy_n(p, remaining, begin(w_queries.back()));
		advance(p, remaining);
		w_reqs.emplace_back(w_queries.back().commit());

		r_queries.emplace_back(com, loc, remaining);

		r_queries.back().addr(addr);
		r_reqs.emplace_back(r_queries.back().commit());
	}

	for(auto& r : w_reqs) {
		r.wait();
	}

	for(auto& r : r_reqs) {
		r.wait();
	}

    auto t_stop = chrono::system_clock::now();
    auto dur = chrono::duration_cast<chrono::milliseconds>(t_stop - t_start);
    cout << "memory test of " << sizeof(Data) * data.size() << " bytes in "
        << dur.count() << " ms  rate (in + out): " 
		<< (float)(sizeof(Data) * data.size() * 2)/((float)dur.count() / 1000.0) 
		<< " bytes/sec \n";

	auto p_res = begin(res_data);
	for(auto& r : r_reqs) {
		//cout << "Copying " << r.size << " words (" << distance(begin(r), end(r)) << ")" << endl;
		p_res = copy(begin(r), end(r), p_res);
	}

	auto cmp = mismatch(begin(res_data), end(res_data), begin(data));
	if( cmp.first != end(res_data) ) {
		cout << "mismatch at position "
			<< distance(begin(res_data), cmp.first) 
			<< " got: " << *cmp.first
			<< " expected: " << *cmp.second
			<< endl;
	}

	for(unsigned int i=0; i<size; i++) {
		//cout << "exp:  " << hex << data[i] << "   got:  " << hex << res_data[i] << '\n';
		if( (data[i] != res_data[i]) && ++fail_count < 20 ) {
			cout << "MISMATCH at " << dec <<  i 
				<< " exp:  " << hex << data[i] 
				<< "   got:  " << hex << res_data[i] << '\n' << dec;
		}
	}
}

/*rw_api::FlyspiCom::Data ocp_fifo_test(rw_api::FlyspiCom& com,
		rw_api::FlyspiCom::Locator const& loc,
		rw_api::FlyspiCom::Address const& addr) {
	SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> q(com, loc);
	PopReadQuery<FlyspiCom, FlyspiCom::OcpChannel> s(com, loc);

	q.iread(addr);
	auto r = q.commit();
	r.wait();

	auto t = s.commit();
	t.wait();

	return t[0];


	typedef SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> ReadQ;
	typedef PopReadQuery<FlyspiCom, FlyspiCom::OcpChannel> PopQ;
	
	MetaQuery<ReadQ, PopQ> metaq(com, loc);
	metaq.iread(addr);
	auto r = metaq.commit();
	r.wait();

	return r[0];
}*/

int main() {
	using namespace std;
	using namespace rw_api;
    using namespace rw_api::flyspi;

	try {
		FlyspiCom com;
		auto loc = com.locate().chip(0).sdram();
		
		bool ramtest_success = true;
        static unsigned int const ramtest_size = 500-3;

		{
			//BlockWriteQuery<FlyspiCom, FlyspiCom::SdramChannel> q(com, loc, ramtest_size);
			SdramBlockWriteQuery q(com, loc, ramtest_size);

			q.addr(0);
			for(unsigned int i=0; i<ramtest_size; i++) {
				q.iwrite(i);
			}
			auto req = q.commit();
			req.wait();
		}

		{
			//BlockReadQuery<FlyspiCom, FlyspiCom::SdramChannel> q(com, loc, ramtest_size);
			SdramBlockReadQuery q(com, loc, ramtest_size);

			q.addr(0);
			auto req = q.commit();

			req.wait();
			for(unsigned int i=0; i<ramtest_size; i++) {
				if( req[i] == i )
					cout << '.';
				else {
					cout << '#' << "(@" << i << " exp: " << i << " got: " << req[i] << '\n';
					ramtest_success = false;
				}
			}
		}

		cout << "\nRamtest " << (ramtest_success ? "passed" : "failed") << endl;

		// STL iterator adapters not yet available
		/*{
			mt19937 rng{};
			uniform_int_distribution<uint32_t> rand(0,0xffffffff);
			array<uint32_t, ramtest_size> data;

			for(auto& i : data) {
				i = rand(rng);
			}

			// transmitting data
			{
				SdramBlockWriteQuery q(com, loc, ramtest_size);
				q.addr(ramtest_size);
				q.resize(data.size());
				copy(data.begin(), data.end(), begin(q));
				//for(auto& elem : data)
					//q.iwrite(elem);
				q.commit().wait();
			}

			// reading back
			{
				SdramBlockReadQuery q(com, loc, ramtest_size);
				q.addr(ramtest_size);
				auto r = q.commit();
				r.wait();

				auto cmp = mismatch(begin(r), end(r), data.begin());
				if( cmp.first != end(r) ) {
					cout << "Ramtest 2 failed at position "
						<< distance(cmp.first, begin(r)) 
						<< " got: " << *cmp.first
						<< " expected: " << *cmp.second
						<< endl;
				}
			}
		}


		for(int sz = 1*1024*1024; sz < 4*1024*1024; sz += 1024*1024) {
			cout << "testing size: " << sz/1024 * sizeof(Data) << " kiB" << endl;
			rate_test(com, loc, 0, sz);
		}

		for(int sz = 1*1024*1024; sz < 16*1024*1024; sz += 1024*1024) {
			cout << "testing size: " << sz/1024 * sizeof(Data) << " kiB" << endl;
			async_test(com, loc, 0, sz);
		}*/

		//bool ocptest_success = true;
		//array<uint32_t, 3> ocp_addr_w{
			//0x80003000,  // addresses in ADC configuration
			//0x80003001,
			//0x80003002
		//};
		//array<uint32_t, 3> ocp_addr_r{
			//0x00003000,  // addresses in ADC configuration
			//0x00003001,
			//0x00003002
		//};
		//array<uint32_t, 3> expected{
			//0xdeadface,
			//0xaffeaffe,
			//0xbeefbeef
		//};

		//for(int i=0; i<3; i++) {
			////WriteQuery<FlyspiCom, FlyspiCom::OcpChannel> q(com, loc, 3);
			//OcpSingleWriteQuery q(com, loc);

			////q.iwrite(ocp_addr_w[0], expected[0]).iwrite(ocp_addr_w[1], expected[1]).iwrite(ocp_addr_w[2], expected[2]);
			//q.iwrite(ocp_addr_w[i], expected[i]);
			//auto req = q.commit();

			//req.wait();
		//}

		//for(int i=0; i<3; i++) {
			////SingleReadQuery<FlyspiCom, FlyspiCom::OcpChannel> q(com, loc);
			//OcpSingleReadQuery q(com, loc);

			//auto req = q.iread(ocp_addr_r[i]).commit();
			//req.wait();

			//if( req[0] == expected[i] )
				//cout << ".";
			//else {
				//cout << "#" << "(@" << ocp_addr_r[i] << " exp: " << expected[i] << " got: " << req[0] << '\n';
				//ocptest_success = false;
			//}
		//}

		//cout << "\nOCP test " << (ocptest_success ? "passed" : "failed") << endl;

		//for(int i=0; i<100; i++) {
			//cout << hex << ocpRead(com, loc, i + 0x3000) << dec << '\n';
		//}
	
		//ocpWrite(com, loc, 0x3000, 0xaffe);

		//for(int i=0; i<100; i++) {
			//cout << hex << ocpRead(com, loc, i + 0x3000) << dec << '\n';
		//}

		ocpWrite(com, loc, 0xc9f, 0xaffe);
		cout << ocpRead(com, loc, 0x805) << std::endl;
		//for(int i=0; i<0x1000; i++) {
			//auto res = ocpRead(com, loc, i);
			//if( res != 0 ) {
				//cout << "found something in " << i << " : " << hex << res << dec << endl;
			//}
		//}

	} catch(ErrorBase const& err) {
		cerr << "Exception occured in '" << err.where()
			<< "' reason:\n" << err.what()
			<< endl;
		return 1;
	}

	return 0;
}

// vim: noexpandtab ts=4 sw=4 softtabstop=0 nosmarttab:
