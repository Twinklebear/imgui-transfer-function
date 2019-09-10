#include <array>
#include <iostream>
#include <memory>
#include <vector>
#include <SDL.h>
#include "gl_core_4_5.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "shader.h"
#include "stb_image.h"
#include "transfer_function_widget.h"

const std::string display_colormap_vs = R"(
#version 450 core

const vec4 pos[4] = vec4[4](
	vec4(-1, 1, 0.5, 1),
	vec4(-1, -1, 0.5, 1),
	vec4(1, 1, 0.5, 1),
	vec4(1, -1, 0.5, 1)
);

const float uv[4] = float[4](0, 0, 1, 1);

out float vuv;

void main(void) {
	gl_Position = pos[gl_VertexID];
	vuv = uv[gl_VertexID];
}
)";

const std::string display_colormap_fs = R"(
#version 450 core

layout(binding = 0) uniform sampler1D img;

in float vuv;

out vec4 color;

void main(void) { 
	color = texture(img, vuv);
	color.rgb = color.rgb * color.a;
})";

int win_width = 1280;
int win_height = 720;

void run_app(int argc, const char **argv, SDL_Window *window);

int main(int argc, const char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
        return -1;
    }

    const char *glsl_version = "#version 450 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow("ImGui Transfer Function Demo",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          win_width,
                                          win_height,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_MakeCurrent(window, gl_context);

    if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {
        std::cerr << "Failed to initialize OpenGL\n";
        return 1;
    }

    // Setup Dear ImGui context
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    run_app(argc, argv, window);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void run_app(int argc, const char **argv, SDL_Window *window)
{
    ImGuiIO &io = ImGui::GetIO();

    TransferFunctionWidget tfn_widget;

    // Load any extra colormaps the user wants to see in the demo
    for (int i = 1; i < argc; ++i) {
        int w, h, n;
        uint8_t *img_data = stbi_load(argv[i], &w, &h, &n, 4);
        auto img = std::vector<uint8_t>(img_data, img_data + w * 1 * 4);
        stbi_image_free(img_data);
        tfn_widget.add_colormap(Colormap(argv[i], img));
    }

    // A texture so we can color the background of the window by the colormap
    GLuint colormap_texture;
    glGenTextures(1, &colormap_texture);
    glBindTexture(GL_TEXTURE_1D, colormap_texture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    auto colormap = tfn_widget.get_colormap();
    glTexImage1D(GL_TEXTURE_1D,
                 0,
                 GL_RGBA8,
                 colormap.size() / 4,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 colormap.data());

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    Shader display_colormap(display_colormap_vs, display_colormap_fs);

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done = true;
            }
            if (!io.WantCaptureKeyboard && event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                win_width = event.window.data1;
                win_height = event.window.data2;
                io.DisplaySize.x = win_width;
                io.DisplaySize.y = win_height;
            }
        }

        if (tfn_widget.changed()) {
            auto colormap = tfn_widget.get_colormap();
            glTexImage1D(GL_TEXTURE_1D,
                         0,
                         GL_RGBA8,
                         colormap.size() / 4,
                         0,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         colormap.data());
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::Begin("Debug Panel");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        tfn_widget.draw_ui();
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(display_colormap.program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }
}

