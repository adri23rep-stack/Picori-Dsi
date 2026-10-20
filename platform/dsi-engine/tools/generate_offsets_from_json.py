#!/usr/bin/env python3
"""Generate public symbol-to-relative-offset headers, with no game data.
Matches asset_processor's EXTRACT offset calculation for JSON map/gfx groups.
For the EU port, asset start = USA JSON start + the active EU offset.
"""
import json
from pathlib import Path
R=Path(__file__).resolve().parents[3]
for name in ('map','gfx'):
    items=json.loads((R/'assets'/f'{name}.json').read_text())
    region_shift=0
    base=None
    offsets={}
    for a in items:
        if 'offsets' in a:
            region_shift=a['offsets'].get('EU',region_shift)
        elif 'calculateOffsets' in a:
            base=a['start']+region_shift
        elif 'path' in a and base is not None:
            if 'variants' in a and 'EU' not in a['variants']:
                continue
            start=a.get('starts',{}).get('EU') if 'starts' in a else a.get('start',0)+region_shift
            symbol=a.get('name') if a.get('type')=='dungeon_map' else Path(Path(a['path']).stem).stem
            if symbol in offsets and offsets[symbol]!=start-base:
                raise RuntimeError(f'duplicate conflicting offset: {symbol}')
            offsets[symbol]=start-base
    output=R/'build'/'EU'/'assets'/f'{name}_offsets.h'
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text('/* EU offsets from assets/'+name+'.json. Not ROM data. */\n'+
                      ''.join(f'#define offset_{k} 0x{v:x}\n' for k,v in offsets.items()))
    print(output.relative_to(R),len(offsets))
