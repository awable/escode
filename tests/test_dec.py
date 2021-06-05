#!/usr/bin/env python
from unittest import TestCase

import sys
import random
import escode
import decimal

class TestDec(TestCase):

    def setUp(self):
        self.decs = [
            decimal.Decimal('%s1e%d' % (
                random.choice(('+','-')),
                random.randint(decimal.MIN_ETINY, decimal.MAX_EMAX)))
            for x in range(15)]

        self.decs += [
            decimal.Decimal('%s1e%d' % (
                random.choice(('+','-')),
                random.randint(-(1<<32), 1<<32)))
            for x in range(15)]

        self.decs += [
            decimal.Decimal('%s1e%d' % (
                random.choice(('+','-')),
                random.randint(-500, 500)))
            for x in range(15)]

    def test_dec(self):
        for dec in self.decs:
            enc = escode.encode(dec)
            self.assertEqual(dec, escode.decode(enc))

        dump = escode.encode(self.decs)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.decs)

    def test_index_order(self):
        zipped = [(d, escode.encode_index((d,))) for d in self.decs]
        numsorted = sorted(zipped, key=lambda num_enc: num_enc[0])
        encsorted = sorted(zipped, key=lambda num_enc: num_enc[1])
        self.assertEqual(numsorted, encsorted)
