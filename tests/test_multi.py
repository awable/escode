#!/usr/bin/env python
from unittest import TestCase

import random
import escode

class TestMulti(TestCase):


    def setUp(self):
        floats = [random.uniform(0, 1000) for x in range(20)]
        nums = [random.randint(-0x0fffffffffffffff, 0x0fffffffffffffff) for x in range(20)]
        hexs = [hex(random.randint(-0x0fffffffffffffff, 0x0fffffffffffffff)) for x in range(20)]
        self.tuples = zip(nums, floats, hexs)
        self.lists = map(list, self.tuples)
        self.dicts = dict(zip(hexs, self.lists))

        input_types = [
            None,
            True,
            False,
            "abcde",
            u"\xe1\xe9\xed\xf3\xfa\xfc\xf1\xbf\xa1",
            b"\xe1\xe9\xed\xf3\xfa\xfc\xf1\xbf\xa1",
            [],
            [True,False],
            [1,2],
            {},
            {'a':True, 'b':1}
        ]

        input_types = input_types + [input_types]
        self.dicts.update(((str(random.randrange(0, 1 << 64)), e) for e in input_types))

    def test_multi(self):
        dump = escode.encode(self.lists)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.lists)

        dump = escode.encode(self.dicts)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.dicts)

    def test_index_order(self):
        zipped = map(lambda x: (x, escode.encode_index(x)), self.tuples)
        tupsorted = sorted(zipped, key=lambda (tup,enc): tup)
        encsorted = sorted(zipped, key=lambda (tup,enc): enc)
        self.assertEqual(tupsorted, encsorted)
