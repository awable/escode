#!/usr/bin/env python
from unittest import TestCase

import escode

class TestFloat(TestCase):


    def setUp(self):

        self.floats = [
            123.123e15,
            123.123e14,
            123.123e-5,
            -0.0,
            -123.123e-4,
            -123.123e-5,
            123.123,
            123.123e-4,
            0.0,
        ]


    def test_float(self):
        dump = escode.encode(self.floats)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.floats)

    def test_index_order(self):
        zipped = map(lambda x: (x, escode.encode_index((x,))), self.floats)
        numsorted = sorted(zipped, key=lambda (num,enc): num)
        encsorted = sorted(zipped, key=lambda (num,enc): enc)
        self.assertEqual(numsorted, encsorted)
