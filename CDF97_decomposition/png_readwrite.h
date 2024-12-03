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

int png_read(const wchar_t* filename, unsigned int& width, unsigned int& height, std::vector<uint8_t>& buffer)
{
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

    // create PNG decoder
    IWICBitmapDecoder* decoder = nullptr;
    hr = wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, 
        WICDecodeMetadataCacheOnDemand, &decoder);

    // create frame
    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    hr = frame->GetSize(&width, &height);

    buffer.resize(4 * width * height);
    hr = frame->CopyPixels(nullptr, 4 * width, 4 * width * height, buffer.data());

    SafeRelease(frame);
    SafeRelease(decoder);
    SafeRelease(wicFactory);
    CoUninitialize();
    return 0;
}


int png_write(int width, int height, std::vector<uint32_t>& buffer, const wchar_t* filename)
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

    std::vector<uint8_t> bytebuffer(4 * width * height);
    for (int i = 0; i < buffer.size(); ++i)
    {
        bytebuffer[4 * i] = buffer[i];
        bytebuffer[4 * i + 1] = buffer[i] / 256;
        bytebuffer[4 * i + 2] = buffer[i] / 256 / 256;
        bytebuffer[4 * i + 3] = 255;
    }

    hr = wicFactory->CreateBitmapFromMemory(width, height,
        GUID_WICPixelFormat32bppRGB, 4 * width,
        (unsigned int)bytebuffer.size(), bytebuffer.data(),
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
