import sys,json
from elftools.elf.elffile import ELFFile
path=sys.argv[1]
with open(path,'rb') as stream:
 dwarf=ELFFile(stream).get_dwarf_info()
 result=[]
 for cu in dwarf.iter_CUs():
  by_offset={d.offset:d for d in cu.iter_DIEs()}
  selected={}
  def names(die):
   seen=set()
   while die is not None and die.offset not in seen:
    seen.add(die.offset)
    a=die.attributes
    name=a.get('DW_AT_linkage_name') or a.get('DW_AT_name')
    if name and any(x in name.value for x in (b'DontCompleteUntil',b'AddNested',b'CreateGraphEvent',b'DispatchSubsequents')):return name.value.decode()
    attr='DW_AT_abstract_origin' if 'DW_AT_abstract_origin' in a else 'DW_AT_specification'
    die=die.get_DIE_from_attribute(attr) if attr in a else None
  for die in by_offset.values():
   if die.tag not in ('DW_TAG_inlined_subroutine','DW_TAG_subprogram'):continue
   name=names(die)
   if not name:continue
   a=die.attributes
   if not any(x in a for x in ('DW_AT_ranges','DW_AT_low_pc')):continue
   ranges=[]
   if 'DW_AT_ranges' in a:
    for r in dwarf.range_lists().get_range_list_at_offset(a['DW_AT_ranges'].value,cu=cu):
     if hasattr(r,'begin_offset'):ranges.append([r.begin_offset,r.end_offset])
   else:ranges=[[a['DW_AT_low_pc'].value,a['DW_AT_low_pc'].value+a.get('DW_AT_high_pc').value]]
   row={'name':name,'die':hex(die.offset),'cu':hex(cu.cu_offset),'ranges':[[hex(x),hex(y)] for x,y in ranges]}
   result.append(row);print(json.dumps(row),flush=True)
 open('evidence/'+sys.argv[2],'w').write(json.dumps({'artifact':path,'entries':result},indent=2)+'\n')
