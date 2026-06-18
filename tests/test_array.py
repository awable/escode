#!/usr/bin/env python

from unittest import TestCase

import escode


class TestArray(TestCase):
    """Exercise collections and strings large enough to need multi-byte
    length headers (the head number widens past 1 byte at 256, past 2 at
    65536, ...), which short fixtures never reach."""

    def _roundtrip(self, obj):
        self.assertEqual(escode.decode(escode.encode(obj)), obj)

    def test_long_list(self):
        # Spans the 1->2 and 2->3 byte length-header boundaries.
        for n in (255, 256, 257, 65535, 65536, 65537):
            self._roundtrip(list(range(n)))

    def test_long_string(self):
        for n in (255, 256, 65535, 65536, 100000):
            self._roundtrip(u'é' * n)   # multi-byte utf8 char
            self._roundtrip(b'\x00\xff' * n)

    def test_wide_dict(self):
        self._roundtrip({str(i): i for i in range(70000)})

    def test_list_of_strings(self):
        doc = {u"lines": [u"line %d" % i for i in range(1000)]}
        self._roundtrip(doc)
