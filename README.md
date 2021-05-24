# Fast Serializing and Deserializing of Python types.

`escode` is a Python C-Extension which quickly serializes and deserializes arbitrary base types into binary data.
The serialization has also been designed to be small. This is useful for storage and network applications. Finally,
the module provides index serialization for a tuple of data - where the serialization maintains sorting order against
all other tuples containing the same types in the same order (or prefixes).

```python
import escode

data = {
    "id": 1234,
    "name":"Akhil",
    "usernames": ["akhil", "awable"],
    "location": {"lat": 28.7041, "lng":77.1025}
}

blob = escode.encode(data)
db.put(<id>,..., blob)

...

dbdata = escode.decode(db.get(<id>))
assert dbdata == data
```

Index encoding is useful for  applications where a multi-attribute index must be maintained in a single binary
field, while maintaining the sort order of the original tuples.

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

A quick note on implementation: Index order of tuples  can be tricky since one must maintain tuple
element boundaries which compare lower than any data. i.e. `('a','z') < ('aa', 'z')` but `'az' > 'aaz'` This is
accomplished in escode by using `'\x00\x00'` as the boundary, and escaping `\x00s` in the tuple elements themselves,
but compressing consecutive `\x00s` to preserve space.
