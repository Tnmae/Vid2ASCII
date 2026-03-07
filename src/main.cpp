#ifdef __linux

#include "sdl.hpp"
#include "video.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

#define WIDTH 900
#define HEIGHT 600

#define GLYPH_SIZE 10

struct VideoReaderState {
  AVFormatContext *avformat_ctx;
  AVCodecContext* avcodec_ctx;
  int video_stream_idx;
  int width, height;
  AVFrame* av_frame;
  AVPacket* av_packet;
  SwsContext* sws_scaler_ctx;
};

static SDL_Renderer *renderer = nullptr;

constexpr int columns = (int)WIDTH/GLYPH_SIZE;
constexpr int rows = (int)HEIGHT/GLYPH_SIZE;

constexpr int num_iter = rows * columns;

static uint8_t frame[ num_iter ];
static uint8_t back_buffer[ num_iter ];

constexpr char luminosity[] = "@%#*+=-:. ";

std::atomic_bool new_frame_ready = false;
std::atomic_bool running = true;

SDL_AppResult CreateTextTexture(TTF_Font* font, SDL_Texture*& mFontTextTexture, std::string text, SDL_Color fg) {

  SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, fg, 0);

  mFontTextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

  SDL_DestroySurface(textSurface);

  return SDL_APP_CONTINUE;
}

SDL_AppResult RenderText(SDL_Texture *mFontTextTexture, float x, float y, float w, float h) {
  SDL_FRect textRect= {x, y, w, h};
  SDL_RenderTexture(renderer, mFontTextTexture, nullptr, &textRect);
  return SDL_APP_CONTINUE;
}


void generate_frame(std::mt19937 &generator, std::uniform_int_distribution<std::mt19937::result_type> &rng) {
  /*
   This function will take raw pixel data and convert a luminosity map from it, and then from that generate a back buffer containing ASCII character corresponding to it's respective luminosity
   */
  for ( int y = 0 ; y < num_iter ; y++ ) {
    back_buffer[y] = rng(generator);
  }
}

void generate_glyph_texture(SDL_Texture *& glyph_texture, TTF_Font* font, char* glyph) {
  SDL_Surface *textSurface = TTF_RenderText_Blended_Wrapped(font, glyph, 0, SDL_Color{255, 255, 255, 255}, 0);
  glyph_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_DestroySurface(textSurface);
}

void build_glyph_cache(std::vector<SDL_Texture* > &textures, TTF_Font *font) {
  for (int i = 0 ; i < 10 ; i++ ) {
    char glyph[2] = { luminosity[i] , '\0'};
    generate_glyph_texture(textures[i], font, glyph);
  }
}

void RenderFrame(std::vector<SDL_Texture* > &textures, TTF_Font *font) {
  for (int y = 0 ; y < rows ; y++ ) {
    int row_num = y * columns;
    for (int x = 0 ; x < columns ; x++) {
      RenderText(textures[frame[row_num + x]], (float)x*GLYPH_SIZE, (float)y*GLYPH_SIZE, GLYPH_SIZE, GLYPH_SIZE);
    }
  }
}

void worker_function() {
  std::random_device rdev;
  std::mt19937 generator(rdev());
  std::uniform_int_distribution<std::mt19937::result_type> range(0, 9);

  while (running) {
    generate_frame(generator, range);

    new_frame_ready.store(true, std::memory_order_release);

    while (new_frame_ready.load(std::memory_order_acquire) && running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

void pixelate_frame() {

}

void decode_video() {
  // opening video file
   

  // processing on raw pixel data
  pixelate_frame();

  // coneverting the frame into ascii 
  //generate_frame();
}

bool video_reader_open(VideoReaderState* state, const char* videoPath) {
  auto& width = state->width;
  auto& height = state->height;
  auto& video_stream_idx = state->video_stream_idx;
  auto& localfmtCtx = state->avformat_ctx;
  auto& localCodecCtx = state->avcodec_ctx;
  auto& localScalerCtx = state->sws_scaler_ctx;
  auto& frame = state->av_frame;
  auto& packet = state->av_packet;

  localfmtCtx = avformat_alloc_context();

  if (!localfmtCtx) {
    std::cerr << "error allocating context for AVFormat\n";
    return false;
  }else {
    std::cout << "format context created successfully\n";
  }

  if (avformat_open_input(&localfmtCtx, videoPath, NULL, NULL) != 0) {
    std::cerr << "error opening video file\n";
    return false;
  }else {
    std::cout << "video frame loaded successfully\n";
  }

  if (avformat_find_stream_info(localfmtCtx, NULL) < 0) {
    std::cerr << "could not find stream info\n";
    return false;
  }else {
    std::cout << "stream info found\n";
  }

  video_stream_idx = -1;

  AVCodecParameters* av_codec_params;
  const AVCodec* avcodec;
    
  for (int i = 0 ; i < localfmtCtx->nb_streams ; i++ ) {
    AVStream* av_stream = localfmtCtx->streams[i];

    av_codec_params = av_stream->codecpar;
    avcodec = avcodec_find_decoder(av_codec_params->codec_id);

    if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_idx = i;
      width = av_codec_params->width;
      height = av_codec_params->height;
      break;
    }
  }

  if (video_stream_idx == -1 ) {
    std::cerr << "could not find a video stream\n";
    return false;
  }else {
    std::cout << "video stream index is at: " << video_stream_idx << '\n';
  }

  localCodecCtx = avcodec_alloc_context3(avcodec);

  if (!localCodecCtx) {
    std::cerr << "could not allocate context for codec\n";
    return false;
  }else {
    std::cout << "context for codec allocated\n";
  }

  if (avcodec_parameters_to_context(localCodecCtx, av_codec_params) < 0) {
    std::cerr << "could not initialize AVCodec context\n";
    return false;
  }else {
    std::cout << "Initialized AVCodec context\n";
  }

  if (avcodec_open2(localCodecCtx, avcodec, NULL)) {
    std::cerr << "Could not open context\n";
    return false;
  }else {
    std::cout << "Opened context successfully\n";
  }

  frame = av_frame_alloc();
  if (!frame) {
    std::cerr << "could not allocate frame\n";
    return false;
  }else {
    std::cout << "allocated frame\n";
  }

  packet = av_packet_alloc();
  if (!packet) {
    std::cerr << "could not allocate packet\n";
    return false;
  }else {
    std::cout << "allocated packet\n";
  }
  
  return true;
}

bool decode_per_frame(VideoReaderState* state, uint8_t* frameBuffer) {
  auto& width = state->width;
  auto& height = state->height;
  auto& video_stream_idx = state->video_stream_idx;
  auto& localfmtCtx = state->avformat_ctx;
  auto& localCodecCtx = state->avcodec_ctx;
  auto& localScalerCtx = state->sws_scaler_ctx;
  auto& frame = state->av_frame;
  auto& packet = state->av_packet;

  int response;
  while (av_read_frame(localfmtCtx, packet) >= 0) {
    if (packet->stream_index != video_stream_idx) {
      av_packet_unref(packet);
      continue;
    }else {
      response = avcodec_send_packet(localCodecCtx, packet);
      if (response < 0) {
        std::cerr << "failed to decode packet: " << av_err2str(response) << '\n';
        return false;
      }

      response = avcodec_receive_frame(localCodecCtx, frame);
      if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        continue;
      }else if (response < 0 ) {
        std::cerr << "failed to decode packet : " << av_err2str(response) << '\n';
        return false;
      }

      localScalerCtx = sws_getContext(width, height, localCodecCtx->pix_fmt, WIDTH, HEIGHT, AV_PIX_FMT_RGB0, SWS_FAST_BILINEAR, NULL, NULL, NULL);

      if (!localScalerCtx) {
        std::cerr << "error creating scaler context\n";
        return false;
      }

      av_packet_unref(packet);
      break;
    }
  }

  uint8_t* dest[4] = {frameBuffer, NULL, NULL, NULL};
  int dest_linesize[4] = {WIDTH * 4, 0, 0, 0};

  sws_scale(localScalerCtx, frame->data, frame->linesize, 0, frame->height, dest, dest_linesize);

  return true;
}

void video_reader_close(VideoReaderState* state) {
  sws_free_context(&state->sws_scaler_ctx);
  avformat_close_input(&state->avformat_ctx);
  avformat_free_context(state->avformat_ctx);
  av_frame_free(&state->av_frame);
  av_packet_free(&state->av_packet);
  avcodec_free_context(&state->avcodec_ctx);

}

void pixelate_frame(uint8_t* frameBuffer) {
  for (int y = 0 ; y < HEIGHT; y = y + GLYPH_SIZE) {
    for (int x = 0 ; x < WIDTH ; x = x + GLYPH_SIZE ) {
      double avg = 0;
      for (int i = 0 ; i < GLYPH_SIZE ; i++ ) {
        for (int j = 0 ; j < GLYPH_SIZE ; j++ ) {
          uint8_t red = frameBuffer[( (y + i) * WIDTH + (x + j) ) * 4 ];
          uint8_t green = frameBuffer[( (y + i) * WIDTH + (x + j) )  * 4 + 1 ];
          uint8_t blue = frameBuffer[( (y + i) * WIDTH + (x + j) ) * 4 + 2 ];
          avg += (red + blue + green)/3;
        }
      }
      avg = avg / (GLYPH_SIZE * GLYPH_SIZE);
      for (int i = 0 ; i < GLYPH_SIZE ; i++ ) {
        for (int j = 0 ; j < GLYPH_SIZE ; j++ ) {
          frameBuffer[( (y + i) * WIDTH + (x + j) ) * 4 ] = avg;
          frameBuffer[( (y + i) * WIDTH + (x + j) ) * 4 + 1 ] = avg;
          frameBuffer[( (y + i) * WIDTH + (x + j) ) * 4  + 2 ] = avg;
        }
      }
    }
  }
}

void video_display_frame(VideoReaderState* state, uint8_t* frameBuffer, std::vector<SDL_Texture* > glyph_texture) {
  pixelate_frame(frameBuffer);
  for (int y = 0 ; y < HEIGHT ; y = y + GLYPH_SIZE ) {
    for (int x = 0 ; x < WIDTH ; x = x + GLYPH_SIZE ) {
      RenderText(glyph_texture[9 - ((float)frameBuffer[ y * WIDTH * 4 + x * 4] / 255.0f) * 9], (float)x , (float)y, GLYPH_SIZE, GLYPH_SIZE);
    }
  }
}

int main(int argc, char *argv[]) {

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed to initialize with error: " << SDL_GetError() << "\n";
    return 1;
  }else {
    std::cout << "SDL3 Initialized successfully\n";
  }

  if (!TTF_Init()) {
    std::cerr << "SDL_Ttf failed to initialize with error: " << SDL_GetError() << "\n";
    return 2;
  }else {
    std::cout << "SDL_Ttf Initliazed successfully\n";
  }

  SDL_Window *window = SDL_CreateWindow("Video to ASCII converted", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  if (!window) {
    std::cerr << "Error creating window : " << SDL_GetError() << '\n';
    return 3;
  }else {
    std::cout << "Window created successfully\n";
  }

  renderer = SDL_CreateRenderer(window, nullptr);

  if (!renderer) {
    std::cerr << "Error creating renderer : " << SDL_GetError() << '\n';
    return 4;
  }else {
    std::cout << "Renderer created successfully\n";
  }

  bool running1 = true;
  SDL_Event event;

  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  
  TTF_Font* font = TTF_OpenFont(ttfPath.c_str(), 60);

  if (!font) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return 5;
  }else {
    std::cout << "Font loaded successfully\n";
  }

  SDL_Color fg = {255, 255, 255, 255};

  std::vector<SDL_Texture* > glyph_texture(10);
  build_glyph_cache(glyph_texture, font);

  auto start = std::chrono::high_resolution_clock::now();
  int frame_count = 0;

  //std::thread worker_thread(worker_function);

  uint8_t* frameBuffer = new uint8_t[WIDTH * HEIGHT * 4];
  int width, height;
  std::string videoPath("../resources/thanos.mp4");
  VideoReaderState state;

  if (!video_reader_open(&state, videoPath.c_str())) {
    std::cerr << "error opening video\n";
    return EXIT_FAILURE;
  }

  if (!decode_per_frame(&state, frameBuffer)) {
    std::cerr << "error decoding frame\n";
  }

  while (running1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        //running.store(false, std::memory_order_release);
        //worker_thread.join();
        running1 = false;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    //if (new_frame_ready.load(std::memory_order_acquire)) {
    //  memcpy(frame, back_buffer, sizeof(frame));
    //  new_frame_ready.store(false);
    //}

    if (!decode_per_frame(&state, frameBuffer)) {
      std::cerr << "error decoding frame\n";
      return EXIT_FAILURE;
    }

    video_display_frame(&state, frameBuffer, glyph_texture);
  
    auto end = std::chrono::high_resolution_clock::now();
    frame_count++;
    double time_elapsed = std::chrono::duration<double>(end-start).count();
    if (time_elapsed > 1.0) {
      std::cout << "fps: " << frame_count/time_elapsed << '\n';
      frame_count = 0;
      start = end;
    }

    SDL_RenderPresent(renderer);

  }


  video_reader_close(&state);

  delete[] frameBuffer;
  for (int i = 0 ; i < 10 ; i++ ) {
    SDL_DestroyTexture(glyph_texture[i]);
  }
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;

}


#elif __WIN32__

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

// FFmpeg headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main(int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("SDL3 + SDL3_image + FFmpeg Test", 800,
                                        600, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr );

  // ---------------- Test SDL3_image ----------------
  SDL_Surface *surface = IMG_Load("../resources/sample.png"); // adjust path
  if (!surface) {
    std::cerr << "IMG_Load failed: " << SDL_GetError() << "\n";
  } else {
    std::cout << "SDL3_image loaded PNG successfully!\n";
    SDL_DestroySurface(surface);
  }

  // ---------------- Test FFmpeg ----------------
  const char *videoFile = "../resources/sample.mp4"; // adjust path
  AVFormatContext *fmtCtx = nullptr;
  if (avformat_open_input(&fmtCtx, videoFile, nullptr, nullptr) < 0) {
    std::cerr << "Could not open video file: " << videoFile << "\n";
  } else {
    std::cout << "FFmpeg opened video successfully!\n";
    avformat_close_input(&fmtCtx);
  }

  // ---------------- Show a blank SDL window ----------------
  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // In a full app, you would decode video frames into a texture here

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

#else

#include <format>
#include <iostream>
#include <print>

int main() {
  std::cout << "this will run on others" << std::endl;
  return EXIT_SUCCESS;
}

#endif
