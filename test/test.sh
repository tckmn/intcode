#!/usr/bin/env bash
find test -regex '.*/[^.]*' -exec bash -c 'diff <(bin/intcode {} <{}.in) {}.out &>/dev/null || echo >&2 {} failed; printf .' \; | wc -c | sed 's/$/ tests completed/'
