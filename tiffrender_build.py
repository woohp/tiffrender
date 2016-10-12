from cffi import FFI
ffi = FFI()

with open('src/tiffrender.cpp') as f:
    ffi.set_source('tiffrender._libtiffrender', f.read(),
                   extra_compile_args=['-std=c++1y'],
                   libraries=['tiff', 'tiffxx'],
                   source_extension='.cpp')

ffi.cdef('''
void* load_stream_from_bytes(const char* file_data, int file_data_length);
void* load_tiff_from_stream(void* stream);
void* load_tiff_from_file(const char* filename);
void delete_stream(void* stream);
void delete_tiff(void* tiff);
int num_pages(void* tiff);
const uint32_t* render_page(void* doc, int index, int* height, int* width, float* dpix, float* dpiy);
void delete_image_data(uint32_t* data);
''')


if __name__ == '__main__':
    ffi.compile()
