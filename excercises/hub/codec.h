#ifndef HUB_CODEC_H
#define HUB_CODEC_H

#include "../../Types.h"
#include "../../Buffer.h"

#include <boost/noncopyable.hpp>

namespace pubsub
{
using std::string;

enum ParseResult
{
 kError,
 kSuccess,
 kContinue,
};

ParseResult parseMessage(Buffer* buf,
                        string* cmd,
                        string* topic,
                        string* content);

}


#endif
