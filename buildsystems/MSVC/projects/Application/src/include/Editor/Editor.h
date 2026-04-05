#pragma once

#include <cstdint>

#include <functional>
#include <string>
#include <iostream>

#include <raylib.h>
#include <raygui.h>

namespace editor
{
    enum class ElementType : uint8_t
    {
        Text,
        Button,
        Slider,
        Checkbox,
    };

    enum class EditorCommand
    {
        None,
        SaveProject,
        LoadProject,
        EnterGame,
        ToggleSidePanel
    };

    struct GuiElement
    {
        std::string           text = "GuiElement";
        ElementType           type = ElementType::Text;
        std::function<void()> action;

        // optional
        Rectangle bounds{ 0, 0, 100, 50 }; // for buttons and sliders
        float     sliderValue   = 0.0f;    // for sliders
        float     size          = 20.0f;   // for text size
        bool      checkboxValue = false;   // for checkboxes
    };

    struct Gui
    {
        std::vector<GuiElement> elements;

        void Draw()
        {
            for (auto& elem : elements)
            {
                switch (elem.type)
                {
                case ElementType::Text:
                    DrawText(
                        elem.text.c_str(), static_cast<int>(elem.bounds.x), static_cast<int>(elem.bounds.y),
                        static_cast<int>(elem.size), BLACK);
                    break;
                case ElementType::Button:
                    if (GuiButton(elem.bounds, elem.text.c_str()))
                        elem.action();
                    break;
                case ElementType::Checkbox:
                    if (GuiCheckBox(elem.bounds, elem.text.c_str(), &elem.checkboxValue))
                        elem.action();
                    break;
                case ElementType::Slider:
                    break;
                default:
                    break;
                }
            }
        }
    };

    struct Editor
    {
        Gui  gui;
        bool enabled = true;
        Scene* scene = nullptr;

        void Initialize(Scene& scene)
        {
            this->scene = &scene;
        }
        void Draw()
        {
            if (!enabled)
                return;

            ClearBackground(RED);
            BeginDrawing();
            gui.Draw();
            EndDrawing();
        }

        void Update() {}

        void Toggle() { enabled = !enabled; }
    };

    struct GuiConfig
    {

        std::vector<std::string> tabs;          // for future use, if we want multiple tabs of elements
        int                      activeTab = 0; // index of the currently active tab

        std::vector<GuiElement> elements; // definition of all GUI elements to be created in the editor
    };

    // for now we just hardcode the config, later should be managed through a lua file
    static GuiConfig CreateGuiConfig()
    {
        GuiConfig config;
        config.tabs = { "Main" };

        const int   numElements = 5;
        const float gap         = 0.0f;
        const float width       = 60.0f;
        const float height      = 20.0f;
        float       offsetX     = 0.0f;
        float       offsetY     = 0.0f;

        for (int i = 0; i < numElements; ++i)
        {
            config.elements.push_back(
                GuiElement{ .text   = "Click Me",
                            .type   = ElementType::Button,
                            .action = [i]() { std::cout << "Button" << std::to_string(i) << " clicked!\n"; },
                            .bounds = Rectangle{ offsetX, offsetY, width, height } });
            offsetX += width + gap;
        }

        config.elements.push_back(
            GuiElement{ .text   = "Hello, World!",
                        .type   = ElementType::Text,
                        .bounds = Rectangle{ 10.0f, 50.0f, 0, 0 },
                        .size   = 30.0f });

        config.elements.push_back(
            GuiElement{ .text   = "Toggle Editor",
                        .type   = ElementType::Checkbox,
                        .action = []() { std::cout << "Checkbox toggled!\n"; },
                        .bounds = Rectangle{ 10.0f, 100.0f, 120.0f, 30.0f } });

        return config;
    }

    static Gui CreateGui(const GuiConfig& config)
    {
        // In a real implementation, you'd want to set up the bounds and other parameters for each element properly
        Gui gui;

        gui.elements = config.elements;

        return gui;
    }
} // namespace editor
