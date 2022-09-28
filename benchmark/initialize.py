from collections import namedtuple

# encoder imports
import escode
import pickle
import json
import cbor
import ujson
import msgpack


TrialRow = namedtuple(
    'TrialRow',
    ['module', 'numdicts', 'chunksize', 'encode', 'decode', 'encodingsize'])

Encoder = namedtuple(
    'Encoder',
    ['module', 'encode', 'enckwargs', 'decode', 'deckwargs'])

ENCODERS = [
    Encoder('escode', escode.encode, {}, escode.decode, {}),
    Encoder('pickle', pickle.dumps, {}, pickle.loads, {}),
#    Encoder('json', json.dumps, {}, json.loads, {}),
#    Encoder('cbor', cbor.dumps, {}, cbor.loads, {}),
#    Encoder('ujson', ujson.dumps, {}, ujson.loads, {}),
#    Encoder('msgpack', msgpack.packb, {}, msgpack.unpackb, {}),
]
