#pragma once

#include <memory>

#include <lua.hpp>

class Scene;

class App
{
  public:
    App();
    ~App();

    bool Initialize();
    int  Run() const;

  private:
    bool InitializeLua();
    bool LoadGame() const;

    void Shutdown();

  private:
    lua_State* m_L = nullptr;

    std::unique_ptr<Scene> m_Scene;
};
