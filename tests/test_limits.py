#!/usr/bin/env python

from unittest import TestCase
import escode


class TestLimits(TestCase):
    def setUp(self):
        self.bad_requests = [1<<64, -(1<<63)-1]

    def test_bad(self):
        for bad in self.bad_requests:
            with self.assertRaises(Exception):
                enc = escode.encode(bad)
