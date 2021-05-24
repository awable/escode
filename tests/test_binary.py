#!/usr/bin/env python
from unittest import TestCase

import random
import struct
import escode


class TestBinary(TestCase):
    def setUp(self):
        n = 10
        self.nums = random.sample(xrange(-0x80000000, 0x7FFFFFFF), n)
        self.format = 'i'*n
        self.bytes = struct.pack(self.format, *self.nums)

    def test_binary(self):
        dump = escode.encode(self.bytes)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.bytes)

        nums = list(struct.unpack(self.format, decoded))
        self.assertEqual(nums, self.nums)
