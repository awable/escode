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


class TestIndexOrdering(TestCase):
    """The core promise of encode_index: byte order of encodings matches
    Python's ordering of the source tuples."""

    def assertOrderPreserved(self, tuples):
        enc = {t: escode.encode_index(t) for t in tuples}
        self.assertEqual(
            sorted(tuples),
            sorted(tuples, key=lambda t: enc[t]),
        )

    def test_prefix_ordering(self):
        # The README's headline case: a tuple sorts before its extensions,
        # i.e. ('a',) < ('a','b'), even though concatenation alone would
        # break this ('a'+'z' > 'aa'+'z').
        tuples = [
            (), (u'a',), (u'a', u'a'), (u'a', u'b'),
            (u'aa',), (u'aa', u'b'), (u'b',),
        ]
        self.assertOrderPreserved(tuples)

    def test_mixed_scalar_types_per_position(self):
        # Each position holds the same type, but values span the type's range.
        firsts = [b'', b'a', b'ab', b'b']
        seconds = [-(1 << 40), -1, 0, 1, (1 << 40)]
        self.assertOrderPreserved(list(itertools.product(firsts, seconds)))

    def test_bool_and_none_ordering(self):
        # None / bool are orderable against themselves under encode_index.
        self.assertOrderPreserved([(None,), (None, 1), (None, 2)])
        self.assertOrderPreserved([(False,), (True,)])

    def test_unicode_ordering(self):
        words = [u'', u'a', u'ab', u'b', u'ba', u'\xe9', u'\xe9a']
        self.assertOrderPreserved([(w,) for w in words])


class TestIndexInc(TestCase):
    """encode_index(tuple, inc) nudges an encoding to build half-open range
    bounds: inc=-1 yields the largest key strictly below, inc=+1 the
    smallest key strictly above."""

    SAMPLES = [
        (0,), (1,), (-1,), (1 << 40,), (-(1 << 40),),
        (u'',), (u'a',), (u'India', 5), (b'\x00',), (3.14,),
    ]

    def test_inc_brackets_base(self):
        for t in self.SAMPLES:
            lo = escode.encode_index(t, -1)
            base = escode.encode_index(t)
            hi = escode.encode_index(t, +1)
            self.assertLess(lo, base, t)
            self.assertLess(base, hi, t)

    def test_inc_excludes_endpoint_in_range(self):
        # A range [encode_index(a), encode_index(b, -1)] is half-open: it
        # must include a's key but exclude b's key.
        keys = sorted(self.SAMPLES, key=escode.encode_index)
        a, b = keys[2], keys[5]
        start = escode.encode_index(a)
        end = escode.encode_index(b, -1)
        ka, kb = escode.encode_index(a), escode.encode_index(b)
        self.assertTrue(start <= ka <= end)
        self.assertFalse(start <= kb <= end)


class TestIndexErrors(TestCase):
    def test_requires_tuple(self):
        for bad in (5, u'a', [1, 2], {1: 2}):
            with self.assertRaises(TypeError):
                escode.encode_index(bad)

    def test_unindexable_types(self):
        # Sets and dicts have no stable ordering, so they are rejected for
        # index encoding rather than producing a misleading key.
        for bad in (({1, 2},), ({u'a': 1},)):
            with self.assertRaises(escode.EncodeError):
                escode.encode_index(bad)
