#include <memory>
#include <sstream>
#include <tiffio.h>
#include <tiffio.hxx>

extern "C"
{

void* load_stream_from_bytes(const char* file_data, int file_data_length)
{
    std::stringstream* stream = new std::stringstream();
    stream->write(file_data, file_data_length);
    return stream;
}

void* load_tiff_from_stream(void* stream)
{
    TIFFSetWarningHandler(nullptr);
    return TIFFStreamOpen("file.tiff", static_cast<std::istream*>(stream));
}

void* load_tiff_from_file(const char* filename)
{
    TIFFSetWarningHandler(nullptr);
    return TIFFOpen(filename, "r");
}

void delete_tiff(void* doc)
{
    TIFFClose(reinterpret_cast<TIFF*>(doc));
}

void delete_stream(void* stream)
{
    delete reinterpret_cast<std::stringstream*>(stream);
}

int num_pages(void* _tiff)
{
    auto tiff = reinterpret_cast<TIFF*>(_tiff);
    int count = 0;

    do
    {
        count++;
    } while (TIFFReadDirectory(tiff));

    return count;
}

const uint32_t* render_page(void* _tiff, int index, int* height, int* width, float* dpix, float* dpiy)
{
    auto tiff = reinterpret_cast<TIFF*>(_tiff);
    TIFFSetDirectory(tiff, index);

    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, height);
    TIFFGetField(tiff, TIFFTAG_XRESOLUTION, dpix);
    TIFFGetField(tiff, TIFFTAG_YRESOLUTION, dpiy);

    auto image_data = reinterpret_cast<uint32_t*>(_TIFFmalloc(*width * *height * 4));
    TIFFReadRGBAImageOriented(tiff, *width, *height, image_data, 1, 0);
    return image_data;
}

void delete_image_data(uint32_t* data)
{
    _TIFFfree(reinterpret_cast<void*>(data));
}

}
