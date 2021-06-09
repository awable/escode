#!/usr/bin/env python
from unittest import TestCase

import itertools
import escode

class TestIndex00s(TestCase):


    def setUp(self):
        strings = [
            b'A%sB\x01'%(b'\x00'*0),
            b'A%sB\x01'%(b'\x00'*10),
            b'A%sB\x01'%(b'\x00'*255),
            b'A%sB\x01'%(b'\x00'*256),
            b'A%sB\x01'%(b'\x00'*300)]

        self.numbers = [
            -0x080, -0x800000000, -0x8000000000000,
            0x080, 0x800000000, 0x8000000000000,
            -0x081, -0x800000001, -0x8000000000001,
            0x081, 0x800000001, 0x8000000000001]

        self.tuples1 = list(itertools.product(strings, self.numbers))
        self.tuples2 = list(itertools.product(self.numbers, strings))
        self.trailing = [s[:-2] for s in strings]


    def test_index00s_order(self):
        #strings first in tuple
        zipped1 = [(t, escode.encode_index(t)) for t in self.tuples1]
        tupsorted = sorted(zipped1, key=lambda tup_enc: tup_enc[0])
        encsorted = sorted(zipped1, key=lambda tup_enc: tup_enc[1])
        self.assertEqual(tupsorted, encsorted)

        #test uniqueness of encodings
        unique1 = set(tup_enc[1] for tup_enc in zipped1)
        self.assertEqual(len(unique1), len(self.tuples1))

        #numbers first in tuple
        zipped2 = [(t, escode.encode_index(t)) for t in self.tuples2]
        tupsorted = sorted(zipped2, key=lambda tup_enc: tup_enc[0])
        encsorted = sorted(zipped2, key=lambda tup_enc: tup_enc[1])
        self.assertEqual(tupsorted, encsorted)


        #test uniqueness of encodings
        unique2 = set(tup_enc[1] for tup_enc in zipped2)
        self.assertEqual(len(unique2), len(self.tuples2))


    def test_string_trailing00s(self):
        encodings = set(escode.encode_index((s,)) for s in self.trailing)
        # trailing 0s should result in the same encoding
        self.assertEqual(len(encodings), 1)

    def test_number_trailing00s(self):
        encodings = set(escode.encode_index((n,)) for n in self.numbers)
        # trailing 0s in numbers should not result in the same encoding
        self.assertEqual(len(encodings), len(self.numbers))
