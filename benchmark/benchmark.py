#!/usr/bin/env python
#
# Compare performance against other encodings
# Create many random dicts, or random sizes, with random data
# Time their write & read times, and measure their encoding sizes
#
# Initial benchmarking ran this script a 100 times:
# for run in {1..100}; do ./benchmark.py >> trials; done
#
from __future__ import print_function
from __future__ import division

from six.moves import xrange
from generate import *
import time

# encoder imports
import escode
import pickle
import six
import json


NUMDICTS = 100
dicts = [generate_dict() for i in xrange(NUMDICTS)]

# Set up encoders based on PY2/PY3
encoderfns = [
    ('escode', escode.encode, {}, escode.decode),
    ('pickle', pickle.dumps, {}, pickle.loads),
    ('json', json.dumps, {}, json.loads),
]
if six.PY3:
    encoderfns.append(('pickle4', pickle.dumps, dict(protocol=4), pickle.loads))
else:
    import cjson
    encoderfns.append(('pickle2', pickle.dumps, dict(protocol=2), pickle.loads))
    encoderfns.append(('cjson', cjson.encode, {}, cjson.decode))


# Shuffle the order we test them in
random.shuffle(encoderfns)

for encname, encfn, kwargs, decfn in encoderfns:
    # Make sure they actually work (JSON fails this)
    for d in dicts:
        assert d == decfn(encfn(d, **kwargs)), encfn.__module__

    # Encode
    s = time.time()
    for d in dicts:
        enc = encfn(d, **kwargs)
    wt = time.time() - s

    # Encode + Decode
    s = time.time()
    for d in dicts:
        decfn(encfn(d, **kwargs))
    rt = time.time() - s

    # Size
    le = 0
    for d in dicts:
        le += len(encfn(d, **kwargs))

    print (','.join((
        encname,
        str((wt*1e9)/NUMDICTS),
        str(((rt-wt)*1e9)/NUMDICTS),
        str(float(le)/NUMDICTS))))
