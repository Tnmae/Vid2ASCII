#include "sdl.hpp"
#include "VR_State.hpp"
#include "DynTexture.hpp"
#include <chrono>

#define WIDTH 900
#define HEIGHT 600


void print_help(std::string program) {
    std::cout << "Usage: " << program << " [options]\n\n";

    std::cout << "Options:\n";
    std::cout << "  -p, --path <file>     Path to video file\n";
    std::cout << "  -w, --webcam          Use webcam as input\n";
    std::cout << "  -f, --font <file>     Path to TTF font\n";
    std::cout << "  -h, --help            Show this help message\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program << " --path sample.mp4 --font Roboto-Regular.ttf\n";
    std::cout << "  " << program << " --webcam\n";
}


int main(int argc, char *argv[]) {

  std::string title = "Video to ASCII Converter";
  bool webcam = false;
  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  std::string videoPath;
  
  std::string program = "./Video2ASCII";

  if (argc < 2 ) {
    print_help(program);
    return EXIT_SUCCESS;
  }

  for (int i = 1 ; i < argc ; i++ ) {
    std::string arg = argv[i];
    if (arg == "--path" || arg == "-p" || arg == "--p")
      videoPath = argv[i+1];
    else if (arg == "--webcam" || arg == "--w" || arg == "-w" )
      webcam = true;
    else if (arg == "--font" || arg == "--f" || arg == "-f")
      ttfPath = argv[i+1];
    else if (arg == "--help" || arg == "--h" || arg == "-h") {
      print_help(program);
      return EXIT_SUCCESS;
    }
  }

  if (webcam) {
    videoPath = std::string("/dev/video0");
  }


  SDL_App app_instance(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  FontTexture fontCache(ttfPath);
  fontCache.build_glyph_cache(app_instance.getRenderer());
  
  uint8_t* frameBuffer = new uint8_t[app_instance.getWindowWidth() * app_instance.getWindowHeight() * 4];

  VideoReaderState vr_state(videoPath, app_instance.getWindowWidth(), app_instance.getWindowHeight());

  StreamingTexture strmText(app_instance.getRenderer(), app_instance.getWindowWidth(), app_instance.getWindowHeight());

  app_instance.setFPS(vr_state.getVideoFPS());

  app_instance.appStartTimer();

  while (app_instance.getStatus()) {

    app_instance.inputHandler();

    auto frame_start = std::chrono::high_resolution_clock::now();

    if (!vr_state.decode_per_frame(frameBuffer)) {
      exit(1);
    }

    vr_state.video_display_frame(app_instance.getRenderer(), frameBuffer, fontCache.getGlyphCache(), &strmText);

    app_instance.update(frame_start, strmText.getDynTexture());

  }

  delete[] frameBuffer;
  return EXIT_SUCCESS;
}
