#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "imgui.h"

#include "gl_core_4_5.h"

struct Colormap {
	std::string name;
	// An RGBA8 1D image
	std::vector<uint8_t> colormap;

	Colormap(const std::string &name, const std::vector<uint8_t> &img);
};

class TransferFunctionWidget {
	struct vec2f {
		float x, y;

		vec2f(float c = 0.f);
		vec2f(float x, float y);
		vec2f(const ImVec2 &v);

		float length() const;

		vec2f operator+(const vec2f &b) const;
		vec2f operator-(const vec2f &b) const;
		vec2f operator/(const vec2f &b) const;
		vec2f operator*(const vec2f &b) const;
		operator ImVec2() const;
	};

	std::vector<Colormap> colormaps;
	size_t selected_colormap = 0;
	std::vector<uint8_t> current_colormap;

	std::vector<vec2f> alpha_control_pts = {vec2f(0.f), vec2f(1.f)};
	size_t selected_point = -1;

	bool colormap_changed = true;
	GLuint colormap_img = -1;

public:
	TransferFunctionWidget();

	// Add a colormap preset. The image should be a 1D RGBA8 image
	void add_colormap(const Colormap &map);
	// Add the transfer function UI into the currently active window
	void draw_ui();
	// Returns true if the colormap was updated since the last
	// call to draw_ui
	bool changed() const;
	// Get back the RGBA8 color data for the transfer function
	std::vector<uint8_t> get_colormap();

private:
	void update_gpu_image();
	void update_colormap();

	void load_embedded_preset(const uint8_t *buf, size_t size,
			const std::string &name);
};

