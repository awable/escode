#!/usr/bin/env python

from unittest import TestCase

import escode


class TestBoolean(TestCase):
    def test_false_value(self):
        data = {"key": False}
        serialized = escode.encode(data)
        deserialized = escode.decode(serialized)

        self.assertIsInstance(deserialized["key"], bool)
        self.assertFalse(deserialized["key"])

    def test_true_value(self):
        data = {"key": True}
        serialized = escode.encode(data)
        deserialized = escode.decode(serialized)

        self.assertIsInstance(deserialized["key"], bool)
        self.assertTrue(deserialized["key"])
