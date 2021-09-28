#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <algorithm>
#include "stb_image.hpp"
#include "stb_image_write.hpp"

static const std::vector<std::vector<std::vector<uint8_t>>> kernels
{
	std::vector<std::vector<uint8_t>>
	{
		{ 1, 1 },
		{ 1, 1 }
	},
	{
		{ 0, 1, 1, 0 },
		{ 1, 1, 1, 1 },
		{ 1, 1, 1, 1 },
		{ 0, 1, 1, 0 }
	},
	{
		{ 0, 0, 1, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 1, 1, 1, 1, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 1, 1, 1, 0 },
		{ 0, 0, 1, 1, 1, 1, 0, 0 },
	}
};

struct Pixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

int main()
{
	namespace fs = std::filesystem;
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
	fs::copy(fs::current_path() / "input", fs::current_path() / "output", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
	for (auto& file : fs::recursive_directory_iterator(fs::current_path() / "output"))
	{
		if (file.is_regular_file() && file.path().extension() == ".png")
		{
			Pixel* input{};
			int width{}, height{}, bpp{};
			input = reinterpret_cast<Pixel*>(stbi_load(file.path().string().c_str(), &width, &height, &bpp, 4));
			const int scale = 4;
			const int cust = 3;
			std::vector<Pixel> output(width * height * scale * scale);
			for (size_t i = 0; i < height; ++i)
			{
				for (size_t j = 0; j < width; ++j)
				{
					const Pixel p = input[i * width + j];
					for (size_t n = 0; n < scale; ++n)
					{
						for (size_t m = 0; m < scale; ++m)
						{
							output[(i * scale + n) * (width * scale) + (j * scale + m)] = p;
						}
					}
				}
			}
			std::vector<unsigned char> windowr(cust * cust);
			std::vector<unsigned char> windowg(cust * cust);
			std::vector<unsigned char> windowb(cust * cust);
			std::vector<unsigned char> windowa(cust * cust);
			const int edge = (cust - 1) / 2;
			const int custq = (cust * cust - 1) / 2;
			for (int i = edge; i < (height * scale) - edge; ++i)
			{
				for (int j = edge; j < (width * scale) - edge; ++j)
				{
					int c = 0;
					for (int fi = -edge; fi <= edge; ++fi)
					{
						int dpy = (fi + i) * (width * scale);
						for (int fj = -edge; fj <= edge; ++fj)
						{
							int dpx = dpy + fj + j;
							windowr[c] = output[dpx].r;
							windowg[c] = output[dpx].g;
							windowb[c] = output[dpx].b;
							windowa[c] = output[dpx].a;
							++c;
						}
					}
					std::sort(windowr.begin(), windowr.end());
					std::sort(windowg.begin(), windowg.end());
					std::sort(windowb.begin(), windowb.end());
					std::sort(windowa.begin(), windowa.end());
					output[i * (width * scale) + j].r = windowr[custq];
					output[i * (width * scale) + j].g = windowg[custq];
					output[i * (width * scale) + j].b = windowb[custq];
					output[i * (width * scale) + j].a = windowa[custq];
				}
			}
			{
				std::vector<unsigned char> pixels(width * height * scale * scale * bpp);
				for (size_t i = 0; i < (height * scale); ++i)
				{
					for (size_t j = 0; j < (width * scale); ++j)
					{
						if (kernels[1][i % scale][j % scale])
						{
							output[i * (width * scale) + j] = input[(i / scale * width) + (j / scale)];
						}
						pixels[(i * (width * scale) + j) * bpp] = output[i * (width * scale) + j].r;
						pixels[(i * (width * scale) + j) * bpp + 1] = output[i * (width * scale) + j].g;
						pixels[(i * (width * scale) + j) * bpp + 2] = output[i * (width * scale) + j].b;
						if (bpp == 4)
							pixels[(i * (width * scale) + j) * bpp + 3] = output[i * (width * scale) + j].a;
					}
				}
				stbi_write_png(file.path().string().c_str(), width * scale, height * scale, bpp, pixels.data(), 0);
			}
			stbi_image_free(input);
		}
	}
	return 0;
}