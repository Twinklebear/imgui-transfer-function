#pragma once

#include <cstdint>
#include <vector>
#include "imgui.h"

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

	std::vector<uint8_t> colormap;
	std::vector<vec2f> alpha_control_pts = {vec2f(0.f), vec2f(1.f)};
	size_t selected_point = -1;
	bool colormap_changed = true;
	GLuint colormap_img = -1;

public:
	TransferFunctionWidget();

	// Add the transfer function UI into the currently active window
	void draw_ui();
	// Returns true if the colormap was updated since the last
	// call to draw_ui
	bool changed() const;
	// Get back the RGBA8 color data for the transfer function
	std::vector<uint8_t> get_colormap();

private:
	void update_colormap();
};
