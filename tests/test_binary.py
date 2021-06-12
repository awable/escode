#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase
from six import moves

import random
import struct
import escode



class TestBinary(TestCase):
    def setUp(self):
        n = 10
        self.nums = random.sample(moves.xrange(-0x80000000, 0x7FFFFFFF), n)
        self.format = 'i'*n
        self.bytes = struct.pack(self.format, *self.nums)

    def test_binary(self):
        dump = escode.encode(self.bytes)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.bytes)

        nums = list(struct.unpack(self.format, decoded))
        self.assertEqual(nums, self.nums)
