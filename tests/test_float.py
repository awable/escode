#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase

import sys
import random
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
            sys.float_info.max,
            sys.float_info.min,
        ] + [random.uniform(sys.float_info.min, sys.float_info.max) for x in range(20)]


    def test_float(self):
        for flt in self.floats:
            enc = escode.encode(flt)
            self.assertEqual(flt, escode.decode(enc))

        dump = escode.encode(self.floats)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.floats)

    def test_index_order(self):
        zipped = [(f, escode.encode_index((f,))) for f in self.floats]
        numsorted = sorted(zipped, key=lambda num_enc: num_enc[0])
        encsorted = sorted(zipped, key=lambda num_enc: num_enc[1])
        self.assertEqual(numsorted, encsorted)
