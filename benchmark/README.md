# Escode Benchmarks

Generates a bunch of random dictionaries of random sizes with random data. See `generate.py`. Also see `requirements.txt` and pip install those package. Usually via

```shell
pip install -r requirements.txt
```

Then run the following. You can control the number of trials in `RUNTRIALS.sh`.

```shell
./RUNTRIALS.sh
     Creating Directory 2021-06-05_17.28.42
     # Writing trial data to file: 2021-06-05_17.28.42/_TRIALS_1
     # Writing trial data to file: 2021-06-05_17.28.42/_TRIALS_2
     ...
     # Writing trial data to file: 2021-06-05_17.28.42/_TRIALS_N
     Merging Trial Files
./analyze.py 2021-06-05_17.28.42/TRIALS
```

The bulk of work is done by `benchmark.py`. The encoders used are defined in `initialize.py`
