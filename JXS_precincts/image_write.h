#pragma once
#include <wincodec.h>
#include <vector>

template <typename T>
inline void SafeRelease(T*& p)
{
    if (NULL != p)
    {
        p->Release();
        p = NULL;
    }
}

int image_write(int width, int height, std::vector<uint8_t>& buffer, const wchar_t* filename)
{
    if (buffer.size() != width * height)
        return -1;
    IWICImagingFactory* wicFactory = nullptr;
    HRESULT hr = S_OK;
    hr = CoInitialize(nullptr);
    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );
    IWICBitmap* bitmap = nullptr;

    hr = wicFactory->CreateBitmapFromMemory(width, height,
        GUID_WICPixelFormat8bppGray, width,
        (unsigned int)buffer.size(), buffer.data(),
        &bitmap);

    IWICStream* wicStream = nullptr;
    hr = wicFactory->CreateStream(&wicStream);
    hr = wicStream->InitializeFromFilename(filename, GENERIC_WRITE);

    // create PNG encoder
    IWICBitmapEncoder* encoder = nullptr;
    hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
    hr = encoder->Initialize(wicStream, WICBitmapEncoderNoCache);

    // create frame
    IWICBitmapFrameEncode* frame = nullptr;
    hr = encoder->CreateNewFrame(&frame, nullptr);
    hr = frame->Initialize(nullptr);

    // write bitmap
    hr = frame->WriteSource(bitmap, nullptr);

    // commit
    hr = frame->Commit();
    hr = encoder->Commit();
    SafeRelease(frame);
    SafeRelease(encoder);
    SafeRelease(wicStream);
    SafeRelease(bitmap);
    SafeRelease(wicFactory);
    CoUninitialize();
    return 0;
}