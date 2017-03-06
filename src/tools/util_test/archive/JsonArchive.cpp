// Archive.cpp

#include "tools/util_test/Common.h"

#include <util/serialization/NetName.h>
#include <util/serialization/UtcTime.h>
#include <util/serialization/stl/vector.h>
#include <util/archive/JsonIArchive.h>

using namespace util::archive;

using namespace framework::configure;

#include <iostream>
#include <fstream>

struct SinaStream
{
	std::string protocal;
	std::string address;
	std::string transcode_system;
	std::string definition;

	template <
		typename Archive
	>
	void serialize(Archive & ar)
	{
		ar  & SERIALIZATION_NVP(protocal)
			& SERIALIZATION_NVP(address)
			& SERIALIZATION_NVP(transcode_system)
			& SERIALIZATION_NVP(definition);
	}
};


struct SinaData
{
	SinaData()
		: server_time(0)
		, program_id(0)
		, channel_id(0)
	{
	}

	boost::uint64_t server_time;
	boost::uint64_t program_id;
	boost::uint64_t channel_id;
	std::vector<SinaStream> streams;

	template <
		typename Archive
	>
	void serialize(Archive & ar)
	{
		ar  & SERIALIZATION_NVP(server_time)
			& SERIALIZATION_NVP(program_id)
			& SERIALIZATION_NVP(channel_id)
			& SERIALIZATION_NVP(streams);
	}
};

struct SinaPlayInfo
{
	SinaPlayInfo()
		: code(0)
	{

	}

	boost::uint32_t code;
	std::string message;
	SinaData data;

    template <
		typename Archive
	>
	void serialize(Archive & ar)
	{
		ar  & SERIALIZATION_NVP(code)
			& SERIALIZATION_NVP(message)
			& SERIALIZATION_NVP(data);
	}

};

struct SinaJumpInfo
{
	SinaJumpInfo()
		: BWType(0)
	{

	}

	util::serialization::UtcTime  server_time;
	framework::network::NetName server_host;
	framework::network::NetName client_ip;
	boost::uint32_t BWType;

	template <
		typename Archive
	>
	void serialize(Archive & ar)
	{
		ar  & SERIALIZATION_NVP(server_time)
			& SERIALIZATION_NVP(server_host)
			& SERIALIZATION_NVP(client_ip)
			&SERIALIZATION_NVP(BWType);
	}

};

void test_play_json()
{
	std::string path_file_ = "D:\\play.json";
	std::ifstream ifs(path_file_.c_str());
	util::archive::JsonIArchive<> boa(ifs);
	SinaPlayInfo info;

	boa >> info;
	if (boa) {
		ifs.close();
	}
}


void test_jump_json()
{
	std::string path_file_ = "D:\\jump.json";
	std::ifstream ifs(path_file_.c_str());
	util::archive::JsonIArchive<> boa(ifs);
	SinaJumpInfo info;

	boa >> info;
	if (boa) {
		ifs.close();
	}
}



void test_json_archive(Config & conf)
{
	test_play_json();
	test_jump_json();
}

static TestRegister test("json_archive", test_json_archive);
