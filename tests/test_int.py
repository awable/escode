#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

from unittest import TestCase

import codecs
import escode
import struct
import io

_ESCODE_TYPE_INT64 = 4;
_ESCODE_TYPE_INT32 = 5;
_ESCODE_TYPE_INT16 = 6;
_ESCODE_TYPE_INT8 = 7;
_ESCODE_TYPE_UINT8 = 8;
_ESCODE_TYPE_UINT16 = 9;
_ESCODE_TYPE_UINT32 = 10;
_ESCODE_TYPE_UINT64 = 11;

class TestInt(TestCase):


    def setUp(self):
        self.good_request_dict = {
            "uint64": 0xFFFFFFFFFFFFFFFF - 1,
            "int64": 0x7FFFFFFFFFFFFFFF - 1,
            "int32": 0x7fffffff
        }
        self.bad_request_dict = {
            "uint64": 0xFFFFFFFFFFFFFFFF << 1
        }

        self.nums = [
            -(1<<63),-(1<<63)+1,
            -(1<<31)-1, -(1<<31), -(1<<31)+1,
            -(1<<15)-1, -(1<<15), -(1<<15)+1,
            -(1<<7)-1, -(1<<7), -(1<<7)+1,
            -1,0,1,
            (1<<7)-1, (1<<7), (1<<7)+1,
            (1<<8)-1, (1<<8), (1<<8)+1,
            (1<<15)-1, (1<<15), (1<<15)+1,
            (1<<16)-1, (1<<16), (1<<16)+1,
            (1<<31)-1, (1<<31), (1<<31)+1,
            (1<<32)-1, (1<<32), (1<<32)+1,
            (1<<63)-1, (1<<63), (1<<63)+1,
            (1<<64)-1,
        ]

    def test_int(self):
        dump = escode.encode(self.good_request_dict)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.good_request_dict)

        dump = escode.encode(self.nums)
        decoded = escode.decode(dump)
        self.assertEqual(decoded, self.nums)

        with self.assertRaises(Exception):
            dump = escode.encode(self.bad_request_dict)
            decoded = escode.decode(dump)

    def _fillzeros(self, encoding):
        output = io.BytesIO()

        idx = 0
        while idx < len(encoding):
            output.write(encoding[idx:idx+1])
            if not encoding[idx]:
                extrax00count = 255 - encoding[idx+1]
                output.write(b'\x00'*(extrax00count))
                idx += 1

            idx += 1

        output.seek(0)
        return bytearray(output.read(-1))

    def test_index_encoding_and_order(self):
        zipped = [(x, escode.encode_index((x,))) for x in self.nums]

        for num, encoded in zipped:
            encbytes = self._fillzeros(bytearray(encoded))
            itype = encbytes[0]
            encbytes = encbytes[1:]

            size = 1
            if itype in (_ESCODE_TYPE_INT16, _ESCODE_TYPE_UINT16): size = 2
            if itype in (_ESCODE_TYPE_INT32, _ESCODE_TYPE_UINT32): size = 4
            if itype in (_ESCODE_TYPE_INT64, _ESCODE_TYPE_UINT64): size = 8

            pad = size - len(encbytes)
            encbytes += b'\x00'*pad

            assert len(encbytes) == size, (len(encbytes), size, hex(num), num, encbytes)

            if itype == _ESCODE_TYPE_INT64:
                encnum = struct.unpack('>q', encbytes)[0]
            elif itype == _ESCODE_TYPE_INT32:
                encnum = struct.unpack('>i', encbytes)[0]
            elif itype == _ESCODE_TYPE_INT16:
                encnum = struct.unpack('>h', encbytes)[0]
            elif itype == _ESCODE_TYPE_INT8:
                encnum = struct.unpack('>b', encbytes)[0]
            elif itype == _ESCODE_TYPE_UINT8:
                encnum = struct.unpack('>B', encbytes)[0]
            elif itype == _ESCODE_TYPE_UINT16:
                encnum = struct.unpack('>H', encbytes)[0]
            elif itype == _ESCODE_TYPE_UINT32:
                encnum = struct.unpack('>I', encbytes)[0]
            elif itype == _ESCODE_TYPE_UINT64:
                encnum = struct.unpack('>Q', encbytes)[0]

            self.assertEqual(num, encnum)

        numsorted = sorted(zipped, key=lambda num_enc: num_enc[0])
        encsorted = sorted(zipped, key=lambda num_enc: num_enc[1])
        self.assertEqual(numsorted, encsorted)
