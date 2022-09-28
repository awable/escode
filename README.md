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
City = namedtuple('City', ('id', 'name', 'country', 'pop'))

citylist = [
    City('city:1', 'Delhi', 'India', 19000000),
    City('city:2', 'Mumbai', 'India', 18000000),
    City('city:3', 'San Francisco', 'USA', 3000000),
    City('city:4', 'Paris', 'France', 5000000),...]

INDEX_NAME = 'index:city:country_pop'
indextuples = []

for city in citylist:
    # store data
    db.put(city.id, city)

    # store index
    indextuple = (INDEX_NAME, city.country, city.pop)
    indextuples.add(indextuple)
    index = escode.encode_index(indextuple)
    db.put(index, city.id)


#retrieval: Indian cities with pop between 1M and 5M
rangestart = escode.encode_index((INDEX_NAME,'India',1000000))
rangeend = escode.encode_index((INDEX_NAME,'India',5000000))
cityids = db.getrange(rangestart, rangeend)
indiancities = db.multiget(cityids)


# also useful for sorting
tuple_encodings = [
    (indextup, escode.encode_index(indextup))
    for indextup, cityid in indextuples]

# sorting by tuples is the same as sorting by the encodings
assert (sorted(indextuples, key=lambda tup_enc: tup_enc[0]) ==
        sorted(indextuples, key=lambda tup_enc: tup_enc[1]))
```


### Format

 Encodings use a 1 byte `headbyte` which stores the data type and some info, followed by a variable length (upto 8 bytes) number. This number is used to either store integer types or lengths for collections.

Index encodings are tricky to implement since one cannot simply concat the index tuples in order to maintain sort ordering i.e. `('a','z') < ('aa', 'z')` but `'az' > 'aaz'` This is accomplished in escode by using `'\x00\x00'` as the boundary between tuple elements, and escaping `\x00s` in the tuple elements themselves. Since elements like 8 byte zeros are fairly common, consecutive `\x00s` inside elements are compressed as an optimization.


![Format Table](https://github.com/awable/escode/blob/master/EscodeFormat.png)
