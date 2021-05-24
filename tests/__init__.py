#!/usr/bin/env python
import os
import sys
import unittest


def loadtestsuite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()

    here = os.path.dirname(__file__)
    for fn in os.listdir(here):
        if fn.startswith("test") and fn.endswith(".py"):
            modname = "tests." + fn[:-3]
            __import__(modname)
            module = sys.modules[modname]
            suite.addTests(loader.loadTestsFromModule(module))
    return suite


def main():
    suite = loadtestsuite()
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)

if __name__ == '__main__':
    main()
