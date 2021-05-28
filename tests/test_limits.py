#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase
from six.moves import xrange
import escode


class TestLimits(TestCase):
    def setUp(self):

        goodlist = list(xrange(0xFFFF))
        self.good_requests = [
            'a'*len(goodlist),
            u'a'*0xFFFF,
            goodlist,
            set(goodlist),
            tuple(goodlist),
            {x:x for x in goodlist}
        ]

        badlist = goodlist + [0x10000]
        self.bad_requests = [
            'a'*0x10000,
            u'a'*0x10000,
            badlist,
            set(badlist),
            tuple(badlist),
            {x:x for x in badlist}
        ]

    def test_good(self):
        for good in self.good_requests:
            enc = escode.encode(good)
            self.assertEqual(escode.decode(enc), good)

    def test_bad(self):
        for bad in self.bad_requests:
            with self.assertRaises(Exception):
                enc = escode.encode(bad)
