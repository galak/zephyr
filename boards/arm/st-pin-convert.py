#!/usr/bin/env python3

import os, sys, re
import glob
from collections import OrderedDict, defaultdict

nargs = len(sys.argv)

file_list = sys.argv[1:]
for f in file_list:
    pinmux = open(f)

    in_node_has_status = False
    nodelabel = None

    pins = defaultdict(list)
    line = 0

    for p in pinmux:
        line += 1
        match_nodelabel = re.search(r"#if DT_NODE_HAS_STATUS.*DT_NODELABEL\((.*)\), okay", p)
        match_stm32_pin_func = re.search(r".*{.*STM32_PIN_([0-9A-Z]*),\s*STM32.*_PINMUX_FUNC_([0-9A-Z]*)_([0-9A-Z_)]*)", p)
        match_stm32_pin = re.search(r".*{.*STM32_PIN_.*", p)
        match_endif = re.search(r".*endif.*", p)
        line_procssed = False

        if match_nodelabel:
            nodelabel = match_nodelabel.group(1)
            if nodelabel.startswith("mac") or nodelabel.startswith("sdmmc1") or nodelabel.startswith("adc") or nodelabel.startswith("can") or nodelabel.startswith("spi") or nodelabel.startswith("usb") :
                nodelabel = None
                continue
            in_node_has_status = True

        if match_endif:
            if in_node_has_status:
                in_node_has_status = False
                nodelabel = None

        if match_stm32_pin_func:
            pin = match_stm32_pin_func.group(1)
            if pin != match_stm32_pin_func.group(2):
                print("MISMATCH file: %s line: %s error %s %s" % (f, line, pin, match_stm32_pin_func.group(2)))
            func = match_stm32_pin_func.group(3).lower()

            # handle CONFIG_SPI_STM32_USE_HW_SS missing w/simple if/endif check
#            if nodelabel is None:
#                if func.startswith("spi"):
                    # get spiN (assume 0..9)
#                    nodelabel = func[0:4]
#                    in_node_has_status = True

            if nodelabel:
                if func.startswith(nodelabel):
                    func = func.lstrip(nodelabel)
                    dev = nodelabel
                else:
                    match_l = re.search("([a-zA-Z]*)([0-9])*_", func)
                    if match_l:
                        print("M")
                        print(match_l.group(0))
                        print(match_l.group(1))
                        dev = match_l.group(1)
                        print("N")
                    print("FUNC nl %s = %s" % (nodelabel, func))
                    print(" %s" % func)

                match_func = re.search(r"[0-9]*_(.*)", func)

                if func[0] == "_":
                    func = func[1:]
                elif match_func:
                    func = match_func.group(1)

            if in_node_has_status:
                handle = f"&{dev.lower()}_{func.lower()}_{pin.lower()}"
                pins[nodelabel].append(handle)
                line_procssed = True
#            else:
                # Handle otg_fs
#                if func.startswith("otg_fs"):
#                    func = func.lstrip("otg_fs")
#                    handle = f"&usb_otg_fs_{func.lower()}_{pin.lower()}"
#                    pins["usbotg_fs"].append(handle)
#                    line_procssed = True

        if match_stm32_pin and line_procssed == False:
            print(f"{f}:{line}:{line_procssed}:{func} - {p.strip()}")


    dts_files = glob.glob( os.path.dirname(f) + "/*.dts*")

    for dts_fn in dts_files:

        dts = open(dts_fn)
        output = open(dts_fn+".tmp", 'w')

        for d in dts:
            output.write(d)
            match_d_nodelabel = re.search(r"&\s*([0-9A-Za-z_]*)\s*{", d)

            if match_d_nodelabel and not d.startswith("arduino") and not d.startswith("lscon_96b") and not d.startswith("feather"):
                node = match_d_nodelabel.group(1)
                if node in pins:
                    output.write("\tpinctrl-0 = < ")
                    for p in pins[node]:
                        output.write("%s " % p)
                    output.write(">;\n")
                    del pins[node]

        dts.close()
        output.close()
        os.rename(dts_fn+".tmp", dts_fn)

        for k in pins:
            print(f"ERROR: {dts_fn} {k}")
