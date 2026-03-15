#include "Debug.h"
#include "ECS.hpp"
#include "Engine.h"
#include "GUI/FileDialog.h"
#include "Scene/Scene.h"

#include <entt/entt.hpp>

#include <raygui.h>
#include <raylib.h>

namespace
{
    // --------------------------------------------------------------------------------------------
    // Game Config
    // --------------------------------------------------------------------------------------------

    void LoadPlayer(Scene& s, Engine& e)
    {
        using namespace components;

        // create player entity
        const entt::entity pid = s.Create();

        // load player texture
        Texture2D* tex = e.LoadTexture("circle.png");

        // register player components
        s.RegisterComponent<PlayerController>(
            pid,
            PlayerController{ .moveSpeed = 300.0f, .jumpSpeed = 200.0f });
        s.RegisterComponent<PhysicsBody>(
            pid,
            PhysicsBody{
                .dynamic      = true,
                .grounded     = true,
                .gravityScale = 1.0f,
            });
        s.RegisterComponent<Velocity>(pid, Velocity{ 0.0f, 0.0f });
        s.RegisterComponent<components::Position>(
            pid,
            components::Position{ 0.0f, 0.0f });
        s.RegisterComponent<cmp::Transform>(
            pid,
            cmp::Transform{ 10.0f, 10.0f });
        s.RegisterComponent<Drawable>(
            pid,
            Drawable{ .texture = tex,
                      .srcRect = { 0, 0, 32.0f, 64.0f },
                      .scale   = { 1.0f, 1.0f },
                      .origin  = { 0, 0 },
                      .tint    = WHITE });
    }
} // namespace

// prototyping namespace
namespace proto
{
    class UpdatePlayerSystem : public ISystem
    {
      public:
        UpdatePlayerSystem() = default;

        void OnUpdate(entt::registry& r, float dt) override
        {
            using namespace components;

            auto pView = r.view<
                PlayerController,
                PhysicsBody,
                Sprite,
                Velocity,
                components::Position>();

            pView.each([&](entt::entity      e,
                           PlayerController& pc,
                           PhysicsBody&      pb,
                           Sprite&           s,
                           Velocity&         v,
                           Position&         pos) {});
        }
    };

    static Camera2D CreateCamera2D(CameraConfig& cfg)
    {
        Camera2D cam = { .offset   = cfg.Offset,
                         .target   = cfg.Target,
                         .rotation = cfg.Rotation,
                         .zoom     = cfg.Zoom };

        return cam;
    }

    static void DrawTerrain(Scene* scene, Texture2D* texture)
    {
        float size    = 16.0f;
        float bigSize = 32.0f;
        enum tile : uint32_t
        {
            A = 0,
            B = 1,
            C = 2,
            D = 3
        };

        // create tiles (obviously do this somewhere else)
        static bool    first        = true;
        const uint32_t count        = 1024;
        static tile    tiles[count] = {};

        if (first)
        {
            memset(tiles, A, sizeof(tiles));
            for (uint32_t i = 0; i < count; i++)
            {

                uint32_t x = i % 32;
                uint32_t y = i / 32;

                if (y >= 16 && x >= 16)
                    tiles[i] = D;
                else if (y >= 16)
                    tiles[i] = C;
                else if (x >= x)
                    tiles[i] = B;
            }
            first = false;
        }

        // draw tiles: A is 0,0, B is 16, 0, C is 0, 16, D is 16, 16
        static Rectangle srcRects[4] = {
            Rectangle{ 0.0f, 0.0f, size, size },
            Rectangle{ size, 0.0f, size, size },
            Rectangle{ 0.0f, size, size, size },
            Rectangle{ size, size, size, size },
        };

        for (uint32_t i = 0; i < count; i++)
        {
            uint32_t x = i % 32;
            uint32_t y = i / 32;
            tile     t = tiles[i];
            DrawTexturePro(
                *texture,
                srcRects[static_cast<uint32_t>(t)],
                Rectangle{ x * bigSize, y * bigSize, bigSize, bigSize },
                Vector2{ 0, 0 },
                0,
                WHITE);
        }
    }
} // namespace proto

struct GameConfig
{
    static constexpr int DEFAULT_WIDTH  = 800;
    static constexpr int DEFAULT_HEIGHT = 800;

    struct WindowConfig
    {
        int         Width  = 800;
        int         Height = 600;
        std::string Title  = "Raylib Works!";
        int         FPS    = 60;

        unsigned int Flags = FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE;
    } Window;

    CameraConfig camCfg = {
        // Camera2D
        .Offset   = { 0, 0 },
        .Target   = { Window.Width * 0.5f, Window.Height * 0.5f },
        .Rotation = 0.0f,
        .Zoom     = 1.0f,

        // Other settings
        .Origin   = { 0, 0 },
        .ZoomBase = 1.0f,
        .ZoomMin  = 0.1f,
        .ZoomMax  = 3.0f,
        .ZoomStep = 0.1f
    };
};

static void RLInitWindowFromConfig(GameConfig& cfg)
{
    SetConfigFlags(cfg.Window.Flags);
    InitWindow(cfg.Window.Width, cfg.Window.Height, cfg.Window.Title.c_str());
    SetTargetFPS(cfg.Window.FPS);
}

// bind function to lua environment
static int l_RLInitWindowFromConfig(GameConfig& cfg)
{
    RLInitWindowFromConfig(cfg);
    return 0;
}

struct PhoneNumber
{
    char characters[11] = {};
};

struct Age
{
    int age = 0;
};

class Character
{
  public:
  private:
    Vector2 position;
    Vector2 velocity;

    bool        alive;
    BoundingBox bb;
};

struct MyPlayer
{
};

void CreateNewCharacter(entt::registry& registry)
{
    using namespace components;

    entt::entity e = registry.create();
    registry.emplace<MyPlayer>(e, MyPlayer{});

    registry.emplace<components::Position>(e, components::Position{ 0, 0 });
    registry.emplace<Velocity>(e, Velocity{ 0, 0 });
}

/// @brief entry point
int main(void)
{

    entt::registry registry;

    CreateNewCharacter(registry);

    float outOfBounds = 100.0f;

    auto view = registry.view<MyPlayer, components::Position>();

    for (auto& e : view)
    {
        auto pos = registry.get<components::Position>(e);

        std::cout << "Entity " << static_cast<unsigned int>(e) << " \n";
        if (pos.position.x > outOfBounds)
            std::cout << "OUT OF BOUNDS\n";
        else
            std::cout << "IN BOUNDS!\n";
    }

    GameConfig cfg = {};

    RLInitWindowFromConfig(cfg);
    // raylib window setup

    // start engine
    Engine engine;

    // create scene
    Scene scene;

    // add initial systems
    scene.AddSystem<InputSystem>();
    scene.AddSystem<RenderSystem>();
    scene.AddSystem<proto::UpdatePlayerSystem>();

    float screenWidth  = cfg.Window.Width;
    float screenHeight = cfg.Window.Height;

    {
        CameraConfig camCfg = {
            // Camera2D
            .Offset   = { 0, 0 },
            .Target   = { screenWidth * 0.5f, screenHeight * 0.5f },
            .Rotation = 0.0f,
            .Zoom     = 1.0f,

            // Other settings
            .Origin   = { 0, 0 },
            .ZoomBase = 1.0f,
            .ZoomMin  = 0.1f,
            .ZoomMax  = 3.0f,
            .ZoomStep = 0.1f
        };

        entt::entity eCam = scene.Create();

        scene.RegisterComponent<Camera2D>(eCam, proto::CreateCamera2D(camCfg));
        scene.RegisterComponent<CameraConfig>(eCam, camCfg);

        scene.GetTrackedEntities().MainCamera = eCam;
    }

    // test: try getting Camera and config
    {
        entt::registry& r = scene.GetRegistry();

        auto view = r.view<Camera2D, CameraConfig>();

        for (auto e : view)
        {
            if (e == scene.GetTrackedEntities().MainCamera)
                std::cout << "Checking main camera..." << (uint32_t)e << "\n";

            auto& camera = view.get<Camera2D>(e);
            auto& cfg    = view.get<CameraConfig>(e);

            float x = camera.offset.x + cfg.Offset.x;
            std::cout << "x: " << x << "\n";
        }
    }

    // prototype: testing raygui by creating a simple file dialog window
    const char*                   cwd = GetWorkingDirectory();
    gui::GuiWindowFileDialogState fileDialog =
        gui::InitGuiWindowFileDialog(cwd, 400, 300);

    Texture2D* tileTexture = engine.LoadTexture("test_tiles.png");
    if (!tileTexture)
        throw std::runtime_error("Failed to load texture!");
    //// load player entity
    //LoadPlayer(scene, engine);

    auto allView = scene.GetRegistry().view<entt::entity>();

    Rectangle srcTiles[4];
    for (int i = 0; i < 4; i++)
    {
        srcTiles[i] = Rectangle{ static_cast<float>((i % 2) * 16),
                                 static_cast<float>((i / 2)) * 16.0f,
                                 16.0f,
                                 16.0f };
    }
    constexpr int dims = 8;

    Rectangle dstTiles[dims * dims] = { 0 };
    Vector2   tilesOffset{ 100, 100 };

    {
        // set up dst tiles for the grid.
        // 0,0 is top ground, 1,0 is top corners (flip for left side)
        // 0,1 is brown dirt, 1,1 is side
        for (int i = 0; i < dims; i++)
        {
            for (int j = 0; j < dims; j++)
            {
                float x = 0, y = 0;

                if (j == 0)
                    dstTiles[i * dims + j] =
                        Rectangle{ x * dims, y * dims, dims, dims };
            }
        }
    }

    // start game loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(SKYBLUE);

        const float dt = GetFrameTime();
        scene.Update(dt);

        scene.Render();

        proto::DrawTerrain(&scene, tileTexture);

        EndDrawing();
    }

    std::cout << "Raylib finished. Exiting...\n";
}
