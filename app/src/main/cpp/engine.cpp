#include "engine.h"

Engine::Engine() {
    mBackgroundRenderer = new BackgroundRenderer();
};

void Engine::init() {
  mBackgroundRenderer->init();
}

void Engine::drawFrame() {
  mBackgroundRenderer->drawFrame();
}
