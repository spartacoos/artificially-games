// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

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


// Now here we include the game logic:
// #include "../../game.h"
// #include "../game.cpp"
import engine;
// BoardState bs{};
// bs.winner = true;

void run_game_engine(BoardState& board,MiniMax &ai,bool &did_cross_win, bool &did_circle_win, bool &draw_condition, bool &popup_closed){
    static bool player_made_move = false;
    static bool selected[3 * 3] = { false, false, false, false, false, false, false, false, false};
    // ImGui::Text("Hello from my Appp!");
    // static char selected[4][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
    for(size_t x=0; x<3; x++){
        for(size_t y=0; y<3; y++){
            // ImGui::Button(" ");
            // ImGui::Selectable("Square", true,0,ImVec2(50,50));
            if(y>0){
                ImGui::SameLine();
            }
            if (board.grid[x][y] == SquareState::Empty){
                ImGui::PushID(3*x + y);
                ImGui::Selectable("Empty", &selected[3 * x + y],ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                if(selected[3*x+y]){
                    board.grid[x][y] = SquareState::Cross;
                    player_made_move = true;
                }
                ImGui::PopID();

            } else if (board.grid[x][y] == SquareState::Circle){
                ImGui::PushID(3*x + y);
                ImGui::Selectable("Circle", &selected[3 * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                ImGui::PopID();
            } else if (board.grid[x][y] == SquareState::Cross){
                ImGui::PushID(3*x + y);
                ImGui::Selectable("Cross", &selected[3 * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
                ImGui::PopID();
            }
        }

    }

    did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    if(!board.is_draw() && !did_cross_win && player_made_move){
        Position ai_pos = ai.next_move(board);
        board.grid[ai_pos.x][ai_pos.y] = ai.maximizing_player;//SquareState::Circle;
        player_made_move = false;
    }
    did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    draw_condition = board.is_draw();

    // if(did_circle_win || did_cross_win){
    //     // printf("HEREEEE circ: %d cross: %d\n", did_circle_win,did_cross_win);
    //     // bool popup_flags = ImGuiPopupFlags_None;//ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_NoOpenOverExistingPopup;
    //     ImGui::OpenPopup("WINNER");//, popup_flags);
    //     if (ImGui::BeginPopup("WINNER")){
    //         if(did_circle_win){
    //             // did_circle_win = false;
    //     // printf("HEREEEE circ: %d cross: %d\n", did_circle_win,did_cross_win);
    //     // printf("HEREEEE\n");
    //             ImGui::Text("Circle Won!!!");
    //         }
    //         if(ImGui::Button("Close")){
    //            did_circle_win = false;
    //            did_cross_win = false;
    //            draw_condition = false;
    //            already_closed = true;
    //            // ImGui::EndPopup();
    //         }
    //             // sleep(2);
    //             // ImGui::CloseCurrentPopup();
    //         //     if(ImGui::IsPopupOpen("WINNER")){
    //         //         printf("herea\n");
    //         //         sleep(2);
    //         //         ImGui::CloseCurrentPopup();
    //         // ImGui::EndPopup();
    //         //         did_circle_win = false;
    //         //     }
    //             // printf("Circle won\n");
    //         // }else{
    //         //     // did_cross_win = false;
    //         //     ImGui::Text("Cross Won!!!");
    //         //     // printf("Cross won\n");
    //         // }
    //         // ImGui::EndPopup();
    //     }
    //     if(already_closed){
    //         ImGui::EndPopup();
    //         ImGui::CloseCurrentPopup();
    //     }
    // }

    // If somebody wins we want to record who won then reset everything
    // otherwise in the next iteration wil lencounter issues where this function
    // will revaluate the board and reset the booleans for did_circle_win, did_cross_win which
    // we don't want since we want to

// }

    if (ImGui::Button("Restart")){
        ImGui::CloseCurrentPopup();
        for(std::size_t i = 0; i < 3; i++){
            for(std::size_t j=0; j < 3; j++){
                selected[3*i+j] = false;
                board.grid[i][j] = SquareState::Empty;
                player_made_move = false;
                did_circle_win = false;
                did_cross_win = false;
                draw_condition = false;
                popup_closed = !popup_closed;
            }
        }
    }
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
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    bool show_demo_window = false;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // GameBoard setup code
    BoardState board{};
    // board.grid[1][1] = SquareState::Circle;
    // board.grid[2][2] = SquareState::Cross;
    // make function that can display board state
    // MiniMax ai{SquareState::Circle};
    // printf("Winner: %d\n", bs.winner);

    // Main loop
    bool done = false;
    bool did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
    bool did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
    bool draw_condition = board.is_draw();
    static bool popup_closed = false;
    MiniMax ai{SquareState::Circle};
#if defined(__EMSCRIPTEN__)
    // See comments around line 30.
    loop = [&]()
    // Note that this doesn't process the 'done' boolean anymore. The function emscripten_set_main_loop will
    // in effect not return, and will set an infinite loop, that is going to be impossible to break. The
    // application can then only stop when the brower's tab is closed, and you will NOT get any notification
    // about exitting. The browser will then cleanup all resources on its own.
#else
    while (!done) //&& !did_circle_win && !did_cross_win && !draw_condition)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

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
            // BoardState bs;
            // bs.winner = true;
            // std::cout << "Winner: " << bs.winner << std::endl;
            // printf("Winner: %d", bs.winner);

            ImGuiSelectableFlags flags = ImGuiWindowFlags_None;//ImGuiSelectableFlags_DontClosePopups;
            flags |= ImGuiWindowFlags_AlwaysAutoResize;
            // flags |= ImGuiSelectableFlags_SpanAllColumns;
            ImGui::Begin("TicTacToe Window", &show_another_window, flags);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            // auto [player_made_move,selected] = run_game_engine(board,ai,did_cross_win, did_circle_win, draw_condition);
            run_game_engine(board,ai,did_cross_win, did_circle_win, draw_condition,popup_closed);
            if(did_circle_win && !popup_closed){
                ImGui::OpenPopup("Winner");
                if(ImGui::BeginPopup("Winner")){
                    ImGui::Text("Circle won!");
                    if(ImGui::Button("Close"))
                        popup_closed = !popup_closed;
                    // did_circle_win = false;
                }
                ImGui::EndPopup();
            }
            // static bool player_made_move = false;
            // static bool selected[3 * 3] = { false, false, false, false, false, false, false, false, false};
            // // ImGui::Text("Hello from my Appp!");
            // // static char selected[4][4] = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
            // for(size_t x=0; x<3; x++){
            //     for(size_t y=0; y<3; y++){
            //         // ImGui::Button(" ");
            //         // ImGui::Selectable("Square", true,0,ImVec2(50,50));
            //         if(y>0){
            //             ImGui::SameLine();
            //         }
            //         if (board.grid[x][y] == SquareState::Empty){
            //             ImGui::PushID(3*x + y);
            //             ImGui::Selectable("Empty", &selected[3 * x + y],ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
            //             if(selected[3*x+y]){
            //                 board.grid[x][y] = SquareState::Cross;
            //                 player_made_move = true;
            //             }
            //             ImGui::PopID();

            //         } else if (board.grid[x][y] == SquareState::Circle){
            //             ImGui::PushID(3*x + y);
            //             ImGui::Selectable("Circle", &selected[3 * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
            //             ImGui::PopID();
            //         } else if (board.grid[x][y] == SquareState::Cross){
            //             ImGui::PushID(3*x + y);
            //             ImGui::Selectable("Cross", &selected[3 * x + y],ImGuiSelectableFlags_DontClosePopups,ImVec2(50,50));
            //             ImGui::PopID();
            //         }
            //     }

            // }

            // did_cross_win = board.exhaustive_check_for_winner(SquareState::Cross);
            // if(!board.is_draw() && !did_cross_win && player_made_move){
            //     Position ai_pos = ai.next_move(board);
            //     board.grid[ai_pos.x][ai_pos.y] = ai.maximizing_player;//SquareState::Circle;
            //     player_made_move = false;
            // }
            // did_circle_win = board.exhaustive_check_for_winner(SquareState::Circle);
            // draw_condition = board.is_draw();

            // if (ImGui::Button("Close Me")){
            //     show_another_window = false;
            // }
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
    };
#if defined(__EMSCRIPTEN__)
    // See comments around line 30.
    // This function call will not return.
    emscripten_set_main_loop(main_loop, 0, true);
    // A fully portable version of this code that doesn't use the lambda hack might have an #else that does:
    //    while (!done) main_loop();
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
