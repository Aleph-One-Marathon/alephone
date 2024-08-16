#include "ScenarioChooser.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cseries.h"

#ifdef HAVE_SDL_IMAGE
#include <SDL2/SDL_image.h>
#endif

#include "find_files.h"
#include "images.h"
#include "sdl_fonts.h"

using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

class ScenarioChooserScenario
{
public:
	bool operator<(const ScenarioChooserScenario& other) const;
	
	std::string path;
	
	std::string name;
	std::shared_ptr<SDL_Surface> image;

	bool load(const std::string& path);
	void find_image();
};

class TitleScreenFinder : public FileFinder
{
public:
	TitleScreenFinder(ScenarioChooserScenario& scenario) : FileFinder(), scenario_{scenario} { }
	virtual bool found(FileSpecifier& file)
	{
		if (boost::algorithm::ends_with(file.GetPath(), ".imgA"))
		{
			auto full_size = find_title_screen(file);
			if (full_size)
			{
				scenario_.image = std::move(full_size);
				return true;
			}
		}
		else if (boost::algorithm::ends_with(file.GetPath(), ".appl"))
		{
			auto full_size = find_m1_title_screen(file);
			if (full_size)
			{
				scenario_.image = std::move(full_size);
				return true;
			}
		}

		return false;
	}
	
private:
	ScenarioChooserScenario& scenario_;
};

bool ScenarioChooserScenario::operator<(const ScenarioChooserScenario& other) const
{
	return std::lexicographical_compare(name.begin(),
										name.end(),
										other.name.begin(),
										other.name.end(),
										[](const char& a, const char& b) {
											return tolower(a) < tolower(b);
										});
}

bool ScenarioChooserScenario::load(const std::string& path)
{
	DirectorySpecifier directory(path);
	DirectorySpecifier scripts = directory + "Scripts";

	if (!scripts.Exists())
	{
		return false;
	}

	std::string base;
	std::string part;

	directory.SplitPath(base, part);

	this->path = path;
	name = part;

	std::vector<dir_entry> entries;
	if (scripts.ReadDirectory(entries))
	{
		std::sort(entries.begin(), entries.end());
		for (auto it = entries.rbegin(); it != entries.rend(); ++it)
		{
			if (it->is_directory ||
				it->name[it->name.length() - 1] == '~' ||
				boost::algorithm::ends_with(it->name, ".lua"))
			{
				continue;
			}

			FileSpecifier mml = scripts + it->name;
			boost::property_tree::ptree tree;
			boost::property_tree::read_xml(mml.GetPath(), tree);

			try
			{
				name = tree.get<std::string>("marathon.scenario.<xmlattr>.name");
				break;
			}
			catch (const boost::property_tree::ptree_error&)
			{

			}
		}
	}

#ifdef HAVE_SDL_IMAGE
	FileSpecifier image_file = directory + "chooser.png";
	OpenedFile of;
	if (image_file.Open(of))
	{
		image.reset(IMG_Load_RW(of.GetRWops(), 0));
	}
#endif

	if (!image)
	{
		FileSpecifier image_file = directory + "chooser.bmp";
		OpenedFile of;
		if (image_file.Open(of))
		{
			image.reset(SDL_LoadBMP_RW(of.GetRWops(), 0));
		}
	}

	if (!image)
	{
		find_image();
	}

	return image.get();
}

void ScenarioChooserScenario::find_image()
{
	TitleScreenFinder finder(*this);
	FileSpecifier f(path);
	finder.Find(f, WILDCARD_TYPE, true);
}

ScenarioChooser::ScenarioChooser() :
	scroll_{0}, selection_{-1}
{

}

ScenarioChooser::~ScenarioChooser()
{

}

void ScenarioChooser::add_scenario(const std::string& path)
{
	ScenarioChooserScenario scenario;
	if (scenario.load(path))
	{
		scenarios_.push_back(scenario);
	}
}

void ScenarioChooser::add_directory(const std::string& path)
{
	DirectorySpecifier d(path);

	// assume each visible subdirectory is a scenario
	std::vector<dir_entry> entries;
	if (d.ReadDirectory(entries))
	{
		for (auto& entry : entries)
		{
			if (entry.is_directory)
			{
				add_scenario((d + entry.name).GetPath());
			}
		}
	}
}

int ScenarioChooser::num_scenarios() const
{
	return static_cast<int>(scenarios_.size());
}

std::string ScenarioChooser::run()
{
	std::sort(scenarios_.begin(), scenarios_.end());

	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(0, &mode);

	window_width_ = mode.w;
	window_height_ = mode.h;

	determine_cols_rows();
	WindowPtr window(SDL_CreateWindow("Choose a Scenario",
									  SDL_WINDOWPOS_CENTERED,
									  SDL_WINDOWPOS_CENTERED,
									  window_width_,
									  window_height_,
									  SDL_WINDOW_FULLSCREEN_DESKTOP),
					 SDL_DestroyWindow);

	for (auto& scenario : scenarios_)
	{
		optimize_image(scenario, window.get());
	}

	SDL_ShowWindow(window.get());

	done_ = false;
	while (!done_)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			handle_event(e);
		}

		redraw(window.get());
		SDL_Delay(30);
	}

	return scenarios_[selection_].path;
}

void ScenarioChooser::determine_cols_rows()
{
	auto available_width = window_width_ - margin;
	auto widget_width = scenario_width + margin;

	cols_ = available_width / widget_width;
	if (cols_ > scenarios_.size())
	{
		cols_ = scenarios_.size();
	}
	
	rows_ = (scenarios_.size() + (cols_ - 1)) / cols_;

	// center
	offsetx_ = (window_width_ - cols_ * (scenario_width + margin) - margin) / 2;

	auto total_height = rows_ * scenario_height + (rows_ + 1) * margin;
	if (total_height > window_height_)
	{
		offsety_ = 0;
		max_scroll_ = total_height - window_height_;
	}
	else
	{
		offsety_ = (window_height_ - total_height) / 2;
		max_scroll_ = 0;
	}

	if (scroll_ > max_scroll_)
	{
		scroll_ = max_scroll_;
	}
}

void ScenarioChooser::ensure_selection_visible()
{
	auto row = selection_ / cols_;
	
	auto top = offsety_ + row * (scenario_height + margin);
	auto bottom = top + scenario_height + 2 * margin;
	
	if (scroll_ > top || scroll_ < bottom - window_height_)
	{
		scroll_ = (top + bottom - window_height_) / 2;
	}
	
	if (scroll_ < 0)
	{
		scroll_ = 0;
	}
	else if (scroll_ > max_scroll_)
	{
			scroll_ = max_scroll_;
	}
}

void ScenarioChooser::handle_event(SDL_Event& e)
{
	switch (e.type)
	{
		case SDL_CONTROLLERBUTTONDOWN:
			switch (e.cbutton.button)
			{
				case SDL_CONTROLLER_BUTTON_B:
					exit(0);
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					move_selection(0, 1);
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					move_selection(-1, 0);
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					move_selection(1, 0);
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					move_selection(0, -1);
					break;
			}
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym)
			{
				case SDLK_DOWN:
					move_selection(0, 1);
					break;
				case SDLK_ESCAPE:
					exit(0);
				case SDLK_LEFT:
					move_selection(-1, 0);
					break;
				case SDLK_PAGEDOWN:
					scroll_ += window_height_;
					if (scroll_ > max_scroll_)
					{
						scroll_ = max_scroll_;
					}
					break;
				case SDLK_PAGEUP:
					scroll_ -= window_height_;
					if (scroll_ < 0)
					{
						scroll_ = 0;
					}
					break;
				case SDLK_RETURN:
					if (selection_ >= 0)
					{
						done_ = true;
					}
					break;
				case SDLK_RIGHT:
					move_selection(1, 0);
					break;
				case SDLK_TAB:
					if (e.key.keysym.mod & KMOD_SHIFT)
					{
						if (selection_ > 0)
						{
							--selection_;
							ensure_selection_visible();
						}
						else if (selection_ < 0)
						{
							selection_ = static_cast<int>(scenarios_.size()) - 1;
							ensure_selection_visible();
						}
					}
					else
					{
						if (selection_ < static_cast<int>(scenarios_.size()) - 1)
						{
							++selection_;
							ensure_selection_visible();
						}
					}
					break;
				case SDLK_UP:
					move_selection(0, -1);
					break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			auto x = e.button.x;
			auto y = e.button.y;

			for (auto i = 0; i < scenarios_.size(); ++i) {
				auto row = i / cols_;
				auto col = i % cols_;

				auto left = offsetx_ + margin + col * (scenario_width + margin);
				auto top = offsety_ + margin + row * (scenario_height + margin) - scroll_;

				if (x >= left && x < left + scenario_width &&
					y >= top && y <= top + scenario_height)
				{
					selection_ = i;
					done_ = true;
					break;
				}
			}
		}
			break;
		
		case SDL_MOUSEWHEEL:
			scroll_ -= e.wheel.y * 40;
			if (scroll_ < 0)
			{
				scroll_ = 0;
			}

			if (scroll_ > max_scroll_)
			{
				scroll_ = max_scroll_;
			}
			break;
		case SDL_QUIT:
			exit(0);
	}
}

void ScenarioChooser::move_selection(int col_delta, int row_delta)
{
	auto selection = selection_;
	if (selection == -1)
	{
		if (row_delta > 0 || col_delta > 0)
		{
			selection = 0;
			row_delta = 0;
			col_delta = 0;
		}
		else if (row_delta < 0)
		{
			selection = cols_ - 1 + (rows_ - 1) * cols_;
		}
		else
		{
			selection = static_cast<int>(scenarios_.size());
		}
	}
	
	auto col = selection % cols_;
	auto row = selection / cols_;
	
	col += col_delta;
	if (col < 0)
	{
		col = 0;
	}

	if (col > cols_ - 1)
	{
		col = cols_ - 1;
	}

	row += row_delta;
	if (row < 0)
	{
		row = 0;
	}

	if (row > rows_ - 1)
	{
		row = rows_ - 1;
	}

	selection = col + row * cols_;
	if (selection >= 0 && selection < static_cast<int>(scenarios_.size()))
	{
		selection_ = selection;
		ensure_selection_visible();
	}
}

void ScenarioChooser::optimize_image(ScenarioChooserScenario& scenario, SDL_Window* window)
{
	auto format = SDL_GetWindowSurface(window)->format;
	SurfacePtr optimized(SDL_ConvertSurface(scenario.image.get(), format, 0), SDL_FreeSurface);

	SDL_Rect src_rect{0, 0, optimized->w, optimized->h};
	SDL_Rect dst_rect{0, (scenario_height - optimized->h / 2) / 2, scenario_width, optimized->h / 2};

	scenario.image.reset(SDL_CreateRGBSurface(0, scenario_width, scenario_height, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask));

	SDL_SoftStretchLinear(optimized.get(), &src_rect, scenario.image.get(), &dst_rect);
}

void ScenarioChooser::redraw(SDL_Window* window)
{
	auto surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 23, 23, 23));

	for (auto i = 0; i < scenarios_.size(); ++i)
	{
		auto row = i / cols_;
		auto col = i % cols_;

		auto x = offsetx_ + margin + col * (scenario_width + margin);
		auto y = offsety_ + margin + row * (scenario_height + margin) - scroll_;

		if (y > window_height_ || y + scenario_height < 0)
		{
			continue;
		}

		if (i == selection_)
		{
			SDL_Rect r{x - 2, y - 2, scenario_width + 4, scenario_height + 4};
			SDL_FillRect(surface, &r, SDL_MapRGB(surface->format, 191, 191, 191));
		}

		SDL_Rect src_rect{0, 0, scenario_width, scenario_height};
		SDL_Rect dst_rect{x, y, scenario_width, scenario_height};
		
		SDL_BlitSurface(scenarios_[i].image.get(), &src_rect, surface, &dst_rect);
	}
	
	SDL_UpdateWindowSurface(window);
}

