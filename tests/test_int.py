#!/usr/bin/env python
from unittest import TestCase

import escode
import struct
import io

_ESCODE_TYPE_LONG = 4  # -(1<<63) <= val < -(1<<31)
_ESCODE_TYPE_INT = 5   # -(1<<31) <= val < (1<<31)
_ESCODE_TYPE_UINT = 6  #    1<<31 <= val < 1<<32
_ESCODE_TYPE_ULONG = 7 #    1<<32 <= val < 1<<64


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
            -(1<<63),
            -(1<<63)+1,

            -(1<<31)-1,
            -(1<<31),
            -(1<<31)+1,

            -1,0,1,

            (1<<31)-1,
            (1<<31),
            (1<<31)+1,

            (1<<32)-1,
            (1<<32),
            (1<<32)+1,

            (1<<63)-1,
            (1<<63),
            (1<<63)+1,

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
            output.write(encoding[idx])

            if not ord(encoding[idx]):
                extrax00count = 255 - ord(encoding[idx+1])
                output.write(b'\x00'*(extrax00count))
                idx += 1

            idx += 1

        output.seek(0)
        return output.read(-1)

    def test_index_encoding_and_order(self):
        zipped = map(lambda x: (x, escode.encode_index((x,))), self.nums)

        for num, encoded in zipped:
            enc = self._fillzeros(encoded)
            encbytes = bytearray(enc)
            itype = encbytes.pop(0)

            size = 4 if itype in (_ESCODE_TYPE_INT, _ESCODE_TYPE_UINT) else 8
            pad = size - len(encbytes)
            encbytes.extend([0]*pad)
            assert len(encbytes) == size, (len(encbytes), size, hex(num), num, encbytes)

            if itype == _ESCODE_TYPE_LONG:
                encnum = struct.unpack('>q', str(encbytes))[0]
            elif itype == _ESCODE_TYPE_INT:
                encbytes[0] ^= 0x80
                encnum = struct.unpack('>i', str(encbytes))[0]
            elif itype == _ESCODE_TYPE_UINT:
                encnum = struct.unpack('>I', str(encbytes))[0]
            elif itype == _ESCODE_TYPE_ULONG:
                encnum = struct.unpack('>Q', str(encbytes))[0]

            self.assertEqual(num, encnum)

        numsorted = sorted(zipped, key=lambda (num,enc): num)
        encsorted = sorted(zipped, key=lambda (num,enc): enc)
        self.assertEqual(numsorted, encsorted)
