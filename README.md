# ImGui Transfer Function

A transfer function widget implemented with ImGui. Supports loading in
arbitrary 1D RGBA8 images, which can then be used as color palettes.
The widget is used to modify the alpha channel as desired to modify
the transfer function.

## Use

Add the transfer function widget C++ and header files to your project,
along with the embedded presets header `embedded_colormaps.h`.
If you're not already using `stbi_image.h` add that file as well,
otherwise you can define `TFN_WIDGET_NO_STB_IMAGE_IMPL` to prevent
the transfer function widget C++ file from setting `STB_IMAGE_IMPLEMENTATION`.
You can also add `gl_core_4_5.h` and `gl_core_4_5.c` to your project,
or swap them for your preferred OpenGL function loader.

If you're not using OpenGL, you'll need to modify `TransferFunctionWidget::update_gpu_image`
to use the right API, and change how the image is passed to ImGui
to match what the ImGui backend expects in `TransferFunctionWidget::draw_ui`.

The widget includes some embedded color palette presets: ParaView's Cool Warm,
Rainbow, Matplotlib Plasma and Virdis, and
[Francesca Samsel's](https://sciviscolor.org/home/colormaps/) Linear Green
and Linear YGB 1211G colormaps. To load additional palettes you can add
colormaps with `TransferFunctionWidget::add_colormap`, which takes a `Colormap`.
The Colormap image should be a 1D RGBA8 image. 

## Example

See the [example/](example/) for an example use case of the widget
with OpenGL. Additional image files can be passed as arguments to load additional
presets beyond the included presets, the resulting colormap is drawn
with alpha blending as the window background. The example requires SDL2
which is found through CMake, if it fails to find it you can specify the
root directory of you SDL2 by passing `-DSDL2_DIR=<path>` when running CMake.

![Example image](https://i.imgur.com/piHEPEl.png)

