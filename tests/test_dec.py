#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

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
                random.randint(decimal.MIN_EMIN, decimal.MAX_EMAX)))
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

        self.decs += [
            decimal.Decimal('0'),
            decimal.Decimal('Infinity'),
            decimal.Decimal('-Infinity')]

    def test_dec(self):
        decs = self.decs;
        decs += [
            decimal.Decimal('0e100'), decimal.Decimal('0e-100'),
            decimal.Decimal('-0'), decimal.Decimal('-0e312'), decimal.Decimal('-0e-312')]

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
