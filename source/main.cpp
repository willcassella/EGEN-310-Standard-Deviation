// main.cpp

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <FreeImage.h>

/** Structure representing a pixel. */
struct Pixel final
{
	/////////////////
	///   Types   ///
public:

	/** Floating-point precision to use. */
	using PrecisionT = double;

	////////////////////////
	///   Constructors   ///
public:

	Pixel()
		: R{ 0 }, G{ 0 }, B{ 0 }
	{
		// All done
	}
	Pixel(PrecisionT r, PrecisionT g, PrecisionT b)
		: R{ r }, G{ g }, B{ b }
	{
		// All done
	}
	Pixel(RGBQUAD fiPixel)
	{
		constexpr auto FIPixelMax = std::numeric_limits<BYTE>::max();
		
		this->R = static_cast<PrecisionT>(fiPixel.rgbRed) / FIPixelMax;
		this->G = static_cast<PrecisionT>(fiPixel.rgbGreen) / FIPixelMax;
		this->B = static_cast<PrecisionT>(fiPixel.rgbBlue) / FIPixelMax;
	}

	//////////////////
	///   Fields   ///
public:

	PrecisionT R = 0;
	PrecisionT G = 0;
	PrecisionT B = 0;

	/////////////////////
	///   Operators   ///
public:

	friend Pixel operator+(const Pixel& lhs, const Pixel& rhs)
	{
		return{ lhs.R + rhs.R, lhs.G + rhs.G, lhs.B + rhs.B };
	}
	Pixel& operator+=(const Pixel& rhs)
	{
		*this = *this + rhs;
		return *this;
	}
	friend Pixel operator-(const Pixel& lhs, const Pixel& rhs)
	{
		return{ lhs.R - rhs.R, lhs.G - rhs.G, lhs.B - rhs.B };
	}
	Pixel& operator-=(const Pixel& rhs)
	{
		*this = *this - rhs;
		return *this;
	}

	template <typename I>
	friend Pixel operator/(const Pixel& lhs, I rhs)
	{
		return{ lhs.R / rhs, lhs.G / rhs, lhs.B / rhs };
	}
	template <typename I>
	Pixel& operator/=(I rhs)
	{
		*this = *this / rhs;
		return *this;
	}
	template <typename I>
	friend Pixel operator*(const Pixel& lhs, I rhs)
	{
		return{ lhs.R * rhs, lhs.G * rhs, lhs.B * rhs };
	}
	template <typename I>
	Pixel& operator*=(I rhs)
	{
		*this = *this * rhs;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& stream, const Pixel& pixel)
	{
		stream << "{ R: " << pixel.R << ", G: " << pixel.G << ", B: " << pixel.B << " }";
		return stream;
	}
};

/** Calculates the standard deviation of the pixels in an image. */
Pixel StandardDeviation(FIBITMAP* image)
{
	// Figure out dimensions of image
	const auto width = FreeImage_GetWidth(image);
	const auto height = FreeImage_GetHeight(image);
	const auto totalPixels = width * height;

	// Mean color of pixels
	Pixel mean;

	// Vector of pixels
	// NOTE: Storing RGBQUAD instead of Pixel for better cache performance
	std::vector<RGBQUAD> pixels;
	pixels.reserve(totalPixels);

	// Iterate over pixels
	for (unsigned y = 0; y < height; ++y)
	{
		for (unsigned x = 0; x < width; ++x)
		{
			// Get the pixel color
			RGBQUAD fiPixel;
			FreeImage_GetPixelColor(image, x, y, &fiPixel);
			Pixel pixel{ fiPixel };

			// Add it to the vector of pixels
			pixels.push_back(fiPixel);

			// Add it to the mean
			mean += pixel / totalPixels;
		}
	}

	// The squared difference of the pixels and the mean
	Pixel squaredDifference;

	// Calculate the squared difference
	for (Pixel pixel : pixels)
	{
		// Subtract the mean
		pixel -= mean;

		// Square it
		pixel.R = std::pow(pixel.R, 2);
		pixel.G = std::pow(pixel.G, 2);
		pixel.B = std::pow(pixel.B, 2);

		// Add it to the squared difference
		squaredDifference += pixel / totalPixels;
	}

	// The standard deviation result
	Pixel result = squaredDifference;
	result.R = std::sqrt(result.R);
	result.G = std::sqrt(result.G);
	result.B = std::sqrt(result.B);

	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "No path to image given, please use as: path_to_image" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Load the image
	const auto format = FreeImage_GetFileType(argv[1]);
	auto* image = FreeImage_Load(format, argv[1], 0);

	// Calculate the standard deviation of the image
	auto standardDeviation = StandardDeviation(image);

	// Unload image
	FreeImage_Unload(image);

	// Print the standard deviation
	std::cout << "Standard deviation: " << standardDeviation << std::endl;

	// Calculate homogeneity
	auto homogeneity = standardDeviation.R + standardDeviation.G + standardDeviation.B;
	homogeneity /= 3;
	homogeneity = 1.0 - homogeneity;

	// Print the homogeneity
	std::cout << homogeneity * 100 << "% homogeneity" << std::endl;
}
