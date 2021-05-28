#!/usr/bin/env python
from __future__ import print_function
from __future__ import division

import collections

data = collections.defaultdict(list)
with open('trials', 'r') as perf:
    for line in perf:
        cols = line.split(",")
        entype = cols[0]
        data[entype].append((float(cols[1]), float(cols[2]), float(cols[3])))


def trialavs(trials):
    return (
        sum(c[0] for c in trials)/len(trials),
        sum(c[1] for c in trials)/len(trials),
        sum(c[2] for c in trials)/len(trials))

escodeavs = trialavs(data['escode'])


print (u'Method | Encode | x | Decode | x | Size | x')
print (u'--- | --- | --- | ---')
for enctype, trials in data.items():
    avs = trialavs(trials)
    print (u' | '.join([
        enctype,
        '%d'%avs[0], '%.2f'%(avs[0]/escodeavs[0]),
        '%d'%avs[1], '%.2f'%(avs[1]/escodeavs[1]),
        '%d'%avs[2], '%.2f'%(avs[2]/escodeavs[2]),
    ]))
