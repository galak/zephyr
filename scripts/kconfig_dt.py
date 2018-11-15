#!/usr/bin/env python3
#
# Copyright (c) 2018 Linaro
#
# SPDX-License-Identifier: Apache-2.0

import sys
import os
import re

# Types we support
# 'string', 'int', 'hex', 'bool'
gen_path="build/zephyr/include/generated/generated_dts_board.conf"

regex = '^CONFIG_' + sys.argv[1] + '=*'

print(os.getcwd())
if os.path.isfile(gen_path):
    while(1):
        pass

    print("file ")
    for line in open(gen_path, 'r', encoding='utf-8'):
        if re.search(regex, line):
            c = line.split('=')[1].strip()
            sys.stdout.write(c)

else:
    print("no file")

