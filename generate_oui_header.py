#!/bin/python3
import sys
from datetime import datetime

out_f = open("include/__generated__oui_array.h", "w")
out_f.write(
f"""/* THIS FILE IS AUTOGENERATED {datetime.today().strftime('%Y-%m-%d-%H:%M:%S')}, DO NOT CHANGE! */

#include <sys/types.h>

static const char * __generated__oui_array[][2] = {{
"""
)
length = 0
with open(sys.argv[1]) as file:
    for line in file:
        pair = line.replace('"', r'\"').rstrip().split('\t')
        mac = pair[0]
        vendor = pair[len(pair) - 1]
        if len(mac) > 8:
            continue
        out_f.write(f'    {{"{mac}", "{vendor}"}},\n')
        length += 1
out_f.write("};\n\n"
           f"static int __generated__oui_array_length = {length};")
out_f.close()
