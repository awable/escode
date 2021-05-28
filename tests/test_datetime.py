#!/usr/bin/env python
from datetime import datetime
from unittest import TestCase

import escode


class TestDateTime(TestCase):
    def test_datetime(self):
        now = datetime.utcnow().replace(microsecond=0)
        obj = {"now": now}

        serialized = escode.encode(obj)
        obj2 = escode.decode(serialized)

        self.assertEqual(obj, obj2)
