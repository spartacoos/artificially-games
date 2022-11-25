// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "SDL_video.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <cstddef>
#include <cstdio>
#include <stdio.h>
#include <SDL.h>
#include <tuple>
#include <unistd.h>
// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
// When compiling with emscripten however, no specific loader is necessary, since the webgl wrapper will do it for you.
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <SDL_opengles2.h>
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
// #include "imgui_impl_opengl3_loader.h"
#endif

#if defined(__EMSCRIPTEN__)
// Emscripten wants to run the mainloop because of the way the browser is single threaded.
// For this, it requires a void() function. In order to avoid breaking the flow of the
// readability of this example, we're going to use a C++11 lambda to keep the mainloop code
// within the rest of the main function. This hack isn't needed in production, and one may
// simply have an actual main loop function to give to emscripten instead.
#include <functional>
static std::function<void()> loop;
static void main_loop() { loop(); }
#endif


import test;

static bool done = false;
static BoardState board{};
static bool popup_closed = true;
static MiniMax ai{SquareState::Circle};
static bool selected[BOARD_SIZE] = { false, false, false, false, false, false, false, false, false};

void init_gui(SDL_Event event, SDL_Window* window){
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
            done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            done = true;
    }

}

void reset_ui(){
    for(std::size_t x = 0; x < DIM; x++){
        for(std::size_t y=0; y < DIM; y++){
            selected[DIM*x+y] = false;
        }
    }
}

void update_gui(SDL_Window* window, SDL_GLContext gl_context, ImVec4 clear_color, ImGuiIO& io, bool show_demo_window, bool show_another_window){
    // Start the Dear ImGui frame

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGuiSelectableFlags flags = ImGuiWindowFlags_None;
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("TicTacToe Window", &show_another_window, flags);

        // auto [btx, bty] = 90,90;
        int btxx= 150, btyy = 150;
        // run_game_engine(board,ai,board.did_cross_win, board.did_circle_win, board.draw_condition,popup_closed);
       //-------------------------
        // static bool dummy[9] = {};
        for(size_t x=0; x<DIM; x++){
            for(size_t y=0; y<DIM; y++){
                if(y>0){
                    ImGui::SameLine();
                }
                if (board.grid[x][y] == SquareState::Empty){
                    ImGui::PushID(DIM*x + y);
                    // ImGui::Selectable("Empty", &selected[DIM * x + y],ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    // ImGui::Selectable("Empty", &dummy[DIM * x + y], ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    // ImGui::PushStyleColor(ImGuiCol_ButtonHovered);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(9.0f,0.5f,0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(9.0f,0.5f,0.5f));
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.5f,9.0f,0.5f));
                    if(ImGui::Button(" ", ImVec2(btxx,btyy))){
                        board.grid[x][y] = SquareState::Cross;
                        board.player_made_move = true;
                    }
                    // if(selected[DIM*x+y]){
                    //     board.grid[x][y] = SquareState::Cross;
                    //     board.player_made_move = true;
                    // }
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();

                } else if (board.grid[x][y] == SquareState::Circle){
                    ImGui::PushID(DIM*x + y);
                    ImGui::Button("O",ImVec2(btxx,btyy));
                    // ImGui::Selectable("Circle", &selected[DIM * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    // ImGui::Selectable("Circle", &dummy, ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    ImGui::PopID();
                } else if (board.grid[x][y] == SquareState::Cross){
                    ImGui::PushID(DIM*x + y);
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV( 7.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(9.0f,0.5f,0.5f));
                    ImGui::Button("X",ImVec2(btxx,btyy));
                    // ImGui::Selectable("Cross", &selected[DIM * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    // ImGui::Selectable("Cross", &dummy, ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                    ImGui::PopStyleColor(2);
                    ImGui::PopID();
                }
            }

        }

        board.did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
        if(!board.is_draw() && !board.did_cross_win && board.player_made_move){
            Position ai_pos = ai.next_move(board);
            board.grid[ai_pos.x][ai_pos.y] = ai.maximizing_player;
            board.player_made_move = false;
        }
        board.did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
        board.draw_condition = board.is_draw();
        if (ImGui::Button("Restart")){
            ImGui::CloseCurrentPopup();
            board.state = game::started;
            // board.state = game::finished;
        }
       //-------------------------
        if(board.did_circle_win && !popup_closed){
            ImGui::OpenPopup("Winner");
            if(ImGui::BeginPopup("Winner")){
                ImGui::Text("Circle won!");
                if(ImGui::Button("Close"))
                    popup_closed = !popup_closed;
                if(ImGui::Button("Quit Game"))
                    board.state = game::finished;
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);





}

void cleanup_gui(SDL_GLContext gl_context, SDL_Window* window){
    // Cleanup_Gui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}



int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(__EMSCRIPTEN__)
    // GLES 3.0
    // For the browser using emscripten, we are going to use WebGL2 with GLES3. See the Makefile.emscripten for requirement details.
    // It is very likely the generated file won't work in many browsers. Firefox is the only sure bet, but I have successfully
    // run this code on Chrome for Android for example.
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    // SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_Window* window = SDL_CreateWindow("TheOld", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(__EMSCRIPTEN__)
    bool err = false; // Emscripten loads everything during SDL_GL_CreateContext
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("Park Tech CG.ttf", 55.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
    // auto fonty = ImFontConfig();
    // fonty.SizePixels = 55.0f;
    // io.Fonts->AddFontDefault(&fonty);




    // io.Fonts->AddFontDefault();
    // ImFont *fonty =io.Fonts->AddFontDefault(); //io.Fonts->AddFontFromFileTTF("Park Tech CG.ttf", 65.0f);
    bool show_demo_window = false;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
#if defined(__EMSCRIPTEN__)
    // See comments around line 30.
    loop = [&](){

        switch (board.state) {
            case game::started:{
            // init everything to the starting conditions
            reset_ui();
            board.reset();
            popup_closed = false;
            board.state = game::playing;
            // #if defined(__EMSCRIPTEN__)
            //     // See comments around line 30.
            //     // This function call will not return.
            //     emscripten_set_main_loop(main_loop, 0, true);
            //     // A fully portable version of this code that doesn't use the lambda hack might have an #else that does:
            //     //    while (!done) main_loop();
            // #endif
            } break;
            case game::playing:{
                SDL_Event event;
                init_gui(event, window);
                update_gui(window, gl_context, clear_color,io, show_demo_window, show_another_window);
            } break;
            case game::finished:{
                done = true;
            } break;
            default:{
                done = true;
            }break;
        }
    };

    emscripten_set_main_loop(main_loop, 0, true);


    // Note that this doesn't process the 'done' boolean anymore. The function emscripten_set_main_loop will
    // in effect not return, and will set an infinite loop, that is going to be impossible to break. The
    // application can then only stop when the brower's tab is closed, and you will NOT get any notification
    // about exitting. The browser will then cleanup_gui all resources on its own.
#else
    while (!done)
#endif
    {
        switch (board.state) {
            case game::started:{
            // init everything to the starting conditions
            reset_ui();
            board.reset();
            popup_closed = false;
            board.state = game::playing;
            #if defined(__EMSCRIPTEN__)
                // See comments around line 30.
                // This function call will not return.
                emscripten_set_main_loop(main_loop, 0, true);
                // A fully portable version of this code that doesn't use the lambda hack might have an #else that does:
                //    while (!done) main_loop();
            #endif
            } break;
            case game::playing:{
                SDL_Event event;
                init_gui(event, window);
                update_gui(window, gl_context, clear_color,io, show_demo_window, show_another_window);
            } break;
            case game::finished:{
                done = true;
            } break;
            default:{
                done = true;
            }break;
        }
    };

    cleanup_gui(gl_context,window);
    return 0;
}
