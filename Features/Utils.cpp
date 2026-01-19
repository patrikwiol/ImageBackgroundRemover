#include "precompiled.h"
#include "Utils.hpp"

constexpr COMDLG_FILTERSPEC ALL_FILE_TYPES[] =
{
	{ L"All Files", L"*.*" },
	{ L"Image Files", L"*.jpg;*.jpeg;*.png;*.bmp" },
};

std::wstring Utils::OpenFileExplorer(bool choosePath, EFileType fileType)
{
	std::wstring filePath = L"";

	HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(result))
		return filePath;

	IFileOpenDialog* openFile;
	result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&openFile));
	if (!SUCCEEDED(result))
	{
		CoUninitialize();
		return filePath;
	}

	if (!choosePath)
	{
		COMDLG_FILTERSPEC type = ALL_FILE_TYPES[fileType];
		openFile->SetFileTypes(1, &type);
	}

	DWORD flags;
	openFile->GetOptions(&flags);

	if (choosePath)
		flags |= FOS_PICKFOLDERS;

	openFile->SetOptions(flags | FOS_FORCEFILESYSTEM);

	result = openFile->Show(NULL);
	if (!SUCCEEDED(result))
	{
		openFile->Release();
		CoUninitialize();
		return filePath;
	}

	IShellItem* item;
	result = openFile->GetResult(&item);
	if (!SUCCEEDED(result))
	{
		openFile->Release();
		CoUninitialize();
		return filePath;
	}

	PWSTR resultFilePath;
	result = item->GetDisplayName(SIGDN_FILESYSPATH, &resultFilePath);
	if (!SUCCEEDED(result))
	{
		item->Release();
		openFile->Release();
		CoUninitialize();
		return filePath;
	}

	filePath = resultFilePath;
	CoTaskMemFree(resultFilePath);

	item->Release();
	openFile->Release();
	CoUninitialize();
	return filePath;
}

ID3D11ShaderResourceView* Utils::MatToTexture(ID3D11Device* device, cv::Mat& img)
{
	ID3D11ShaderResourceView* textureView = nullptr;
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = img.cols;
	desc.Height = img.rows;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = img.data;
	sd.SysMemPitch = (UINT)(img.cols * 4);

	ID3D11Texture2D* texture = nullptr;
	device->CreateTexture2D(&desc, &sd, &texture);
	if (!texture)
		return textureView;

	device->CreateShaderResourceView(texture, nullptr, &textureView);
	texture->Release();
	return textureView;
}

cv::Mat Utils::RemoveBackground(cv::Mat src, Options options)
{
	cv::TickMeter tm;

	tm.start();

	if (src.empty())
		return cv::Mat();

	if (options.removeSpecificColor)
	{
		cv::Mat hsv;
		cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

		cv::Scalar targetCol(options.specificColor.Value.x * 255.f, options.specificColor.Value.y * 255.f, options.specificColor.Value.z * 255.f);
		cv::Mat targetMat(1, 1, CV_8UC3, targetCol);
		cv::Mat targetHSV;
		cv::cvtColor(targetMat, targetHSV, cv::COLOR_RGB2HSV);
		cv::Vec3b hsvCenter = targetHSV.at<cv::Vec3b>(0, 0);

		float invAcc = 1.0f - options.accuracy;
		int hTol = (int)(180 * invAcc);
		int sTol = (int)(255 * invAcc * 1.5f);
		int vTol = (int)(255 * invAcc * 1.5f);

		cv::Scalar lower(hsvCenter[0] - hTol, hsvCenter[1] - sTol, hsvCenter[2] - vTol);
		cv::Scalar upper(hsvCenter[0] + hTol, hsvCenter[1] + sTol, hsvCenter[2] + vTol);

		int sMax = (int)(255 * (1.0f - options.accuracy));
		int vMin = (int)(255 * options.accuracy);

		cv::Mat colorMask;
		cv::inRange(hsv, cv::Scalar(0, 0, vMin), cv::Scalar(180, sMax, 255), colorMask);

		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::morphologyEx(colorMask, colorMask, cv::MORPH_CLOSE, kernel);
		cv::morphologyEx(colorMask, colorMask, cv::MORPH_OPEN, kernel);

		cv::Mat finalMask;
		cv::bitwise_not(colorMask, finalMask);
		cv::GaussianBlur(finalMask, finalMask, cv::Size(3, 3), 0);

		if (options.removeOutline)
		{
			cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * options.erosionSize + 1, 2 * options.erosionSize + 1));
			cv::erode(finalMask, finalMask, element);
		}

		std::vector<cv::Mat> bgrChannels;
		cv::split(src, bgrChannels);
		bgrChannels.push_back(finalMask);

		cv::Mat resultBGRA;
		cv::merge(bgrChannels, resultBGRA);

		tm.stop();
		elapsedTime = tm.getTimeSec();

		return resultBGRA;
	}

	cv::Mat newMat = src;
	if (options.downscaleImage)
		cv::resize(src, newMat, cv::Size(), 0.5, 0.5);

	cv::Rect rect(5, 5, newMat.cols - 10, newMat.rows - 10);
	cv::Mat bgModel, fgModel, mask;

	cv::grabCut(newMat, mask, rect, bgModel, fgModel, options.iterations, cv::GC_INIT_WITH_RECT);
	cv::Mat binaryMask = (mask == cv::GC_PR_FGD) | (mask == cv::GC_FGD);

	if (options.downscaleImage)
		cv::resize(binaryMask, binaryMask, src.size(), 0, 0, cv::INTER_NEAREST);

	cv::Mat finalMask = binaryMask;
	if (options.removeOutline)
	{
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * options.erosionSize + 1, 2 * options.erosionSize + 1));
		cv::erode(binaryMask, binaryMask, element);

		binaryMask.convertTo(finalMask, CV_32F, 1. / 255.);
		cv::GaussianBlur(finalMask, finalMask, cv::Size(3, 3), 0);
	}

	cv::Mat foreground;
	src.copyTo(foreground, binaryMask);

	cv::Mat bgra;
	cv::cvtColor(foreground, bgra, cv::COLOR_BGR2BGRA);

	if (options.removeOutline && !options.removeSpecificColor)
	{
		for (int y = 0; y < bgra.rows; ++y)
		{
			cv::Vec4b* pBgra = bgra.ptr<cv::Vec4b>(y);
			const float* pAlpha = finalMask.ptr<float>(y);

			for (int x = 0; x < bgra.cols; ++x)
			{
				float alpha = pAlpha[x];

				pBgra[x][3] = static_cast<uchar>(alpha * 255.f);

				if (alpha < 0.9f && alpha > 0.1f)
				{
					pBgra[x][0] = static_cast<uchar>(pBgra[x][0] * alpha);
					pBgra[x][1] = static_cast<uchar>(pBgra[x][1] * alpha);
					pBgra[x][2] = static_cast<uchar>(pBgra[x][2] * alpha);
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < bgra.rows; ++y)
		{
			for (int x = 0; x < bgra.cols; ++x)
			{
				if (finalMask.at<uchar>(y, x) == 0)
					bgra.at<cv::Vec4b>(y, x)[3] = 0;
			}
		}
	}

	tm.stop();
	elapsedTime = tm.getTimeSec();

	return bgra;
}