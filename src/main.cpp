#include "sdl.hpp"
#include "VR_State.hpp"
#include "DynTexture.hpp"
#include "filters.hpp"

#define WIDTH 1080
#define HEIGHT 720

std::atomic_bool backbuffer_ready = false;
std::atomic_bool running = true;

std::atomic_bool decoded_frame_ready = true;

void frame_decoder(VideoReaderState &vr_state, uint8_t* backBuffer) {
  while (running) {
    if (decoded_frame_ready.load(std::memory_order_acquire)) {
      std::this_thread::yield();
      continue;
    }

    backbuffer_ready.store(false, std::memory_order_release);
    if (!(vr_state.decode_per_frame(backBuffer))) {
      return;
    }

    decoded_frame_ready.store(true, std::memory_order_release);

  }
}

void post_processing_function(uint8_t* backBuffer, int width, int height, int glyph_size) {
  while (running) {
    if (backbuffer_ready.load(std::memory_order_acquire)) {
      std::this_thread::yield();
      continue;
    }

    if (decoded_frame_ready.load(std::memory_order_acquire)) {
      filters::pixelate_frame(backBuffer, width, height, glyph_size);
      backbuffer_ready.store(true, std::memory_order_release);
    }
  }
}

void print_help(std::string program) {
    std::cout << "Usage: " << program << " [options]\n\n";

    std::cout << "Options:\n";
    std::cout << "  -p, --path <file>           Path to video file\n";
    std::cout << "  -w, --webcam                Use webcam as input\n";
    std::cout << "  -d, --device                specify a  webcam device to use, mainly for windows\n";
    std::cout << "  -f, --font <file>           Path to TTF font\n";
    std::cout << "  --framerate <framerate>     to specify framerate for the video to be played\n";
    //std::cout << "  --save                      to save video output\n";
    std::cout << "  -h, --help                  Show this help message\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program << " --path sample.mp4 --font Roboto-Regular.ttf\n";
    std::cout << "  " << program << " --webcam\n";
}


/* Sample line to check if the git push action will work*/

int main(int argc, char *argv[]) {

  std::string title = "Video to ASCII Converter";
  bool webcam = false;
  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  std::string video;
  int framerate = 0;
  
  std::string program = "./Video2ASCII";

  if (argc < 2 ) {
    print_help(program);
    return EXIT_SUCCESS;
  }

  for (int i = 1 ; i < argc ; i++ ) {
    std::string arg = argv[i];
    if (arg == "--path" || arg == "-p")
      video = argv[i+1];
    else if (arg == "--webcam" || arg == "-w" )
      webcam = true;
    else if (arg == "--font" || arg == "-f")
      ttfPath = argv[i+1];
    else if (arg == "--framerate")
      framerate = std::stoi(argv[i+1]);
    else if (arg == "--device" || arg == "-d") {
      std::string temp;
      webcam = true;
      for (int j = i+1 ; j < argc ; j++) {
        std::string localArg = std::string(argv[j]);
        if (localArg[0] == '-') break;
        temp += std::string(" ") + localArg;
      }
      video = temp.substr(1, temp.size());
    }
    else if (arg == "--save") {
      // TO BE IMPLEMENTED
    }
    else if (arg == "--help" || arg == "-h") {
      print_help(program);
      return EXIT_SUCCESS;
    }
  }

  std::vector<uint8_t> frameBuffer(WIDTH*HEIGHT*4);
  std::vector<uint8_t> backBuffer(WIDTH*HEIGHT*4);

  SDL_App app_instance(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  FontTexture fontCache(ttfPath);
  fontCache.build_glyph_cache(app_instance.getRenderer());
  

  VideoReaderState vr_state(video, webcam, app_instance.getWindowWidth(), app_instance.getWindowHeight());
  
  // framerate
  framerate = (framerate) ? framerate : vr_state.getVideoFPS();
  std::cout << "Framerate of the video: " << vr_state.getVideoFPS() << '\n';

  StreamingTexture strmText(app_instance.getRenderer(), app_instance.getWindowWidth(), app_instance.getWindowHeight());

  app_instance.setFPS(framerate);

  app_instance.appStartTimer();

  if (!vr_state.decode_per_frame(backBuffer.data())) {
    return EXIT_FAILURE;
  }


  int frame_count = 0;
  auto start = std::chrono::high_resolution_clock::now();

  // TODO: implement pipelining
  std::thread post_processing_thread(post_processing_function, backBuffer.data(), app_instance.getWindowWidth(), app_instance.getWindowHeight(), PIXEL_SIZE);

  std::thread frame_decoder_thread(frame_decoder, std::ref(vr_state), backBuffer.data());

  while (app_instance.getStatus()) {

    app_instance.inputHandler(running);

    auto frame_start = std::chrono::high_resolution_clock::now();

    if (backbuffer_ready.load(std::memory_order_acquire)) {
      frameBuffer = backBuffer;
      decoded_frame_ready.store(false, std::memory_order_release);
      backbuffer_ready.store(true, std::memory_order_release);
      frame_count++;
    }

    vr_state.video_display_frame(app_instance.getRenderer(), frameBuffer.data(), fontCache.getGlyphCache(), &strmText);

    app_instance.update(frame_start, strmText.getDynTexture());

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration<double>(end - start).count();
    if (time_elapsed >= 1.0f) {
      std::cout << "fps : " << frame_count/time_elapsed << '\n';
      frame_count = 0;
      start = end;
    }
  }

  post_processing_thread.join();
  frame_decoder_thread.join();

  return EXIT_SUCCESS;
}
