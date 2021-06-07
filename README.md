# Fast Serializing and Deserializing for Python.

`escode` is a very fast binary serialize/deserialize module for Python. It is written as a Python C-Extension and operates on most portable types. It is designed to be fast, generate small encoding sizes, and have a version that is indexable/sortable. This last part is the primary motivation for `escode` since most data retrieval happens via indexed range queries.

### Performance

Below is the space and time performance of `escode` as compared to other major encodings. It was generally faster, and produced sizes comparable to dense formats like `cbor` or `msgpack`. The code used can be found in the [benchmark](https://github.com/awable/escode/tree/master/benchmark) directory.

Method | Encode (μs) |     x | Decode (μs) |     x |   Size |     x
     --- |    --- |   --- |    --- |   --- |    --- |   ---
  escode |    981 |  1.00 |   2411 |  1.00 |    185 |  1.00
  pickle |   1723 |  1.76 |   2697 |  1.12 |    233 |  1.26
    json |   5185 |  5.28 |   7685 |  3.19 |    338 |  1.82
    cbor |   1534 |  1.56 |   2456 |  1.02 |    180 |  0.97
   ujson |   2017 |  2.06 |   3687 |  1.53 |    330 |  1.78
 msgpack |   1329 |  1.36 |   3086 |  1.28 |    179 |  0.97

### Installation

```shell
# Requires gcc and python-dev
# Requires Python2.7+ or Python3+
pip install escode
```


### Usage

```python
import escode

data = {"id": <id>, "name":"James Maddison", ...}
blob = escode.encode(data)
db.put(<id>, blob)
...
dbdata = escode.decode(db.get(<id>))
assert dbdata == data
```

Most data retrieval for data happens via range queries which operates on data attributes. `escode.encode_index` produces an encoding that matches the sort order of the input. i.e.

```cmp(tup1, tup2) == cmp(encoded_tup1, encoded_tup2)```

Index encoding is **not decodable** as it skips some of info (like lengths for strings/collections)- it is only meant to be used to store and compare against an index of the same type.

```python
City = namedtuple('City', ('id', 'country', 'state', 'pop'))

citylist = [
    City('data:city:2137:delhi',  'India', 'Delhi',       19000000),
    City('data:city:2138:gurgaon','India', 'Haryana',      1153000),
    City('data:city:2139:mumbai', 'India', 'Maharashtra', 12478447),
    City('data:city:2718:sf',     'USA',   'California',     88149),
    City('data:city:7983:denver', 'USA',   'Colorado',      600158)]

INDEXID = 'index:city:cp'
for city in citylist:
    # store data
    db.put(city.id, city)

    # store index
    indextuple = (INDEXID, city.country, city.pop)
    index = escode.encode_index(indextuple)
    db.put(index, city.id)

#retrieval: indian cities with pop > 5M
query = escode.encode_index((INDEXID,'India',5000000))
# assuming range uses arg1 <= val <= arg2
cityids = db.getrange(query, query)
indiancities = db.multiget(cityids)


# also useful for sorting
tuple_encodings = [
    (indextup, escode.encode_index(indextup))
    for indextup, cityid in indextuples]

# sorting by tuples is the same as sorting by the encodings
assert (sorted(indextuples, key=lambda tup_enc: tup_enc[0]) ==
        sorted(indextuples, key=lambda tup_enc: tup_enc[1]))
```

A quick note on implementation: Index order of tuples  can be tricky since one must maintain tuple element boundaries which compare lower than any data. i.e. `('a','z') < ('aa', 'z')` but `'az' > 'aaz'` This is accomplished in escode by using `'\x00\x00'` as the boundary, and escaping `\x00s` in the tuple elements themselves, but compressing consecutive `\x00s` to compress space usage.

### Format

Regular encodings use a 1 byte `headbyte` which stores the data type and some info, followed by a variable length (upto 8 bytes) number for storing lengths or integers. Index encodings skip some of these parts and have `\x00s` escaped.

![Format Table](~/Documents/code/github/escode/format.png")
