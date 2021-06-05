#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase

import random
import escode

class TestInt(TestCase):


    def setUp(self):

        self.nums = [
            -(1<<63),-(1<<63)+1,
            -(1<<31)-1, -(1<<31), -(1<<31)+1,
            -(1<<15)-1, -(1<<15), -(1<<15)+1,
            -(1<<7)-1, -(1<<7), -(1<<7)+1,
            -1,0,1,
            (1<<7)-1, (1<<7), (1<<7)+1,
            (1<<8)-1, (1<<8), (1<<8)+1,
            (1<<15)-1, (1<<15), (1<<15)+1,
            (1<<16)-1, (1<<16), (1<<16)+1,
            (1<<31)-1, (1<<31), (1<<31)+1,
            (1<<32)-1, (1<<32), (1<<32)+1,
            (1<<63)-1, (1<<63), (1<<63)+1,
            (1<<64)-1,
        ] + [random.randint(-0x0fffffffffffffff, 0x0fffffffffffffff) for x in range(20)]

    def test_int(self):
        for num in self.nums:
            enc = escode.encode(num)
            self.assertEqual(num, escode.decode(enc))

        dump = escode.encode(self.nums)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.nums)


    def test_index_order(self):
        zipped = [(x, escode.encode_index((x,))) for x in self.nums]
        numsorted = sorted(zipped, key=lambda num_enc: num_enc[0])
        encsorted = sorted(zipped, key=lambda num_enc: num_enc[1])
        self.assertEqual(numsorted, encsorted)
