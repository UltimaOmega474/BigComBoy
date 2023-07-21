#include "menu_bar.hpp"
#include "gui_constants.hpp"
#include "state.hpp"
#include "config.hpp"
#include "palette_edit.hpp"
#include <gb/constants.hpp>
#include <string>
#include <fmt/format.h>
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <nfd.hpp>
#include <chrono>
#include <iostream>
int main(int argc, char **argv)
{
	using namespace std::chrono_literals;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fmt::print("Unable to initialize SDL2 Video\n");
		return 0;
	}

	NFD::Init();

#ifdef WIN32
	// d3d12 crashes when resizing and the default is d3d9
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");
#endif

	SDL_Window *window = SDL_CreateWindow("SunBoy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SunBoy::LCD_WIDTH * 3, SunBoy::LCD_HEIGHT * 3 + SunBoy::MENU_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	SDL_SetWindowMinimumSize(window, SunBoy::LCD_WIDTH, SunBoy::LCD_HEIGHT + SunBoy::MENU_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	auto ctx = ImGui::CreateContext();
	ImGui::StyleColorsLight();
	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.FontDefault = io.Fonts->AddFontFromFileTTF(SunBoy::OPEN_SANS_SEMIBOLD_PATH.c_str(), SunBoy::FONT_RENDER_SIZE);
	io.FontGlobalScale = SunBoy::FONT_SIZE / SunBoy::FONT_RENDER_SIZE;

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	bool running = true;
	constexpr std::chrono::nanoseconds max_step_delay = 33ms;
	constexpr auto logic_rate = SunBoy::set_update_frequency_hz(60);
	auto accumulator = 0ns, sram_accumulator = 0ns;
	auto old_time = std::chrono::steady_clock::now();

	SunBoy::MenuBar menu;
	auto &config = SunBoy::Configuration::get();
	auto &state = SunBoy::EmulationState::current_state();

	state.create_texture(renderer);
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT ||
				event.type == SDL_WINDOWEVENT &&
					event.window.event == SDL_WINDOWEVENT_CLOSE &&
					event.window.windowID == SDL_GetWindowID(window))
			{
				running = false;
			}
		}

		auto time_now = std::chrono::steady_clock::now();
		auto full_delta = time_now - old_time;
		old_time = time_now;

		if (full_delta > max_step_delay)
			full_delta = max_step_delay;

		accumulator += full_delta;

		state.keyboard_input.update_state(state.core.pad);

		if (accumulator >= logic_rate)
		{
			if (state.status == SunBoy::Status::Running)
				state.step_frame();

			accumulator -= logic_rate;
		}

		if (config.allow_sram_saving && state.cart)
		{
			sram_accumulator += full_delta;
			auto target_rate_sram = std::chrono::seconds(config.sram_save_interval);
			if (sram_accumulator > 1s)
			{
				fmt::print("saving: {}\n", state.cart->header.title);

				state.cart->save_sram_to_file();
				sram_accumulator -= target_rate_sram;
			}
		}
		else
		{
			sram_accumulator = 0ns;
		}

		SDL_RenderClear(renderer);

		if (state.status == SunBoy::Status::Running)
			state.draw_frame(window, renderer);

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		menu.draw();
		ImGui::Render();
		SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);

		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}
	if (state.cart)
		state.cart->save_sram_to_file();

	config.save_as_toml_file();
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	NFD::Quit();
	SDL_Quit();

	return 0;
}