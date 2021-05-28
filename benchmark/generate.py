from __future__ import print_function
from __future__ import division

from six.moves import xrange
import random
import string



# GENERATE RANDOM BASIC TYPES
basicfuncs = []

def generate_tiny():
    return random.randint(-0x80,0x7F)
basicfuncs.append(generate_tiny)

def generate_small():
    return random.randint(-0x8000,0x7FFF)
basicfuncs.append(generate_small)

def generate_int():
    return random.randint(-0x80000000,0x7FFFFFFF)
basicfuncs.append(generate_int)

def generate_long():
    return random.randint(-0x8000000000000000,0x7FFFFFFFFFFFFFFF)
basicfuncs.append(generate_long)

letters = string.ascii_letters + string.digits + string.punctuation
def generate_str():
    return ''.join(random.sample(letters, random.randint(2,30)))
basicfuncs.append(generate_str)

def generate_bool():
    return bool(random.randint(0,2))
basicfuncs.append(generate_bool)

def generate_float():
    return float(generate_long())/generate_long()
basicfuncs.append(generate_float)


# GENERATE RANDOM COLLECTIONS TYPES
collectiontypes = []

def generate_list():
    size = random.randint(2,100)
    return [fn() for fn in (random.choice(basicfuncs) for _ in xrange(size))]
collectiontypes.append(generate_list)

# def generate_set():
#     return set(generate_list())
# collectiontypes.append(generate_set)

# def generate_tuple():
#     return tuple(generate_list())
# collectiontypes.append(generate_tuple)

# GENERATE RANDOM DICTS WITH BASIC KEYS AND ANY VALUES
dictfuncs = basicfuncs + collectiontypes
def generate_dict():
    size = random.randint(3,30)
    return {generate_str(): random.choice(dictfuncs)() for _ in xrange(size)}
