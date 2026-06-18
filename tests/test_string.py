# coding: utf8
#!/usr/bin/env python

from unittest import TestCase

import escode


class TestString(TestCase):
    def setUp(self):
        self.stringdict = {
            u'ʑʒʓʔʕʗʘʙʚʛʜʝʞ': 1,
            b'\xca\x91\xca\x92\xca\x93\xca\x94\xca\x95\xca\x97\xca\x98\xca\x99\xca\x9a\xca\x9b\xca\x9c\xca\x9d\xca\x9e': 2,
            '0xdeadbeef': 3}

    def test_strings(self):
        serialized = escode.encode(self.stringdict)
        stringdict = escode.decode(serialized)
        self.assertEqual(stringdict, self.stringdict)

    def test_empty(self):
        # Empty text and empty bytes must round-trip and stay distinct types.
        self.assertEqual(escode.decode(escode.encode(u'')), u'')
        self.assertEqual(escode.decode(escode.encode(b'')), b'')
        self.assertIsInstance(escode.decode(escode.encode(u'')), str)
        self.assertIsInstance(escode.decode(escode.encode(b'')), bytes)

    def test_type_identity(self):
        # bytes and str share an ESTYPE; decode must preserve which is which,
        # even when their contents are byte-identical.
        for text in (u'', u'hello', u'ʑʒʓ', u'\x00\x00'):
            out = escode.decode(escode.encode(text))
            self.assertIsInstance(out, str)
            self.assertEqual(out, text)
        for raw in (b'', b'hello', b'\xca\x91', b'\x00\x00'):
            out = escode.decode(escode.encode(raw))
            self.assertIsInstance(out, bytes)
            self.assertEqual(out, raw)

    def test_embedded_nulls(self):
        # NUL bytes inside contents must survive a normal (non-index) round-trip.
        for raw in (b'\x00', b'a\x00b', b'\x00' * 300, b'\x00a\x00'):
            self.assertEqual(escode.decode(escode.encode(raw)), raw)
