#include <memory>
#include <sstream>
#include <iostream>
#include <tiffio.h>
#include <tiffio.hxx>
#include <pybind11/pybind11.h>
using namespace std;
namespace py = pybind11;


class Tiff
{
public:
    static Tiff frombytes(py::buffer bytes)
    {
        TIFFSetWarningHandler(nullptr);
        py::buffer_info info = bytes.request();

        auto sstream = new stringstream;
        sstream->write(reinterpret_cast<char*>(info.ptr), info.size);
        auto document = TIFFStreamOpen("file.tiff", static_cast<std::istream*>(sstream));
        if (!document)
            throw std::invalid_argument("invalid tiff file");

        return Tiff(document, sstream);
    }

    static Tiff fromfile(const string& filename)
    {
        TIFFSetWarningHandler(nullptr);
        auto document = TIFFOpen(filename.c_str(), "r");
        if (!document)
            throw std::invalid_argument("invalid tiff file");

        return Tiff(document, nullptr);
    }

    Tiff(const Tiff& other) = delete;

    Tiff(Tiff&& other):
        document(move(other.document)),
        num_pages(other.num_pages),
        sstream(move(other.sstream))
    {
        other.num_pages = 0;
    }

    Tiff& __enter__()
    {
        return *this;
    }

    void __exit__(py::object, py::object, py::object)
    {
        this->document.reset();
        this->sstream.reset();
        this->num_pages = 0;
    }

    py::object render_page(int page_index)
    {
        if (!this->document)
            throw std::runtime_error("Invalid tiff document.");

        if (page_index < 0 || page_index >= this->num_pages)
            throw std::invalid_argument("page index out of range");

        auto doc = this->document.get();

        TIFFSetDirectory(doc, page_index);

        int width, height;
        float dpix, dpiy;
        TIFFGetField(doc, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(doc, TIFFTAG_IMAGELENGTH, &height);

        auto image_data = new uint32_t[width * height];
        TIFFReadRGBAImageOriented(doc, width, height, image_data, 1, 0);
        auto size = make_pair(width, height);
        py::bytes buffer(reinterpret_cast<char*>(image_data), width * height * 4);
        py::module Image = py::module::import("PIL.Image");

        auto pil_image = Image.attr("frombytes")("RGBA", size, buffer, "raw", "BGRA");
        auto info = pil_image.attr("info");
        if (TIFFGetField(doc, TIFFTAG_XRESOLUTION, &dpix) && TIFFGetField(doc, TIFFTAG_YRESOLUTION, &dpiy))
            info["dpi"] = make_pair(int(dpix), int(dpiy));
        delete[] image_data;

        return pil_image;
    }

    size_t size() const
    {
        return this->num_pages;
    }

private:
    struct TIFFDeleter
    {
        void operator()(TIFF* tiff)
        {
            TIFFClose(tiff);
        }
    };

    Tiff(TIFF* document, stringstream* sstream):
        document(document), sstream(sstream)
    {
        _set_num_pages();
    }

    void _set_num_pages()
    {
        this->num_pages = 0;
        auto doc = this->document.get();

        do
        {
            this->num_pages++;
        } while (TIFFReadDirectory(doc));
    }

    unique_ptr<TIFF, TIFFDeleter> document;
    int num_pages = 0;
    unique_ptr<stringstream> sstream;
};


PYBIND11_MODULE(tiffrender, m)
{
    using namespace pybind11::literals;

    m.doc() = "TIFF rendering";

    const auto frombytes_docstring =
R"(Opens a TIFF file using the raw bytes of the file.

    Args:
        bytes: The raw bytes of the TIFF file.

    Returns:
        A Tiff object

    Raises:
        InvalidArgument exception if the file is invalid, or if the file is password-locked.
)";

    const auto fromfile_docstring =
R"(Opens a TIFF file given the file name.

    Args:
        filename: the filename to open.

    Returns:
        A Tiff object

    Raises:
        InvalidArgument exception if the file is invalid, or if the file is password-locked.
)";

    const auto render_page_docstring =
R"(Renders a page of the TIFF file as a PIL image.

    Args:
        page_index: 0-based index of the page to render

    Returns:
        The PIL image, or None if unsuccessful.
        The image can have mode 'L', 'RGB', or 'RGBA'
)";

    py::class_<Tiff>(m, "Tiff")
        .def_static("frombytes", &Tiff::frombytes, frombytes_docstring, "bytes"_a)
        .def_static("fromfile", &Tiff::fromfile, fromfile_docstring, "filename"_a)
        .def("__len__", &Tiff::size)
        .def("__enter__", &Tiff::__enter__)
        .def("__exit__", &Tiff::__exit__)
        .def("render_page", &Tiff::render_page, render_page_docstring, "page_index"_a);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#endif
}
