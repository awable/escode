#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase
from six.moves import xrange
import escode


class TestLimits(TestCase):
    def setUp(self):
        self.bad_requests = [1<<64, -(1<<63)-1]

    def test_bad(self):
        for bad in self.bad_requests:
            with self.assertRaises(Exception):
                enc = escode.encode(bad)
