#!/usr/bin/env python3
"""Read build-specific type/function facts from the shipped DWARF sidecar.

Requires pyelftools; this produces shell-derived evidence, not REA Evidence.
Usage: PYTHONPATH=.tools/python python3 scripts/inspect_tick_dwarf.py
"""
import json
import pathlib
import sys
from elftools.elf.elffile import ELFFile

ROOT = pathlib.Path(__file__).resolve().parents[1]
DEBUG = ROOT / 'server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.debug'
NAMES = {b'AFGBuildableSubsystem', b'FConveyorTickGroup',
         b'IFGFactoryTickHandlerInterface', b'AFGBuildable'}
ADDRESSES = {0x187faf0, 0x187fc40, 0x189dbe0, 0x189dcc0, 0x189dd80, 0x189dec0}


def value(attribute):
    if attribute is None:
        return None
    return attribute.value.decode(errors='replace') if isinstance(attribute.value, bytes) else attribute.value


def contains(dwarf, cu, pc):
    attributes = cu.get_top_DIE().attributes
    if 'DW_AT_ranges' not in attributes:
        low = value(attributes.get('DW_AT_low_pc'))
        high = attributes.get('DW_AT_high_pc')
        if low is None or high is None:
            return False
        end = high.value if high.form == 'DW_FORM_addr' else low + high.value
        return low <= pc < end
    base = value(attributes.get('DW_AT_low_pc')) or 0
    for item in dwarf.range_lists().get_range_list_at_offset(attributes['DW_AT_ranges'].value, cu=cu):
        if hasattr(item, 'base_address'):
            base = item.base_address
            continue
        start = item.begin_offset if item.is_absolute else base + item.begin_offset
        end = item.end_offset if item.is_absolute else base + item.end_offset
        if start <= pc < end:
            return True
    return False


def main():
    with DEBUG.open('rb') as stream:
        dwarf = ELFFile(stream).get_dwarf_info()
        candidates = [cu for cu in dwarf.iter_CUs() if contains(dwarf, cu, 0x187fc40)]
        if len(candidates) != 1:
            raise RuntimeError(f'Expected exactly one CU covering TickFactoryActors, got {len(candidates)}')
        cu = candidates[0]
        result = {'artifact': str(DEBUG.relative_to(ROOT)), 'cu_offset': hex(cu.cu_offset),
                  'types': [], 'functions': [],
                  'limitations': ['Build-specific DWARF; declarations can lack full type layouts.',
                                  'Static metadata does not prove thread safety or runtime behavior.']}
        for die in cu.iter_DIEs():
            attributes = die.attributes
            if die.tag in ('DW_TAG_structure_type', 'DW_TAG_class_type') and value(attributes.get('DW_AT_name')) in {name.decode() for name in NAMES}:
                result['types'].append({'die_offset': hex(die.offset), 'name': value(attributes['DW_AT_name']),
                    'declaration': value(attributes.get('DW_AT_declaration')) or False,
                    'size': value(attributes.get('DW_AT_byte_size')),
                    'members': [{key: value(attribute) for key, attribute in child.attributes.items()}
                                for child in die.iter_children() if child.tag == 'DW_TAG_member']})
            if die.tag == 'DW_TAG_subprogram' and value(attributes.get('DW_AT_low_pc')) in ADDRESSES:
                result['functions'].append({'die_offset': hex(die.offset),
                    'address': hex(value(attributes['DW_AT_low_pc'])),
                    'high_pc_value': value(attributes.get('DW_AT_high_pc')),
                    'high_pc_form': attributes['DW_AT_high_pc'].form,
                    'decl_line': value(attributes.get('DW_AT_decl_line'))})
        missing = ADDRESSES - {int(function['address'], 16) for function in result['functions']}
        if missing:
            raise RuntimeError(f'Expected function DIEs missing: {sorted(missing)}')
        json.dump(result, sys.stdout, indent=2)
        print()


if __name__ == '__main__':
    main()
