#!/usr/bin/env python3

import os, sys, re

nargs = len(sys.argv)
compats = []

cl = open("compat-list")
for l in cl:
    compats.append(l.strip())

regex_compat = '|'.join(compats)

if not 1 <= nargs:
    print("usage: %s [infile [outfile]]" %
        os.path.basename(sys.argv[0]))
    print(sys.argv)
    sys.exit(1)

file_list = sys.argv[1:]
file_has_drv_compat = []


for f in file_list:
    compats_per_file = {}
    compat = ""
    line = 0
    match_drv_compat = None
    prim_compat = None

    input = open(f)

    for s in input:
        match_drv_compat = re.search(r"DT_DRV_COMPAT", s)
        if match_drv_compat and f not in file_has_drv_compat:
            file_has_drv_compat.append(f)

        match_dt_inst = re.search(r"DT_INST_(\d+)_("+regex_compat+")([0-9A-Z_]*)", s)
        match_h_h = re.search(r"DT_INST_##\s*(\w*)\s*##_("+regex_compat+")([0-9A-Z_]*)",s)
        if match_dt_inst is not None:
            compat = match_dt_inst.group(2)
            if compat in compats_per_file:
                compats_per_file[compat] += 1
            else:
                compats_per_file[compat] = 1

        if match_h_h is not None:
            compat = match_h_h.group(2)
            if compat in compats_per_file:
                compats_per_file[compat] += 1
            else:
                compats_per_file[compat] = 1


    input.close()

    if compats_per_file:
        ratio = 0
        if len(compats_per_file) == 2:
            k = list(compats_per_file.values())
            if k[0] > k[1]:
                ratio = k[0] / k[1]
            else:
                ratio = k[1] / k[0]

            if (ratio > 2.0) and min(k[0], k[1]) < 5:
                if k[0] > k[1]:
                    prim_compat = list(compats_per_file.keys())[0]
                else:
                    prim_compat = list(compats_per_file.keys())[1]

    input = open(f)
    output = open(f+".tmp", 'w')

    line = 0
    for s in input:
        line_use_drv_inst = None
        s_orig = s
        used_dt_inst_ = False
        line = line + 1
        match_dt_inst = re.search(r"DT_INST_(\d+)_("+regex_compat+")([0-9A-Z_]*)", s)
        match_h_h = re.search(r"DT_INST_##\s*(\w*)\s*##_("+regex_compat+")([0-9A-Z_]*)",s)
        match_bus = re.search(r"DT_("+regex_compat+")_BUS_([0-9A-Z]*)", s)
        match_alias_label = re.search(r"DT_ALIAS_(.*)_LABEL", s)
        new_dt = None
        new_defined = None

        if match_alias_label:
            alias = match_alias_label.group(1)
            x = None
#            print(f"MATCH ALIAS {f}:{line} a:{alias} {s.strip()}")

            if s.startswith("#ifdef") or s.startswith("#if defined"):
                x = f"#if DT_HAS_NODE(DT_ALIAS({alias.lower()}))\n"
                pass
            elif s.startswith("#if !defined"):
                x = f"#if !DT_HAS_NODE(DT_ALIAS({alias.lower()}))\n"
            elif s.startswith("#if"):
                x = s.replace(match_alias_label.group(0), f"DT_HAS_NODE(DT_ALIAS({alias.lower()}))")
            elif s.startswith("#elif defined") or s.startswith("#elif"):
                x = f"#elif DT_HAS_NODE(DT_ALIAS({alias.lower()}))\n"
            else:
                x = s.replace(match_alias_label.group(0), f"DT_LABEL(DT_ALIAS({alias.lower()}))")

            if x is not None:
                output.write(x)
                continue

        if match_bus:
            compat = match_bus.group(1)
            bus = match_bus.group(2)
            x = None
            print(f"MATCH BUS {f}:{line} c:{compat} b:{bus} {s.strip()}")
            if s.startswith("#ifdef") or s.startswith("#if defined"):
                print("BUS IF DEFINED")
                x = f"#if DT_ANY_INST_ON_BUS({bus.lower()})\n"
                print(f"BUS IFd: {s}")
                print(f"BUS IFd: {x}")
                pass
            elif s.startswith("#if"):
                x = s.replace(match_bus.group(0), f"DT_ANY_INST_ON_BUS({bus.lower()})")
                print(f"BUS: {x}")
                pass
            elif s.startswith("#elif defined") or s.startswith("#elif"):
                x = f"#elif DT_ANY_INST_ON_BUS({bus.lower()})\n"
                pass
            elif s.startswith("#endif"):
                x = s.replace(match_bus.group(0), f"DT_ANY_INST_ON_BUS({bus.lower()})")
                print(f"BUS: {x}")
                pass
            if x is not None:
                output.write(x)
                continue

        if match_dt_inst is not None or match_h_h is not None:
            idx = None
            match_hash = re.search(r"#", s)
            match_defined = re.search(r"defined", s)
            match_ifdef = re.search(r"ifdef", s)
            match_ifndef = re.search(r"ifndef", s)
            match_endif = re.search(r"#endif.*(DT_INST_\d+_("+regex_compat+")[0-9A-Z_]*)", s)
            match_elif = re.search(r"#elif.*(DT_INST_\d+_("+regex_compat+")[0-9A-Z_]*)", s)
            match_else = re.search(r"#else.*(DT_INST_\d+_("+regex_compat+")[0-9A-Z_]*)", s)
            match_if = re.search(r"#if\s+(?!defined|!defined|\(defined)", s)
            match_define = re.search(r"#define\s+\w+\s+.*(DT_INST_\d+_("+regex_compat+")[0-9A-Z_]*)", s)

            if match_dt_inst:
                inst = match_dt_inst.group(1)
                compat = match_dt_inst.group(2)
                prop = match_dt_inst.group(3)

            if match_h_h:
                inst = match_h_h.group(1)
                compat = match_h_h.group(2)
                prop = match_h_h.group(3)
                full = match_h_h.group(0)

                if prop.endswith("_"):
                    print(f"WARNING: {f}:{line} ## something weird prop: {prop} - {s.strip()}")

            if prop:
                prop = prop[1:]

            # needs to be before we modify prop
            match_irq = re.search(r"^IRQ_(\d+|[0-9A-Z_]*)(_(FLAGS|SENSE|PRIORITY))?", prop)

            match_prop_idx = re.search(r"([A-Z0-9_]*)_(\d+)", prop)
            if match_prop_idx:
                prop = match_prop_idx.group(1)
                idx = match_prop_idx.group(2)

            match_gpio = re.search(r"([0-9A-Z_]*_GPIOS)_([0-9A-Z_]*)", prop)
            match_io = prop.find("IO_CHANNELS")

            dt_inst_compat = f"DT_INST({inst}, {compat.lower()})"
            inst_orig = inst

            if f.startswith("drivers"):
                line_use_drv_inst = True
                if len(compats_per_file) > 1:
                    line_use_drv_inst = False
                    if compat == prim_compat:
                        line_use_drv_inst = True
            else:
                line_use_drv_inst = False

            # print(f"MATCH c:{compat} p:{prop} drv_inst:{line_use_drv_inst} len: {len(compats_per_file)} - {s.strip()}")

            # use DT_INST version or not
            if line_use_drv_inst:
                inst_macro = "_INST"
            else:
                inst = dt_inst_compat
                inst_macro = ""

            if match_gpio:
                gpio_known_cells = ('CONTROLLER', 'PIN', 'FLAGS', 'LABEL')
                gpio_pha = match_gpio.group(1)
                gpio_cell = match_gpio.group(2)
                if gpio_cell == "CONTROLLER":
                    gpio_cell = "LABEL"

                if idx is None or idx == 0:
                    new_defined = f"DT{inst_macro}_NODE_HAS_PROP({inst}, {gpio_pha.lower()})"
                else:
                    new_defined = f"DT{inst_macro}_PROP_HAS_IDX({inst}, {gpio_pha.lower()}, {idx})"
                if gpio_pha == "CS_GPIOS":
                    if match_hash and not match_h_h and gpio_cell != "LABEL" and not re.search(r"#define", s):
                        print(f"WARNING: {f}:{line} CS_GPIOS cell {gpio_cell} convert to SPI_DEV_HAS_CS? - {s.strip()}")
                    new_dt = f"DT{inst_macro}_SPI_DEV_CS_GPIOS_{gpio_cell}({inst})"
                    new_defined = f"DT{inst_macro}_SPI_DEV_HAS_CS_GPIOS({inst})"
                else:
                    if gpio_cell in gpio_known_cells:
                        if idx is not None:
                            new_dt = f"DT{inst_macro}_GPIO_{gpio_cell}_BY_IDX({inst}, {gpio_pha.lower()}, {idx})"
                        else:
                            new_dt = f"DT{inst_macro}_GPIO_{gpio_cell}({inst}, {gpio_pha.lower()})"
                    else:
                        print("fWARNING: {f}:{line} UNKNOWN GPIO CELL: {gpio_cell}")
            elif prop == "BASE_ADDRESS":
                if idx is not None:
                    new_dt = f"DT{inst_macro}_REG_ADDR_BY_IDX({inst}, {idx})"
                else:
                    new_dt = f"DT{inst_macro}_REG_ADDR({inst})"
                    new_defined = f"DT{inst_macro}_NODE_HAS_PROP({inst}, reg)"
            elif prop.endswith("_BASE_ADDRESS"):
                reg_name = prop.split('_BASE_ADDRESS')[0]
                new_dt = f"DT{inst_macro}_REG_ADDR_BY_NAME({inst}, {reg_name.lower()})"
            elif prop == "SIZE":
                if idx is not None:
                    new_dt = f"DT{inst_macro}_REG_SIZE_BY_IDX({inst}, {idx})"
                else:
                    new_dt = f"DT{inst_macro}_REG_SIZE({inst})"
            elif prop == "BUS_NAME":
                new_dt = f"DT{inst_macro}_BUS_LABEL({inst})"
            elif prop == "LABEL":
                new_dt = f"DT{inst_macro}_LABEL({inst})"
                new_defined = f"DT{inst_macro}_NODE_HAS_PROP({inst}, {prop.lower()})"
            elif match_io != -1:
                io_known_cells = ('CONTROLLER', 'INPUT', 'LABEL')
                io_name = None
                if match_io == 0:
                    io_cell = prop[12:]
                else:
                    io_name = prop[0:match_io-1]
                    io_cell = prop[match_io+12:]

                if io_cell == "CONTROLLER":
                    io_cell = "LABEL"
                if idx is None:
                    if io_name is not None:
                        new_dt = f"DT{inst_macro}_IO_CHANNELS_{io_cell}_BY_NAME({inst}, {io_name.lower()})"
                    else:
                        new_dt = f"DT{inst_macro}_IO_CHANNELS_{io_cell}({inst})"
                else:
                    if io_name is not None:
                        new_dt = f"DT{inst_macro}_IO_CHANNELS_{io_cell}_BY_NAME({inst}, {io_name.lower()})"
                    else:
                        new_dt = f"DT{inst_macro}_IO_CHANNELS_{io_cell}_BY_IDX({inst}, {idx})"
            elif prop.startswith("CLOCKS_"):
                clk_cell = prop[7:]
                if clk_cell != "CLOCK_FREQUENCY":
                    print("WARNING: {f}:{line} cell is not CLOCK_FREQUENCY, not supported!")
                new_dt = f"DT{inst_macro}_PROP_BY_PHANDLE({inst}, clocks, clock_frequency)"
                new_defined = f"DT_NODE_HAS_PROP(DT{inst_macro}_PHANDLE({inst}, clocks), clock_frequency)"
            elif prop.startswith("CLOCK_") and not prop.startswith("CLOCK_FREQUENCY"):
                clk_cell = prop[6:]
                if idx is not None:
                    print("WARNING: {f}:{line}: CLK: cell %s idx %s" % (clk_cell, idx))
                if clk_cell == "CONTROLLER":
                    clk_cell = "LABEL"
                if clk_cell == "LABEL":
                    new_dt = f"DT{inst_macro}_CLOCKS_LABEL({inst})"
                else:
                    new_dt = f"DT{inst_macro}_CLOCKS_CELL({inst}, {clk_cell.lower()})"
            elif match_irq:
                irq_id = match_irq.group(1)
                irq_cell = "irq"
                if re.match(r"\d", irq_id):
                    if match_irq.group(3):
                        irq_cell = match_irq.group(3)
                    if irq_cell == "irq":
                        if irq_id == "0":
                            new_dt = f"DT{inst_macro}_IRQN({inst})"
                            new_defined = f"DT{inst_macro}_IRQ_HAS_CELL({inst}, {irq_cell.lower()})"
                        else:
                            new_dt = f"DT{inst_macro}_IRQ_BY_IDX({inst}, {irq_id}, {irq_cell.lower()})"
                            new_defined = f"DT{inst_macro}_IRQ_HAS_IDX({inst}, {irq_id})"
                    else:
                        if irq_id == "0":
                            new_dt = f"DT{inst_macro}_IRQ({inst}, {irq_cell.lower()})"
                            new_defined = f"DT{inst_macro}_IRQ_HAS_CELL({inst}, {irq_cell.lower()})"
                        else:
                            new_dt = f"DT{inst_macro}_IRQ_BY_IDX({inst}, {irq_id}, {irq_cell.lower()})"
                            new_defined = f"DT{inst_macro}_IRQ_HAS_CELL_AT_IDX({inst}, {irq_id}, {irq_cell.lower()})"
                else:
                    irq_name = irq_id
                    if irq_id.endswith(('_PRIORITY', '_FLAGS', '_SENSE')):
                        irq_name = '_'.join(irq_id.split('_')[:-1])
                        irq_cell = irq_id.split('_')[-1]
                    new_defined = f"DT{inst_macro}_IRQ_HAS_NAME({inst}, {irq_name.lower()})"
                    new_dt = f"DT{inst_macro}_IRQ_BY_NAME({inst}, {irq_name.lower()}, {irq_cell.lower()})"
            elif prop:
                # special case for pwm_litex.c
                reg_size = ('ENABLE_SIZE', 'WIDTH_SIZE', 'PERIOD_SIZE')
                if prop in reg_size:
                    reg_name = prop.split('_SIZE')[0]
                    new_dt = f"DT{inst_macro}_REG_SIZE_BY_NAME({inst}, {reg_name.lower()})"
                else:
                    if prop.endswith("_SIZE"):
                        print(f"WARNING: {f}:{line} prop could be REG_SIZE %s" % prop)
                    # TODO: deal w/ clocks and io-controller
                    # special case location array for silabs
                    if idx and prop.startswith("LOCATION"):
                        new_dt = f"DT{inst_macro}_PROP_BY_IDX({inst}, {prop.lower()}, {idx})"
                    elif idx:
                        prop = f"{prop}_{idx}"
                        new_dt = f"DT{inst_macro}_PROP({inst}, {prop.lower()})"
                    else:
                        new_dt = f"DT{inst_macro}_PROP({inst}, {prop.lower()})"
                        new_defined = f"DT{inst_macro}_NODE_HAS_PROP({inst}, {prop.lower()})"
    #                if match_hash_hash is None and match_hash is None:
    #                    print("DBG PROP(%s, %s) %s" % (inst, prop, s))
    #                    print(f"DBG PROP {prop}")
            else:
                if match_defined or match_ifdef or match_ifndef or match_endif or match_elif or match_else:
                    if line_use_drv_inst:
                        new_defined = f"DT_HAS_DRV_INST({inst_orig})"
                    else:
                        new_defined = f"DT_HAS_NODE(DT_INST({inst_orig}, {compat.lower()}))"
                if match_if:
                    if line_use_drv_inst:
                        new_dt = f"DT_HAS_DRV_INST({inst_orig})"
                    else:
                        new_dt = f"DT_HAS_NODE(DT_INST({inst_orig}, {compat.lower()}))"

            if match_h_h:
                s = x = s.replace(full, new_dt)
                output.write(s)
                continue

            dbg_info = f"{f}:{line}: {s.strip()}"
            file_info = f"{f}:{line}"

            if match_if:
                print(f"IF ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_dt:
                    s = x= re.sub(r"DT_INST_\d+_"+compat+"[0-9A-Z_]*", new_dt, s)
                    print(f"IF ({f}:{line}) [{new_defined}] A: {s.strip()}")

            elif match_defined:
                print(f"DEFINED ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_defined:
                    s = x= re.sub(r"(defined[\(\s]DT_INST_\d+_"+compat+"[0-9A-Z_]*\)?)", new_defined, s)
                    print(f"DEFINED ({f}:{line}) [{new_defined}] A: {s.strip()}")

            elif match_ifdef:
                print(f"IFDEF ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_defined:
                    new_defined = "#if " + new_defined
                    s = x= re.sub(r"(#ifdef[\(\s]DT_INST_\d+_"+compat+"[0-9A-Z_]*\)?)", new_defined, s)
                    print(f"IFDEF ({f}:{line}) [{new_defined}] A: {s.strip()}")

            elif match_ifndef:
                print(f"IFNDEF ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_defined:
                    new_defined = "#if !" + new_defined
                    s = x= re.sub(r"(#ifndef[\(\s]DT_INST_\d+_"+compat+"[0-9A-Z_]*\)?)", new_defined, s)
                    print(f"IFNDEF ({f}:{line}) [{new_defined}] A: {s.strip()}")

            elif match_elif:
                print(f"ELIF ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_dt:
                    s = x = s.replace(match_elif.group(1), new_dt)
                    print(f"ELIF ({f}:{line}) [{new_defined}] A: {x.strip()}")

            elif match_else:
                print(f"WARNING: {f}:{line} #else - could be incorrect - {s.strip()}")
                print(f"ELSE ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_dt:
                    s = x = s.replace(match_else.group(1), new_dt)
                    print(f"ELSE ({f}:{line}) [{new_defined}] A: {x.strip()}")

            elif match_endif:
                print(f"ENDIF ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_defined:
                    s = x = s.replace(match_endif.group(1), new_defined)
                    print(f"ENDIF ({f}:{line}) [{new_defined}] A: {x.strip()}")

            elif match_define:
                print(f"DEFINE ({f}:{line}) [{new_defined}] B: {s.strip()}")
                if new_dt:
                    s = x = s.replace(match_define.group(1), new_dt)
                    print(f"DEFINE ({f}:{line}) [{new_defined}] A: {s.strip()}")
                pass

            if not match_hash:
                if new_dt is not None:
                    s = re.sub(r"(DT_INST_\d_"+compat+"_[0-9A-Z_]*)", new_dt, s)

        if s_orig != s:
            if re.search(r"DT_INST_([0-9A-Z_]*)\(", s) and not line_use_drv_inst:
                print(f"WARNING: {f}:{line} COMPAT change to {compat} - {s.strip()}")


        output.write(s)

    input.close()
    output.close()
    os.rename(f+".tmp", f)

    if not f in file_has_drv_compat and compat != "" and f[-2:] != '.h' and f.startswith("drivers"):
        input = open(f)
        output = open(f+".tmp", 'w')
        line = 1
        start_comment = False
        first_line_end = None
        output_compat = False
        for s in input:
            if line == 1 and s.startswith("/*"):
                start_comment = True
            if start_comment and s.strip().endswith("/"):
                if first_line_end is None:
                    first_line_end = line + 1
            if line == first_line_end and not output_compat:
                output.write("\n")
                output.write(f"#define DT_DRV_COMPAT {compat.lower()}\n")
                output_compat = True
            output.write(s)
            line = line + 1
        if not output_compat:
            print("WARNING: did not output for {f}")
        input.close()
        output.close()
        os.rename(f+".tmp", f)
