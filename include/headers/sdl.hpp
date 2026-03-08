#ifndef SDL_H
#define SDL_H

#include "FontTexture.hpp"

class SDL_App {
public:
  SDL_App(std::string title, int width, int height, SDL_WindowFlags flag);
  ~SDL_App();
  SDL_AppResult initWindow(std::string title, int width, int height, SDL_WindowFlags flag);
  SDL_AppResult initRenderer();
  Uint32 getWindowHeight();
  Uint32 getWindowWidth();
  SDL_Window* getWindow();
  SDL_Renderer* getRenderer();
  bool getStatus();
  void setTargetFrameTime();
  void update(std::chrono::time_point<std::chrono::high_resolution_clock> frame_start, SDL_Texture* texture);
  void inputHandler();
  void fpsLimiter(std::chrono::time_point<std::chrono::high_resolution_clock> frame_start);
  void fpsCounter();
  void appStartTimer();

private:
  SDL_Window *gWindow = NULL;
  SDL_Renderer *gRenderer = NULL;
  Uint32 width;
  Uint32 height;
  bool running = true;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  int FPS = 30;
  int frame_count = 0;
  double target_frame_time;
  SDL_FRect frameRect;
};

#endif /*SDL_H*/
