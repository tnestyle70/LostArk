# -*- coding: utf-8 -*-
import struct, os, sys
import numpy as np
sys.path.insert(0,'.')
import ipk_to_bk2 as T
MOV=r"D:\Games\LOSTARK\EFGame\Movies"
OUT=r"D:\ClaudeWork\movietest\lobbybg_verified"
os.makedirs(OUT,exist_ok=True)
prefixes=['OEL9O70L','PFMAP81M','SIPDSB4P']
def fd(raw):
    key=T.recover_key(raw)
    a=np.frombuffer(raw,dtype=np.uint8).copy()
    a^=np.tile(np.frombuffer(key,dtype=np.uint8),len(a)//48+1)[:len(a)]
    return a.tobytes()
man=[]
for f in sorted(os.listdir(MOV)):
    if not f.endswith('.ipk'): continue
    if not any(f.startswith(p) for p in prefixes): continue
    p=os.path.join(MOV,f)
    if os.path.getsize(p)>120*1024*1024: continue
    raw=open(p,'rb').read(); n=len(raw)
    try: dec=fd(raw)
    except Exception as e:
        print(f"{f[:12]} 실패:{e}",flush=True); continue
    fr=struct.unpack_from('<I',dec,0x08)[0]
    w=struct.unpack_from('<I',dec,0x14)[0]; h=struct.unpack_from('<I',dec,0x18)[0]
    if not(0<fr<100000 and 100<w<4000 and 100<h<4000): continue
    offs=[struct.unpack_from('<I',dec,0x2c+i*4)[0] for i in range(fr+1)]
    bad=sum(1 for i in range(1,fr) if offs[i]>offs[i+1])
    stem=f[:8]
    open(os.path.join(OUT,stem+".bk2"),'wb').write(dec)
    line=f"{stem}\t{dec[:4].decode('latin1')}\t{w}x{h}\tf={fr}\tbad={bad}\t{f}"
    man.append(line); print(line,flush=True)
open(os.path.join(OUT,"manifest.txt"),'w',encoding='utf-8').write("\n".join(man)+"\n")
print(f"\n== {len(man)}개 저장 -> {OUT}",flush=True)
