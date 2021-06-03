#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

import sys
import statistics

from initialize import *
from collections import defaultdict

OVERALL = 0
data = {enc.module: defaultdict(list) for enc in ENCODERS}

with open(sys.argv[1], 'r') as perftrials:
    for line in perftrials:
        trial = TrialRow(*line.split(","))
        data[trial.module][(trial.chunksize,'encode')].append(trial.encode)
        data[trial.module][(trial.chunksize,'decode')].append(trial.decode)

        data[trial.module][(OVERALL,'encode')].append(trial.encode)
        data[trial.module][(OVERALL,'decode')].append(trial.decode)
        if float(trial.encodingsize):
            data[trial.module][(OVERALL,'size')].append(trial.encodingsize)

for enc in data:
    for partition, vals in data[enc].items():
        vals = list(map(float, vals))
        data[enc][partition] = (statistics.mean(vals), statistics.stdev(vals))

base_encode_mean = statistics.mean(data['escode'][(OVERALL,'encode')])
base_decode_mean = statistics.mean(data['escode'][(OVERALL,'decode')])
base_size_mean = statistics.mean(data['escode'][(OVERALL,'size')])


fmts = ('%8s', '%6s', '%5s', '%6s', '%5s', '%6s', '%5s')
header = ('Method', 'Encode','x','Decode', 'x','Size', 'x')

print (u' | '.join(fmt % h for fmt,h in zip(fmts,header)))
print (u' | '.join(fmt % '---' for fmt in fmts))

for encname, encdata in data.items():
    encode_mean = statistics.mean(encdata[(OVERALL,'encode')])
    decode_mean = statistics.mean(encdata[(OVERALL,'decode')])
    size_mean = statistics.mean(encdata[(OVERALL,'size')])

    print (u' | '.join([
        '%8s'%encname,
        '%6d'%encode_mean, '%5.2f'%(float(encode_mean)/base_encode_mean),
        '%6d'%decode_mean, '%5.2f'%(float(decode_mean)/base_decode_mean),
        '%6d'%size_mean, '%5.2f'%(float(size_mean)/base_size_mean),
    ]))
