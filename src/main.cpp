#include "sdl.hpp"
#include "VR_State.hpp"
#include "DynTexture.hpp"
#include "filters.hpp"
#include <condition_variable>

#define WIDTH 1920
#define HEIGHT 1080

std::atomic_bool backbuffer_ready = false;
std::atomic_bool running = true;

std::atomic_bool decoded_frame_ready = true;


struct PipelineState {
    std::mutex mtx;
    std::condition_variable cv_decode;
    std::condition_variable cv_process;

    bool decoded_ready = false;
    bool processed_ready = false;
};


void frame_decoder(VideoReaderState &vr_state,
                   std::vector<uint8_t> &decodeBuffer,
                   PipelineState &state)
{
    while (running) {

        std::unique_lock lock(state.mtx);

        state.cv_decode.wait(lock, [&]{
            return !state.decoded_ready || !running;
        });

        if (!running) return;

        lock.unlock();

        if (!vr_state.decode_per_frame(decodeBuffer.data()))
            return;

        lock.lock();

        state.decoded_ready = true;

        lock.unlock();
        state.cv_process.notify_one();
    }
}


void post_processing_function(std::vector<uint8_t> &decodeBuffer,
                              std::vector<uint8_t> &processBuffer,
                              int width,
                              int height,
                              int glyph_size,
                              PipelineState &state)
{
    while (running) {

        std::unique_lock lock(state.mtx);

        state.cv_process.wait(lock, [&]{
            return state.decoded_ready || !running;
        });

        if (!running) return;

        processBuffer.swap(decodeBuffer);

        state.decoded_ready = false;

        lock.unlock();
        state.cv_decode.notify_one();

        filters::pixelate_frame(processBuffer.data(), width, height, glyph_size);

        lock.lock();
        state.processed_ready = true;
        lock.unlock();
    }
}



void print_help(std::string program) {
    std::cout << "Usage: " << program << " [options]\n\n";

    std::cout << "Options:\n";
    std::cout << "  -p, --path <file>           Path to video file\n";
    std::cout << "  -w, --webcam                Use webcam as input\n";
    std::cout << "  -f, --font <file>           Path to TTF font\n";
    std::cout << "  --framerate <framerate>     to specify framerate for the video to be played\n";
    //std::cout << "  --save                      to save video output\n";
    std::cout << "  -h, --help                  Show this help message\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program << " --path sample.mp4 --font Roboto-Regular.ttf\n";
    std::cout << "  " << program << " --webcam\n";
}


int main(int argc, char *argv[]) {

  std::string title = "Video to ASCII Converter";
  bool webcam = false;
  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  std::string videoPath;
  int framerate = 0;
  
  std::string program = "./Video2ASCII";

  if (argc < 2 ) {
    print_help(program);
    return EXIT_SUCCESS;
  }

  for (int i = 1 ; i < argc ; i++ ) {
    std::string arg = argv[i];
    if (arg == "--path" || arg == "-p")
      videoPath = argv[i+1];
    else if (arg == "--webcam" || arg == "-w" )
      webcam = true;
    else if (arg == "--font" || arg == "-f")
      ttfPath = argv[i+1];
    else if (arg == "--framerate")
      framerate = std::stoi(argv[i+1]);
    else if (arg == "--help" || arg == "-h") {
      print_help(program);
      return EXIT_SUCCESS;
    }
  }

  if (webcam) {
    videoPath = std::string("/dev/video0");
  }

  std::vector<uint8_t> decodeBuffer(WIDTH*HEIGHT*4);
  std::vector<uint8_t> processBuffer(WIDTH*HEIGHT*4);
  std::vector<uint8_t> frameBuffer(WIDTH*HEIGHT*4);

  SDL_App app_instance(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  FontTexture fontCache(ttfPath);
  fontCache.build_glyph_cache(app_instance.getRenderer());
  

  VideoReaderState vr_state(videoPath, app_instance.getWindowWidth(), app_instance.getWindowHeight());
  
  // framerate
  framerate = (framerate) ? framerate : vr_state.getVideoFPS();
  std::cout << "Framerate of the video: " << vr_state.getVideoFPS() << '\n';

  StreamingTexture strmText(app_instance.getRenderer(), app_instance.getWindowWidth(), app_instance.getWindowHeight());

  app_instance.setFPS(framerate);

  app_instance.appStartTimer();

  if (!vr_state.decode_per_frame(decodeBuffer.data())) {
    return EXIT_FAILURE;
  }


  int frame_count = 0;
  auto start = std::chrono::high_resolution_clock::now();

  // TODO: implement pipelining
  PipelineState pipeline;
  
  std::thread decoder_thread(
      frame_decoder,
      std::ref(vr_state),
      std::ref(decodeBuffer),
      std::ref(pipeline)
  );
  
  std::thread filter_thread(
      post_processing_function,
      std::ref(decodeBuffer),
      std::ref(processBuffer),
      WIDTH,
      HEIGHT,
      PIXEL_SIZE,
      std::ref(pipeline)
  );

  while (app_instance.getStatus()) {

    app_instance.inputHandler(running);

    auto frame_start = std::chrono::high_resolution_clock::now();


    if (pipeline.processed_ready)
    {
        frameBuffer.swap(processBuffer);
    
        pipeline.processed_ready = false;
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

  filter_thread.join();
  decoder_thread.join();

  return EXIT_SUCCESS;
}
