#!/bin/sh

valgrind --tool=memcheck --leak-check=yes -v ./jawp alone jawp.cf
