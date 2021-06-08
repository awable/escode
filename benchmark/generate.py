from __future__ import print_function
from __future__ import division

from six.moves import xrange
import six
import random
import string
import sys

def generate_tiny():
    return random.randint(-0x80,0x7F)

def generate_small():
    return random.randint(-0x8000,0x7FFF)

def generate_int():
    return random.randint(-0x80000000,0x7FFFFFFF)

def generate_long():
    return random.randint(-0x8000000000000000,0x7FFFFFFFFFFFFFFF)

letters = string.ascii_letters + string.digits + string.punctuation
def generate_str(minlen=1, maxlen=20):
    slen = random.randint(minlen, maxlen)
    return ''.join(random.choice(letters) for _ in xrange(slen))

def generate_bin(minlen=1, maxlen=20):
    slen = random.randint(minlen, maxlen)
    return bytes(random.randint(0,255) for _ in xrange(slen))

def generate_unicode(minlen=1, maxlen=100):
    # Create a list of unicode characters within the range 0000-D7F
    ulen = random.randint(minlen, maxlen)
    return u"".join(six.unichr(random.randrange(0xD7FF)) for _ in xrange(ulen))

def generate_bool():
    return bool(random.randint(0,2))

def generate_float():
    return random.uniform(sys.float_info.min, sys.float_info.max)

basicfuncs = []
basicfuncs.append(generate_bool)
basicfuncs.append(generate_tiny)
basicfuncs.append(generate_small)
basicfuncs.append(generate_int)
basicfuncs.append(generate_long)
basicfuncs.append(generate_float)
basicfuncs.append(generate_str)
basicfuncs.append(generate_bin)
basicfuncs.append(generate_unicode)


def generate_list(minlen=1, maxlen=10, fn=None):
    size = random.randint(minlen, maxlen)
    return [(fn or random.choice(basicfuncs))() for _ in xrange(size)]

def generate_set(minlen=1, maxlen=10, fn=None):
    return set(generate_list(minlen, maxlen))

def generate_tuple(minlen=1, maxlen=10, fn=None):
    return tuple(generate_list((minlen, maxlen)))


collectiontypes = []
collectiontypes.append(generate_list)
# collectiontypes.append(generate_set)
# collectiontypes.append(generate_tuple)

# GENERATE RANDOM DICTS WITH BASIC KEYS AND ANY VALUES
dictfuncs = basicfuncs + collectiontypes
def generate_dict(minlen=5, maxlen=10, fn=None):
    size = random.randint(minlen,maxlen)
    return {generate_str(): (fn or random.choice(dictfuncs))() for _ in xrange(size)}
