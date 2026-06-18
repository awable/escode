#!/usr/bin/env python

from unittest import TestCase

import sys
import escode


class TestEncodeErrors(TestCase):
    def test_unsupported_types(self):
        from datetime import datetime

        def fn():
            return None

        for bad in (datetime.utcnow(), object(), bytearray(b'ab'), 1 + 2j, fn):
            with self.assertRaises(escode.UnsupportedTypeError):
                escode.encode(bad)


class TestDecodeInput(TestCase):
    def test_decode_rejects_non_bytes(self):
        for bad in (u'notbytes', 123, None, [1, 2], bytearray(b'x')):
            with self.assertRaises(escode.DecodeError):
                escode.decode(bad)

    def test_decode_empty_is_none(self):
        # Documented convention: an empty blob decodes to None.
        self.assertIsNone(escode.decode(b''))

    def test_unknown_headbyte(self):
        # 0xff is not a valid type tag.
        with self.assertRaises(escode.DecodeError):
            escode.decode(b'\xff')


class TestExceptionHierarchy(TestCase):
    def test_subclassing(self):
        for exc in (escode.EncodeError, escode.DecodeError,
                    escode.UnsupportedTypeError):
            self.assertTrue(issubclass(exc, escode.Error))

    def test_qualified_names(self):
        # Exceptions must live under the 'escode' module, not a stray prefix.
        for exc in (escode.Error, escode.EncodeError, escode.DecodeError,
                    escode.UnsupportedTypeError):
            self.assertEqual(exc.__module__, 'escode')


class TestRobustness(TestCase):
    """A deserializer reads blobs back from storage, so corrupt or hostile
    input must fail with an exception, never crash the interpreter."""

    def test_truncated_input_never_crashes(self):
        # Every prefix of a valid encoding must decode or raise, not segfault.
        good = escode.encode({u'a': [1, 2, 3], u'b': u'hello', u'c': (1, 2)})
        for n in range(len(good)):
            try:
                escode.decode(good[:n])
            except Exception:
                pass

    def test_corrupt_length_header_rejected(self):
        # A collection header claiming far more elements than the blob holds
        # must be rejected rather than triggering a huge allocation (OOM).
        for blob in (b'\x5b\x7a\x37\xb2\xd9\xc2\xa9',   # bogus list length
                     b'\x6b\xff\xff\xff\xff'):           # bogus set/dict length
            with self.assertRaises(escode.DecodeError):
                escode.decode(blob)

    def test_deeply_nested_encode_raises(self):
        depth = sys.getrecursionlimit() * 4
        nested = []
        cur = nested
        for _ in range(depth):
            child = []
            cur.append(child)
            cur = child
        with self.assertRaises(RecursionError):
            escode.encode(nested)

    def test_self_referential_encode_raises(self):
        cyclic = []
        cyclic.append(cyclic)
        with self.assertRaises(RecursionError):
            escode.encode(cyclic)

    def test_deeply_nested_decode_raises(self):
        # Craft a blob of deeply nested single-element lists. Without a guard
        # this overflows the C stack; it must raise RecursionError instead.
        leaf = escode.encode(None)
        one_list = escode.encode([None])
        header = one_list[:len(one_list) - len(leaf)]   # "list, length 1"
        blob = header * (sys.getrecursionlimit() * 4) + leaf
        with self.assertRaises(RecursionError):
            escode.decode(blob)
