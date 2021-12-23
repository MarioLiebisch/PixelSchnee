#include <SFML/Graphics.hpp>
#include <math.h>

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	namespace {
		// https://stackoverflow.com/questions/1683791/drawing-on-the-desktop-background-as-wallpaper-replacement-windows-c

		BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
			HWND p = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
			HWND* ret = (HWND*)lParam;

			if (p)
			{
				// Gets the WorkerW Window after the current one.
				*ret = FindWindowEx(NULL, hwnd, "WorkerW", NULL);
			}
			return true;
		}

		HWND get_wallpaper_window() {
			// Fetch the Progman window
			HWND progman = FindWindow("ProgMan", NULL);
			// Send 0x052C to Progman. This message directs Progman to spawn a 
			// WorkerW behind the desktop icons. If it is already there, nothing 
			// happens.
			SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
			// We enumerate all Windows, until we find one, that has the SHELLDLL_DefView 
			// as a child. 
			// If we found that window, we take its next sibling and assign it to workerw.
			HWND wallpaper_hwnd = nullptr;
			EnumWindows(EnumWindowsProc, (LPARAM)&wallpaper_hwnd);
			// Return the handle you're looking for.
			return wallpaper_hwnd;
		}
	}
#else
	#include <X11/Xlib.h>
#endif

auto main(int argc, char** argv) -> int {
	// Initialize the pseudo random number generator
	srand(static_cast<unsigned int>(time(nullptr)));

#ifdef NDEBUG
	// For Release builds, try to grab the desktop background
	#ifdef WIN32
		sf::RenderWindow window(get_wallpaper_window());
	#else
		Display* d = XOpenDisplay(NULL);
		int s = DefaultScreen(d);
		Window w = RootWindow(d, s);
		sf::RenderWindow window(w);
	#endif
#else
	// For debugging, just use a regular SFML window
	sf::RenderWindow window(sf::VideoMode{ 640,480 }, "PixelSchnee");
#endif

	// Make sure to only render 15 times a second to limit and reduce CPU/GPU load
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(15);

	// Desktop/window size
	const auto res = window.getSize();

	// Setup our snowflakes as a vertex array
	// The number of flakes depends on the screen resolution
	sf::VertexArray snow(sf::Points, res.x * res.y / 500);

	// Put snowflakes into random positions
	for (auto i = 0; i < snow.getVertexCount(); i++) {
		auto& flake = snow[i];
		flake.position.x = static_cast<float>(rand() % res.x);
		flake.position.y = static_cast<float>(rand() % res.y);
	}

	sf::Clock timer;

	// Create a simple gradient background
	sf::VertexArray bg(sf::TriangleFan, 4);
	bg[1].position.x = bg[2].position.x = static_cast<float>(res.x);
	bg[2].position.y = bg[3].position.y = static_cast<float>(res.y);
	bg[0].color = bg[1].color = sf::Color::Black;
	bg[2].color = bg[3].color = sf::Color(16, 32, 64);


	float last = 0.f;

	while (window.isOpen()) {
		sf::Event event;
		// Handle events (e.g. for graceful shutdown)
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		// Listen for global keyboard input to quit
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F4) && (sf::Keyboard::isKeyPressed(sf::Keyboard::LSystem) || sf::Keyboard::isKeyPressed(sf::Keyboard::RSystem))) {
			window.close();
			break;
		}

		// Calculate the detla time since last frame
		const float time = timer.getElapsedTime().asSeconds();
		const float delta = time - last;
		last = time;

		// Let's calculate some procedural wind
		const float wind = (std::abs(std::sin(time)) + std::cos(time / 5.f) + std::sin(time + 2.f) + std::cos(time / 10.f)) / 4.f;

		// Move all our snowflakes
		for (auto i = 0; i < snow.getVertexCount(); i++) {
			auto& flake = snow[i];
			// Update their position
			flake.position.x += wind * (1 + i % 5) * 32.f * delta;
			flake.position.y += (1 + i % 4) * 16.f * delta;
			// Wrap them around the borders of the screen
			// The bottom one,
			if (flake.position.y > res.y) {
				flake.position.y -= res.y;
			}
			// the left one,
			if (flake.position.x < 0) {
				flake.position.x += res.x;
			// and the right one
			} else if (flake.position.x > res.x) {
				flake.position.x -= res.x;
			}
		}

		// Draw and display everything
		window.draw(bg);
		window.draw(snow);
		window.display();
	}

#ifdef NDEBUG
	#ifndef WIN32
		XCloseDisplay(d);
	#endif
#endif

	return 0;
}
