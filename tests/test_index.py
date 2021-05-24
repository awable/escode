#!/usr/bin/env python
from unittest import TestCase

import itertools
import escode

class TestIndex00s(TestCase):


    def setUp(self):
        strings = [
            'A{}B\x01'.format('\x00'*0),
            'A{}B\x01'.format('\x00'*10),
            'A{}B\x01'.format('\x00'*255),
            'A{}B\x01'.format('\x00'*256),
            'A{}B\x01'.format('\x00'*300)]

        self.numbers = [
            -0x080, -0x800000000, -0x8000000000000,
            0x080, 0x800000000, 0x8000000000000,
            -0x081, -0x800000001, -0x8000000000001,
            0x081, 0x800000001, 0x8000000000001]

        self.tuples1 = itertools.product(strings, self.numbers)
        self.tuples2 = itertools.product(self.numbers, strings)
        self.trailing = [s[:-2] for s in strings]


    def test_index00s_order(self):
        #strings first in tuple
        zipped = map(lambda x: (x, escode.encode_index(x)), self.tuples1)
        tupsorted = sorted(zipped, key=lambda (tup,enc): tup)
        encsorted = sorted(zipped, key=lambda (tup,enc): enc)
        self.assertEqual(tupsorted, encsorted)

        #numbers first in tuple
        zipped = map(lambda x: (x, escode.encode_index(x)), self.tuples2)
        tupsorted = sorted(zipped, key=lambda (tup,enc): tup)
        encsorted = sorted(zipped, key=lambda (tup,enc): enc)
        self.assertEqual(tupsorted, encsorted)


    def test_string_trailing00s(self):
        encoding = escode.encode_index(tuple(self.trailing))

        # check that the 00s have been correctly escaped
        parts = encoding.split("\x00\x00")
        self.assertEqual(len(parts), len(self.trailing))

        # trailing 0s should result in the same encoding
        self.assertEqual(len(set(parts)), 1)

    def test_number_trailing00s(self):
        encoding = escode.encode_index(tuple(self.numbers))

        # check that the 00s have been correctly escaped
        parts = encoding.split("\x00\x00")
        self.assertEqual(len(parts), len(self.numbers))

        # trailing 0s in numbers should not result in the same encoding
        self.assertEqual(len(set(parts)), len(self.numbers))
