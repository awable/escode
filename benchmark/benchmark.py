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

from six.moves import xrange, map
from generate import *
from initialize import *

import time
import six
import sys
import os

CHUNKSIZES = [1, 10, 100, 1000, 10000]
NUMDICTS = max(CHUNKSIZES)

OUTFILENAME = sys.argv[1] if len(sys.argv) > 1 else 'TRIALS'
print('# Writing trial data to file: %s' % OUTFILENAME)

# Shuffle the order we test them in
random.shuffle(ENCODERS)
#print ('# Testing: ', ', '.join(e[0] for e in ENCODERS))

#print ("# Generating %d data objects..." % NUMDICTS)
dicts = [generate_dict() for i in xrange(NUMDICTS)]
#print ("# done.")

with open(OUTFILENAME, "w") as out:
    for encname, encfn, enckwargs, decfn, deckwargs in ENCODERS:
        for chunksize in CHUNKSIZES:
            chunks = [dicts[idx:idx+chunksize] for idx in xrange(0, NUMDICTS, chunksize)]

            # ENCODING
            s = time.process_time_ns()
            for chunk in chunks:
                enc = encfn(chunk, **enckwargs)
            wt = time.process_time_ns() - s

            # ENCODING + DECODING
            s = time.process_time_ns()
            for chunk in chunks:
                decfn(encfn(chunk, **enckwargs), **deckwargs)
            rt = time.process_time_ns() - s

            le = 0
            if chunksize == max(CHUNKSIZES):
                le = float(len(enc))/chunksize

            output = TrialRow(
                module=encname,
                numdicts=NUMDICTS,
                chunksize=chunksize,
                encode=wt/NUMDICTS,
                decode=(rt-wt)/NUMDICTS,
                encodingsize=le,
            )

            out.write(','.join(map(str, output)))
            out.write('\n')
