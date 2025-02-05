#pragma once
#include <atomic>
#include <vector>
#include <map>
#include <mutex>
#include <Poco/AutoPtr.h>
#include <Poco/Channel.h>
#include "ExtendedLogChannel.h"

#ifdef WITH_TEXT_LOG
namespace DB
{
    class TextLog;
}
#endif

namespace DB
{
/// Works as Poco::SplitterChannel, but performs additional work:
///  passes logs to Client via TCP interface
///  tries to use extended logging interface of child for more comprehensive logging
class OwnSplitChannel : public Poco::Channel
{
public:
    /// Makes an extended message from msg and passes it to the client logs queue and child (if possible)
    void log(const Poco::Message & msg) override;

    void setChannelProperty(const std::string& channel_name, const std::string& name, const std::string& value);

    /// Adds a child channel
    void addChannel(Poco::AutoPtr<Poco::Channel> channel, const std::string & name);

#ifdef WITH_TEXT_LOG
    void addTextLog(std::shared_ptr<DB::TextLog> log, int max_priority);
#endif

    void setLevel(const std::string & name, int level);

private:
    void logSplit(const Poco::Message & msg);
    void tryLogSplit(const Poco::Message & msg);

    using ChannelPtr = Poco::AutoPtr<Poco::Channel>;
    /// Handler and its pointer casted to extended interface
    using ExtendedChannelPtrPair = std::pair<ChannelPtr, ExtendedLogChannel *>;
    std::map<std::string, ExtendedChannelPtrPair> channels;

    std::mutex text_log_mutex;

#ifdef WITH_TEXT_LOG
    std::weak_ptr<DB::TextLog> text_log;
    std::atomic<int> text_log_max_priority = -1;
#endif
};

}
