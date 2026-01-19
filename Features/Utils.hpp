#pragma once

namespace Utils
{
	enum EFileType
	{
		FT_ALL,
		FT_IMAGE
	};

	struct Options
	{
		bool removeSpecificColor = false;
		ImColor specificColor = ImColor(255, 255, 255, 255);
		float accuracy = 0.5f;
		bool removeOutline = true;
		int erosionSize = 2;
		bool downscaleImage = true;
		int iterations = 5;
	};

	inline Options options{};

	inline double elapsedTime = 0.;

	std::wstring OpenFileExplorer(bool choosePath, EFileType fileType = FT_ALL);
	ID3D11ShaderResourceView* MatToTexture(ID3D11Device* device, cv::Mat& img);
	cv::Mat RemoveBackground(cv::Mat src, Options options);
}