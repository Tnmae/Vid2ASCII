#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

class video {
public:
  void open_video(std::string path);

private:
};

#endif /*VIDEO_HPP*/
