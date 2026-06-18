#!/usr/bin/env python

from unittest import TestCase

import escode


class TestRoundtrip(TestCase):
    """Ground-level guarantee: decode(encode(x)) == x, with the decoded
    object having the same type as the original. The existing per-type
    suites cover value ranges; these pin down the type mapping and the
    empty/degenerate cases that boundary fixtures skip."""

    def assertRoundtrip(self, obj):
        out = escode.decode(escode.encode(obj))
        self.assertEqual(out, obj)
        self.assertIs(type(out), type(obj))
        return out

    def test_top_level_scalars(self):
        # Every scalar must survive on its own, not just nested in a dict.
        for x in (None, True, False, 0, 1, -1, 1.5, -0.0, u'', b''):
            self.assertRoundtrip(x)

    def test_none_and_bool_identity(self):
        # bool must not collapse to int (True == 1 in Python).
        self.assertIs(escode.decode(escode.encode(None)), None)
        self.assertIs(escode.decode(escode.encode(True)), True)
        self.assertIs(escode.decode(escode.encode(False)), False)
        self.assertIsInstance(escode.decode(escode.encode(1)), int)

    def test_empty_containers(self):
        for empty in ([], (), {}, set()):
            self.assertRoundtrip(empty)

    def test_container_identity(self):
        # list/tuple share an ESTYPE and set/dict share another; decode must
        # recover the original container kind.
        self.assertIs(type(escode.decode(escode.encode([1, 2]))), list)
        self.assertIs(type(escode.decode(escode.encode((1, 2)))), tuple)
        self.assertIs(type(escode.decode(escode.encode({1, 2}))), set)
        self.assertIs(type(escode.decode(escode.encode({1: 2}))), dict)

    def test_nested_identity_preserved(self):
        obj = {u'k': [(1, 2), [3, 4], {5, 6}, {u'a': None}]}
        out = self.assertRoundtrip(obj)
        self.assertIs(type(out[u'k'][0]), tuple)
        self.assertIs(type(out[u'k'][1]), list)
        self.assertIs(type(out[u'k'][2]), set)
        self.assertIs(type(out[u'k'][3]), dict)

    def test_frozenset_decodes_to_set(self):
        # Documented lossy mapping: frozenset has no distinct wire type and
        # comes back as a plain set. Pinned so a change here is deliberate.
        out = escode.decode(escode.encode(frozenset([1, 2])))
        self.assertEqual(out, {1, 2})
        self.assertIs(type(out), set)

    def test_encode_is_deterministic(self):
        # A stable byte output lets callers use encodings as cache keys /
        # for dedup. Repeated encodes of the same value must match.
        for x in (None, 123, u'hello', b'bytes', [1, u'a', 2.0], (1, 2, 3)):
            self.assertEqual(escode.encode(x), escode.encode(x))
