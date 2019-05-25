#include <iostream>
#include <algorithm>
#include "transfer_function_widget.h"
#include "embedded_colormaps.h"

#ifndef TFN_WIDGET_NO_STB_IMAGE_IMPL
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "stb_image.h"

template<typename T>
T clamp(T x, T min, T max) {
	if (x < min) {
		return min;
	}
	if (x > max) {
		return max;
	}
	return x;
}

Colormap::Colormap(const std::string &name, const std::vector<uint8_t> &img)
	: name(name), colormap(img)
{}

TransferFunctionWidget::vec2f::vec2f(float c) : x(c), y(c) {}
TransferFunctionWidget::vec2f::vec2f(float x, float y) : x(x), y(y) {}
TransferFunctionWidget::vec2f::vec2f(const ImVec2 &v) : x(v.x), y(v.y) {}

float TransferFunctionWidget::vec2f::length() const {
	return std::sqrt(x * x + y * y);
}

TransferFunctionWidget::vec2f
TransferFunctionWidget::vec2f::operator+(const TransferFunctionWidget::vec2f &b) const {
	return vec2f(x + b.x, y + b.y);
}
TransferFunctionWidget::vec2f
TransferFunctionWidget::vec2f::operator-(const TransferFunctionWidget::vec2f &b) const {
	return vec2f(x - b.x, y - b.y);
}
TransferFunctionWidget::vec2f
TransferFunctionWidget::vec2f::operator/(const TransferFunctionWidget::vec2f &b) const {
	return vec2f(x / b.x, y / b.y);
}
TransferFunctionWidget::vec2f
TransferFunctionWidget::vec2f::operator*(const TransferFunctionWidget::vec2f &b) const {
	return vec2f(x * b.x, y * b.y);
}
TransferFunctionWidget::vec2f::operator ImVec2() const {
	return ImVec2(x, y);
}

TransferFunctionWidget::TransferFunctionWidget() {
	// Load up the embedded colormaps as the default options
	load_embedded_preset(paraview_cool_warm, sizeof(paraview_cool_warm),
			"ParaView Cool Warm");
	load_embedded_preset(rainbow, sizeof(rainbow),
			"Rainbow");
	load_embedded_preset(matplotlib_plasma, sizeof(matplotlib_plasma),
			"Matplotlib Plasma");
	load_embedded_preset(matplotlib_virdis, sizeof(matplotlib_virdis),
			"Matplotlib Virds");
	load_embedded_preset(samsel_linear_green, sizeof(samsel_linear_green),
			"Samsel Linear Green");
	load_embedded_preset(samsel_linear_ygb_1211g, sizeof(samsel_linear_ygb_1211g),
			"Samsel Linear YGB 1211G");

	// Initialize the colormap alpha channel w/ a linear ramp
	update_colormap();
}

void TransferFunctionWidget::add_colormap(const Colormap &map) {
	colormaps.push_back(map);
}

void TransferFunctionWidget::draw_ui() {
	update_gpu_image();
	colormap_changed = false;

	const ImGuiIO &io = ImGui::GetIO();

	ImGui::Text("Transfer Function");
	ImGui::TextWrapped("Left click to add a point, right click remove. "
			"Left click + drag to move points.");

	if (ImGui::BeginCombo("Colormap", colormaps[selected_colormap].name.c_str())) {
		for (size_t i = 0; i < colormaps.size(); ++i) {
			if (ImGui::Selectable(colormaps[i].name.c_str(), selected_colormap == i)) {
				selected_colormap = i;
				update_colormap();
			}
		}
		ImGui::EndCombo();
	}

	vec2f canvas_size = ImGui::GetContentRegionAvail();
	// Note: If you're not using OpenGL for rendering your UI, the setup for
	// displaying the colormap texture in the UI will need to be updated.
	ImGui::Image(reinterpret_cast<void*>(colormap_img), ImVec2(canvas_size.x, 16));
	vec2f canvas_pos = ImGui::GetCursorScreenPos();
	canvas_size.y -= 80;

	const float point_radius = 10.f;

	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

	const vec2f view_scale(canvas_size.x, -canvas_size.y);
	const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);

	draw_list->AddRect(canvas_pos, canvas_pos + canvas_size,
			ImColor(180, 180, 180, 255));

	ImGui::InvisibleButton("tfn_canvas", canvas_size);
	if (ImGui::IsItemHovered()) {
		vec2f mouse_pos = (vec2f(io.MousePos) - view_offset) / view_scale;
		mouse_pos.x = clamp(mouse_pos.x, 0.f, 1.f);
		mouse_pos.y = clamp(mouse_pos.y, 0.f, 1.f);

		if (io.MouseDown[0]) {
			if (selected_point != (size_t)-1) {
				alpha_control_pts[selected_point] = mouse_pos;

				// Keep the first and last control points at the edges
				if (selected_point == 0) {
					alpha_control_pts[selected_point].x = 0.f;
				} else if (selected_point == alpha_control_pts.size() - 1) {
					alpha_control_pts[selected_point].x = 1.f;
				}
			} else {
				// See if we're selecting a point or adding one
				if (io.MousePos.x - canvas_pos.x <= point_radius) {
					selected_point = 0;
				} else if (io.MousePos.x - canvas_pos.x >= canvas_size.x - point_radius) {
					selected_point = alpha_control_pts.size() - 1;
				} else {
					auto fnd = std::find_if(alpha_control_pts.begin(), alpha_control_pts.end(),
						[&](const vec2f &p) {
							const vec2f pt_pos = p * view_scale + view_offset;
							float dist = (pt_pos - vec2f(io.MousePos)).length();
							return dist <= point_radius;
						});
					// No nearby point, we're adding a new one
					if (fnd == alpha_control_pts.end()) {
						alpha_control_pts.push_back(mouse_pos);
					}
				}
			}
			// Keep alpha control points ordered by x coordinate, update selected
			// point index to match
			std::sort(alpha_control_pts.begin(), alpha_control_pts.end(),
				[](const vec2f &a, const vec2f &b) {
					return a.x < b.x;
				});
			if (selected_point != 0 && selected_point != alpha_control_pts.size() - 1) {
				auto fnd = std::find_if(alpha_control_pts.begin(), alpha_control_pts.end(),
					[&](const vec2f &p) {
						const vec2f pt_pos = p * view_scale + view_offset;
						float dist = (pt_pos - vec2f(io.MousePos)).length();
						return dist <= point_radius;
					});
				selected_point = std::distance(alpha_control_pts.begin(), fnd);
			}
			update_colormap();
		} else if (ImGui::IsMouseClicked(1)) {
			selected_point = -1;
			// Find and remove the point
			auto fnd = std::find_if(alpha_control_pts.begin(), alpha_control_pts.end(),
				[&](const vec2f &p) {
					const vec2f pt_pos = p * view_scale + view_offset;
					float dist = (pt_pos - vec2f(io.MousePos)).length();
					return dist <= point_radius;
				});
			// We also want to prevent erasing the first and last points
			if (fnd != alpha_control_pts.end()
				&& fnd != alpha_control_pts.begin() && fnd != alpha_control_pts.end() - 1)
			{
				alpha_control_pts.erase(fnd);
			}
			update_colormap();
		} else {
			selected_point = -1;
		}
	}

	// Draw the alpha control points, and build the points for the polyline
	// which connects them
	std::vector<ImVec2> polyline_pts;
	for (const auto &pt : alpha_control_pts) {
		const vec2f pt_pos = pt * view_scale + view_offset;
		polyline_pts.push_back(pt_pos);
		draw_list->AddCircleFilled(pt_pos, point_radius, 0xFFFFFFFF);
	}
	draw_list->AddPolyline(polyline_pts.data(), polyline_pts.size(), 0xFFFFFFFF, false, 2.f);
	draw_list->PopClipRect();
}

bool TransferFunctionWidget::changed() const {
	return colormap_changed;
}

std::vector<uint8_t> TransferFunctionWidget::get_colormap() {
	return current_colormap;
}

void TransferFunctionWidget::update_gpu_image() {
	GLint prev_tex_2d = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex_2d);

	if (colormap_img == (GLuint)-1) {
		glGenTextures(1, &colormap_img);
		glBindTexture(GL_TEXTURE_2D, colormap_img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	if (colormap_changed) {
		glBindTexture(GL_TEXTURE_2D, colormap_img);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, current_colormap.size() / 4, 1, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, current_colormap.data());
	}
	if (prev_tex_2d != 0) {
		glBindTexture(GL_TEXTURE_2D, prev_tex_2d);
	}
}

void TransferFunctionWidget::update_colormap() {
	colormap_changed = true;
	current_colormap = colormaps[selected_colormap].colormap;
	// We only change opacities for now, so go through and update the opacity
	// by blending between the neighboring control points
	auto a_it = alpha_control_pts.begin();
	const size_t npixels = current_colormap.size() / 4;
	for (size_t i = 0; i < npixels; ++i) {
		float x = static_cast<float>(i) / npixels;
		auto high = a_it + 1;
		if (x > high->x) {
			++a_it;
			++high;
		}
		float t = (x - a_it->x) / (high->x - a_it->x);
		float alpha = (1.f - t) * a_it->y + t * high->y;
		current_colormap[i * 4 + 3] = static_cast<uint8_t>(clamp(alpha * 255.f, 0.f, 255.f));
	}
}
void TransferFunctionWidget::load_embedded_preset(const uint8_t *buf, size_t size,
		const std::string &name)
{
	int w, h, n;
	uint8_t *img_data = stbi_load_from_memory(buf, size, &w, &h, &n, 4);
	auto img = std::vector<uint8_t>(img_data, img_data + w * 1 * 4);
	stbi_image_free(img_data);
	colormaps.emplace_back(name, img);
}

