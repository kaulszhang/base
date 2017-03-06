// Archive.cpp

#include "tools/util_test/Common.h"

#include <util/serialization/VariableNumber.h>
#include <util/serialization/stl/deque.h>
#include <util/archive/TextIArchive.h>
#include <util/archive/TextOArchive.h>
#include <util/archive/BinaryIArchive.h>
#include <util/archive/BinaryOArchive.h>
#include <util/archive/BigEndianBinaryIArchive.h>
#include <util/archive/BigEndianBinaryOArchive.h>
#include <util/archive/ArchiveBuffer.h>

using namespace util::archive;

using namespace framework::configure;

#include <istream>
#include <ostream>
#include <fstream>

struct LocalPlayHistoryItem
{
    LocalPlayHistoryItem(){}
    LocalPlayHistoryItem(const std::string& video_name){}

    std::string video_name_;
    boost::uint32_t video_size_;
    boost::uint32_t bitrate_;
    boost::uint32_t start_pos_;
    boost::uint32_t end_pos_;
    time_t play_start_time_;

    void SetPlayPostion(boost::uint32_t play_pos);
    bool IsValidHistoryItem() const;

    boost::uint32_t GetTotalDuration() const;
    boost::uint32_t GetPlayDuration() const;
    time_t GetPlayStartTime() const;
};

template <typename Archive>
void serialize(Archive & ar, LocalPlayHistoryItem & t)
{
    ar & util::serialization::make_sized<boost::uint16_t>(t.video_name_);
    ar & t.video_size_;
    ar & t.bitrate_;
    ar & t.start_pos_;
    ar & t.end_pos_;
    ar & t.play_start_time_;
}

std::deque<LocalPlayHistoryItem> play_history_deq_;
std::string path_file_ = "playhistory.dat";

void SaveToFile()
{
    std::ofstream ofs(path_file_.c_str(), std::ios::binary);
    util::archive::BinaryOArchive<> boa(ofs);

    boa & play_history_deq_;
    if (boa) {
        //ofs.flush();
        ofs.close();
        //BackupFile(path_file_, path_file_ + BAK_EXT_NAME);
    }
}

bool LoadFromFileImp(const std::string& path_file)
{
    std::ifstream ifs(path_file.c_str(), std::ios::binary);
    util::archive::BinaryIArchive<> bia(ifs);

    BOOST_ASSERT(play_history_deq_.size() == 0);

    play_history_deq_.clear();
    bia & play_history_deq_;

    if (play_history_deq_.size() > 0) {
        return true;
    }

    return bia;
}


void test_archive(Config & conf)
{
    int a = 4;
    int b = 6;
    boost::uint64_t xxx = 3424;

    //framework::system::VariableNumber<> n(3424UL);
    framework::system::VariableNumber<> n(xxx);

    char cbuf[1024];
    ArchiveBuffer<char> buf(cbuf, sizeof(cbuf));

    {
        TextOArchive<char> oa(buf);

        oa << (a);
        oa << (b);

        TextIArchive<char> ia(buf);

        ia >> (a) >> (b);
    }

    {
        BinaryOArchive<char> oa(buf);

        oa << (a) << (b);

        BinaryIArchive<char> ia(buf);

        ia >> (a) >> (b);
    }

    {
        BigEndianBinaryOArchive<> oa(buf);

        oa << a << b << n;

        BigEndianBinaryIArchive<> ia(buf);

        ia >> a >> b >> n;
    }

    {
        LoadFromFileImp( path_file_ );
    }
}

static TestRegister test("archive", test_archive);
