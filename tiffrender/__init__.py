from PIL import Image
from _libtiffrender import ffi, lib


class Tiff(object):
    def __init__(self):
        self.height_buffer = ffi.new('int*')
        self.width_buffer = ffi.new('int*')
        self.dpix_buffer = ffi.new('float*')
        self.dpiy_buffer = ffi.new('float*')

    @staticmethod
    def from_bytes(data):
        data = bytearray(data)
        stream = lib.load_stream_from_bytes(
            ffi.from_buffer(data),
            ffi.cast('int', len(data))
        )
        tiff_object = lib.load_tiff_from_stream(stream)
        if tiff_object == ffi.NULL:
            raise ValueError('invalid tiff data')

        stream = ffi.gc(stream, lib.delete_stream)
        tiff_object = ffi.gc(tiff_object, lib.delete_tiff)

        tiff = Tiff()
        tiff.tiff_object = tiff_object
        tiff._stream = stream
        tiff.num_pages = lib.num_pages(tiff_object)
        return tiff

    @staticmethod
    def from_file(filename):
        c_filename = ffi.new('char[]', filename)
        tiff_object = lib.load_tiff_from_file(c_filename)
        if tiff_object == ffi.NULL:
            raise ValueError('invalid tiff data')

        tiff_object = ffi.gc(tiff_object, lib.delete_tiff)

        tiff = Tiff()
        tiff.tiff_object = tiff_object
        tiff.num_pages = lib.num_pages(tiff_object)
        return tiff

    def __len__(self):
        return self.num_pages

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        del self.tiff_object
        if hasattr(self, '_stream'):
            del self._stream
        self._bytes_data = None

    def render_page(self, page_index):
        if page_index < 0 or page_index >= self.num_pages:
            raise IndexError('list index out of range')

        image_data = lib.render_page(
            self.tiff_object,
            ffi.cast('int', page_index),
            self.height_buffer,
            self.width_buffer,
            self.dpix_buffer,
            self.dpiy_buffer,
        )

        if image_data == ffi.NULL:
            return None

        height = self.height_buffer[0]
        width = self.width_buffer[0]
        dpix = self.dpix_buffer[0]
        dpiy = self.dpiy_buffer[0]

        image_data_buffer = ffi.buffer(image_data, width * height * 4)
        image = Image.frombytes('RGBA', (width, height), image_data_buffer, 'raw', 'RGBA')
        image.info['dpi'] = (dpix, dpiy)

        lib.delete_image_data(image_data)
        return image
