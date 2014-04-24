#include <windows.h>
#include <wincodec.h>
#include <assert.h>
#include <iostream>

#ifndef WINVER                  // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600        // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers


#define HR(hr, errstr) \
    if(FAILED(hr)) { \
        std::cerr << errstr << std::endl; \
        printHresultError(hr); \
        return EXIT_FAILURE; \
    } \


template <typename T>
inline void SafeRelease(T *&p)
{
    if (NULL != p) {
        p->Release();
        p = NULL;
    }
}


void printHresultError(const HRESULT hr)
{
    LPTSTR errorText = NULL;

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
       NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);

    if ( NULL != errorText ) {
        std::cerr << errorText << std::endl;
        LocalFree(errorText);
        errorText = NULL;
    }
}


unsigned GetStride(const unsigned width, const unsigned bitsPerPixel)
{
  assert(0 == bitsPerPixel % 8);

  const unsigned byteCount = bitsPerPixel / 8;
  const unsigned stride = (width * byteCount + 3) & ~3;

  assert(0 == stride % sizeof(DWORD));
  return stride;
}


int main(int argc, char *argv[])
{
    // check args
    if(argc != 3) {
        std::cerr << "Wrong args count. Usage:\n\ndng2jpeg <input_file> <output_file>" << std::endl;
        return EXIT_FAILURE;
    }

    setlocale( LC_ALL, "" );

    HRESULT hr = S_OK;

    // initialize COM
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    HR(hr, "Can't init COM");

    // Create WIC factory
    IWICImagingFactory *factory = NULL;
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    HR(hr, "Can't create IWICImagingFactory");

    //Decode the source image to IWICBitmapSource
    //Create a decoder
    IWICBitmapDecoder *decoder = NULL;
    std::wstring inFileName;
    mbstowcs( &inFileName[0], argv[1], strlen(argv[1])+1);
    hr = factory->CreateDecoderFromFilename(inFileName.data(), NULL, GENERIC_READ,
                                            WICDecodeMetadataCacheOnDemand, &decoder);
    HR(hr, "Can't create IWICBitmapDecoder");

    //Retrieve the first frame of the image from the decoder
    IWICBitmapFrameDecode *sourceFrame = NULL;
    hr = decoder->GetFrame(0, &sourceFrame);
    HR(hr, "Can't get frame");


    // create JPEG encoder
    IWICBitmapEncoder* encoder = NULL;
    hr = factory->CreateEncoder(GUID_ContainerFormatJpeg, NULL, &encoder);
    HR(hr, "Can't create encoder");

    IWICStream *outStream = NULL;
    hr = factory->CreateStream(&outStream);
    HR(hr, "Can't create stream");

    wchar_t* outFileName = new wchar_t[strlen(argv[2])+1];
    mbstowcs(outFileName, argv[2], strlen(argv[2])+1);
    hr = outStream->InitializeFromFilename(outFileName, GENERIC_WRITE);
    delete outFileName;
    HR(hr, std::string("Can't InitializeFromFilename ") + argv[2]);

    hr = encoder->Initialize(outStream, WICBitmapEncoderNoCache);
    HR(hr, "Can't initialize encoder");

    UINT width = 0;
    UINT height = 0;
    hr = sourceFrame->GetSize(&width, &height);
    HR(hr, "Can't get size from source frame");

    GUID pixelFormat = { 0, 0, 0, 0 };
    hr = sourceFrame->GetPixelFormat(&pixelFormat);
    HR(hr, "Can't get pixel format from source frame");

    // Prepare the target frame
    IWICBitmapFrameEncode *targetFrame = NULL;
    hr = encoder->CreateNewFrame(&targetFrame, 0);
    HR(hr, "Can't create target frame");

    hr = targetFrame->Initialize(0);
    HR(hr, "Can't init target frame");

    hr = targetFrame->SetSize(width, height);
    HR(hr, "Can't setSize to target frame");

    hr = targetFrame->SetPixelFormat(&pixelFormat);
    HR(hr, "Can't setPixelFormat to target frame");

    // Copy the pixels and commit frame
    hr = targetFrame->WriteSource(sourceFrame, 0);
    HR(hr, "Can't write source to target frame");

    hr = targetFrame->Commit();
    HR(hr, "Can't commit to target frame");

    // Commit image to stream
    hr = encoder->Commit();
    HR(hr, "Can't commit to encoder");

    SafeRelease(factory);
    SafeRelease(decoder);
    SafeRelease(encoder);
    SafeRelease(sourceFrame);
    SafeRelease(targetFrame);
    SafeRelease(outStream);

    return EXIT_SUCCESS;
}
