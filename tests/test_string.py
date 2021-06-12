# coding: utf8
#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase

import escode


class TestArray(TestCase):
    def setUp(self):
        self.stringdict = {
            u'ʑʒʓʔʕʗʘʙʚʛʜʝʞ': 1,
            b'\xca\x91\xca\x92\xca\x93\xca\x94\xca\x95\xca\x97\xca\x98\xca\x99\xca\x9a\xca\x9b\xca\x9c\xca\x9d\xca\x9e': 2,
            '0xdeadbeef': 3}

    def test_strings(self):
        serialized = escode.encode(self.stringdict)
        stringdict = escode.decode(serialized)
        self.assertEquals(stringdict, self.stringdict)
