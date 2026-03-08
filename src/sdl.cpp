#include "../include/headers/sdl.hpp"

SDL_App::SDL_App(std::string title, int width, int height, SDL_WindowFlags flag) {

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed to initialize with error: " << SDL_GetError() << "\n";
    SDL_ClearError();
    exit(1);
  }else {
    std::cout << "SDL3 Initialized successfully\n";
  }

  if (SDL_App::initWindow(title, width, height, flag) == SDL_APP_FAILURE) {
    exit(1);
  };

  if (SDL_App::initRenderer() == SDL_APP_FAILURE) {
    exit(1);
  };

  SDL_App::frameRect = {0, 0, (float)width, (float)height};

}

SDL_App::~SDL_App() {
  if (gRenderer) {
    SDL_DestroyRenderer(gRenderer);
  }
  if (gWindow) {
    SDL_DestroyWindow(gWindow);
  }
  SDL_Quit();
}

SDL_AppResult SDL_App::initWindow(std::string title, int width, int height, SDL_WindowFlags flag) {
  
  SDL_App::width = width;
  SDL_App::height = height;
  SDL_App::gWindow = SDL_CreateWindow(title.c_str(), width, height, flag);

  if (!gWindow) {
    std::cerr << "Error creating window : " << SDL_GetError() << '\n';
    return SDL_APP_FAILURE;
  }else {
    std::cout << "Window created successfully\n";
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_App::initRenderer() {
  
  SDL_App::gRenderer = SDL_CreateRenderer(gWindow, nullptr);

  if (!gRenderer) {
    std::cerr << "Error creating renderer : " << SDL_GetError() << '\n';
    SDL_APP_FAILURE;
  }else {
    std::cout << "Renderer created successfully\n";
  }
  return SDL_APP_CONTINUE;
}

void SDL_App::update(std::chrono::time_point<std::chrono::high_resolution_clock> frame_start, SDL_Texture* texture) {
  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
  SDL_RenderClear(gRenderer);
  SDL_RenderTexture(gRenderer, texture, NULL, &(SDL_App::frameRect));
  SDL_RenderPresent(gRenderer);
  SDL_App::fpsLimiter(frame_start);
  SDL_App::fpsCounter();
}

void SDL_App::inputHandler() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      SDL_App::running = false;
    }
  }
}

void SDL_App::appStartTimer() {
  SDL_App::start = std::chrono::high_resolution_clock::now();
}

void SDL_App::setTargetFrameTime() {
  SDL_App::target_frame_time = 1000.0f/(SDL_App::FPS);
}

void SDL_App::fpsLimiter(std::chrono::time_point<std::chrono::high_resolution_clock> frame_start) {
  auto frame_end = std::chrono::high_resolution_clock::now();
  double frame_time = std::chrono::duration<double, std::milli>(frame_end - frame_start).count();

  if (frame_time < SDL_App::target_frame_time) {
    SDL_Delay(SDL_App::target_frame_time - frame_time);
  }

}

void SDL_App::fpsCounter() {
  SDL_App::frame_count++;
  auto end = std::chrono::high_resolution_clock::now();
  double time_elapsed = std::chrono::duration<double>(end-SDL_App::start).count();
  if (time_elapsed > 1.0) {
    std::cout << "fps: " << frame_count/time_elapsed << '\n';
    frame_count = 0;
    start = end;
  }

}

bool SDL_App::getStatus() {
  return SDL_App::running;
}

Uint32 SDL_App::getWindowHeight() {
  return SDL_App::height;
}

Uint32 SDL_App::getWindowWidth() {
  return SDL_App::width;
}

SDL_Renderer* SDL_App::getRenderer() {
  return SDL_App::gRenderer;
}

SDL_Window* SDL_App::getWindow() {
  return SDL_App::gWindow;
}
