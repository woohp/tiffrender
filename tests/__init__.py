import unittest
from tiffrender import Tiff
from PIL import Image


class TiffTestCase(unittest.TestCase):
    def test_open_from_file(self):
        doc = Tiff.fromfile('tests/assets/CCITT_1.TIF')
        self.assertIsNotNone(doc)
        self.assertEqual(len(doc), 1)

    def test_open_nonexisting_file(self):
        with self.assertRaisesRegexp(ValueError, 'invalid tiff file'):
            doc = Tiff.fromfile('tests/assets/missing.tiff')

    def test_open_corrupted_file(self):
        with self.assertRaisesRegexp(ValueError, 'invalid tiff file'):
            doc = Tiff.fromfile('tests/assets/corrupted.tif')

    def test_open_from_bytes(self):
        with open('tests/assets/CCITT_1.TIF', 'rb') as f:
            file_bytes = f.read()
        doc = Tiff.frombytes(file_bytes)
        self.assertIsNotNone(doc)
        self.assertEqual(len(doc), 1)

    def test_open_from_corrupted_bytes(self):
        with self.assertRaisesRegexp(ValueError, 'invalid tiff file'):
            Tiff.frombytes(b'foo')

    def test_render_page(self):
        doc = Tiff.fromfile('tests/assets/multipage_tiff_example.tif')
        self.assertEqual(len(doc), 10)
        page = doc.render_page(0)
        self.assertIsInstance(page, Image.Image)
        self.assertEqual(page.size, (800, 600))
        self.assertEqual(page.mode, 'RGBA')
        self.assertEqual(page.load()[100, 100], (255, 255, 255, 255))
        self.assertEqual(page.info['dpi'], (96, 96))

    def test_render_page_invalid_index(self):
        doc = Tiff.fromfile('tests/assets/multipage_tiff_example.tif')
        with self.assertRaisesRegexp(ValueError, 'page index out of range'):
            doc.render_page(-1)
        with self.assertRaisesRegexp(ValueError, 'page index out of range'):
            doc.render_page(20)
