#pragma once

#include "Scene/Scene.h"

#include <raylib.h>

class Game
{
  public:
    Game() = default;

  private:
    std::vector<Scene> m_Scenes;
};
