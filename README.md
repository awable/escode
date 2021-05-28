# Fast Serializing and Deserializing for Python.

`escode` is a very fast serialize/deserialize module for Python (2.7+ & 3+). It is written as a Python C-Extension and operates on all base types and `list/dict/set/tuple`. The serialized data has also been designed to be small. This is useful for storage and network applications. Finally, the module provides index serialization for an index tuple which can be stored in a single binary field. This is useful for arbitrary indices on data for lookups and sorting

### Installation

`pip install escode`

### Performance

The primary options on python for serializing data today are json/pickle - but neither are particularly efficient in terms of space or time. Below is the space and time performance of escode as compared to pickle, json, and [cjson](https://pypi.org/project/python-cjson/) which is a C-Extension implementation of json. The code used can be found in the [benchmark](https://github.com/awable/escode/tree/main/benchmark) directory.


Method | Encode (μs) | x |  Decode (μs) | x |  Size | x
--- | --- |  --- |  --- | --- |  --- |  ---
escode | 5,724 | 1.00 | 7,759 | 1.00 | 1,142 | 1.00
cjson | 36,515 | 6.38 | 29,939 | 3.86 | 1,914 | 1.68
json | 36,880 | 6.44 | 46,736 | 6.02 | 1,914 | 1.68
pickle2 | 251,921 | 44.00 | 140,304 | 18.08 | 1438 | 1.26
pickle | 253,143 | 44.22 | 256,430 | 33.05 | 2,131 | 1.87


On Python 3.7+, the new pickle protocol `pickle(data, protocol=4)` comes pretty close though escode is still slightly faster.


Method | Encode (μs) | x |  Decode (μs) | x |  Size | x
--- | --- |  --- |  --- | --- |  --- |  ---
escode | 6,663 | 1.00 | 9,786 | 1.00 | 1,139 | 1.00
pickle4 | 9,440 | 1.42 | 13,163 | 1.34 | 1,213 | 1.07
pickle | 9,436 | 1.42 | 12,903 | 1.32 | 1,341 | 1.18

### Usage

```python
import escode

data = {
  "id": <id>, "name":"James Maddison",
  "usernames": ["jmad", "jamesm"],
  "location": {"lat": 28.7041, "lng":77.1025}
}

blob = escode.encode(data)
db.put(<id>, blob)

...

dbdata = escode.decode(db.get(<id>))
assert dbdata == data
```

Index encoding is useful for  applications where a multi-attribute index must be maintained in a single binary field, while maintaining the sort order of the original tuples. i.e. `cmp(tup1, tup2) == cmp(encoded_tup1, encoded_tup2)`. Index encoding is not decodable as it skips the data type information - it is only meant to be used to store and compare against an index of the same type.

```python
tuples = [
    (1234, 'Akhil', 13.014492753623188),
    (831, 'Rohit', 1.3086614173228346),
    (844, 'Poornima', 1.2375366568914956),
    (723, 'Mark', 2.0716332378223496),
    (874, 'John', 4.065116279069767),
    (94, 'Takkar', 0.09456740442655935)
]

tuple_encodings = [
    (t, escode.encode_index(t))
    for t in tuples]

# sorting by tuples is the same as sorting by the encodings
assert (sorted(tuple_encodings, key=lambda tup_enc: tup_enc[0]) ==
        sorted(tuple_encodings, key=lambda tup_enc: tup_enc[1]))
```

A quick note on implementation: Index order of tuples  can be tricky since one must maintain tuple element boundaries which compare lower than any data. i.e. `('a','z') < ('aa', 'z')` but `'az' > 'aaz'` This is accomplished in escode by using `'\x00\x00'` as the boundary, and escaping `\x00s` in the tuple elements themselves, but compressing consecutive `\x00s` to preserve space.
