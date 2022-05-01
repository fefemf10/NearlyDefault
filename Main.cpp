#include <fstream>
#include <vector>
#include <map>
#include <filesystem>
#include <string>
#include <algorithm>
#include "stb_image.hpp"
#include "stb_image_write.hpp"

std::map<size_t, std::vector<std::vector<std::vector<uint8_t>>>> kernels;

struct Pixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

int main(int argc, char* argv[])
{
	namespace fs = std::filesystem;
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
	std::ifstream kernelsFile("kernels.txt");
	while (!kernelsFile.eof())
	{
		int size;
		kernelsFile >> size;
		kernels[size].push_back(std::vector<std::vector<uint8_t>>(size, std::vector<uint8_t>(size)));
		for (size_t i = 0; i < size; i++)
		{
			for (size_t j = 0; j < size; j++)
			{
				kernelsFile >> (*kernels[size].rbegin())[i][j];
			}
		}
	}
	int scale{2}, kernelVariant{};
	bool invert{};
	if (argc > 1)
	{
		scale = std::stoi(argv[1]);
		kernelVariant = std::stoi(argv[2]);
		invert = std::stoi(argv[3]);
	}
	for (auto& file : fs::recursive_directory_iterator(fs::current_path() / "input"))
	{
		if (file.is_regular_file())
		{
			Pixel* input{};
			int width{}, height{}, bpp{};
			input = reinterpret_cast<Pixel*>(stbi_load(file.path().string().c_str(), &width, &height, &bpp, 4));
			
			const int kernelSize = scale;
			
			const int window = scale + 1 + scale % 2;
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
			std::vector<unsigned char> windowr(window * window);
			std::vector<unsigned char> windowg(window * window);
			std::vector<unsigned char> windowb(window * window);
			std::vector<unsigned char> windowa(window * window);
			const int edge = (window - 1) / 2;
			const int median = (window * window - 1) / 2;
			for (int i = edge; i < (height * scale) - edge; ++i)
			{
				for (int j = edge; j < (width * scale) - edge; ++j)
				{
					int c = 0;
					for (int fi = -edge; fi <= edge; ++fi)
					{
						const int dpy = (fi + i) * (width * scale);
						for (int fj = -edge; fj <= edge; ++fj)
						{
							const int dpx = dpy + fj + j;
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
					const unsigned int index = i * width * scale + j;
					output[index].r = windowr[median];
					output[index].g = windowg[median];
					output[index].b = windowb[median];
					output[index].a = windowa[median];
				}
			}
			{
				std::vector<unsigned char> pixels(width * height * scale * scale * bpp);
				for (size_t i = 0; i < (height * scale); ++i)
				{
					for (size_t j = 0; j < (width * scale); ++j)
					{
						const unsigned int index = i * width * scale + j;
						if (kernels[kernelSize][kernelVariant][i % scale][j % scale] == (invert ? '0' : '1'))
						{
							output[index] = input[(i / scale * width) + (j / scale)];
						}
						pixels[index * bpp] = output[index].r;
						pixels[index * bpp + 1] = output[index].g;
						pixels[index * bpp + 2] = output[index].b;
						if (bpp == 4)
							pixels[index * bpp + 3] = output[index].a;
					}
				}
				fs::path tmp = fs::current_path() / "output" / file.path().stem();
				tmp.replace_extension("png");
				stbi_write_png(tmp.string().c_str(), width * scale, height * scale, bpp, pixels.data(), 0);
			}
			stbi_image_free(input);
		}
	}
	return 0;
}