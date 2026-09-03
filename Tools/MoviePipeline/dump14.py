# -*- coding: utf-8 -*-
import struct, os, sys
import numpy as np
sys.path.insert(0,'.')
import ipk_to_bk2 as T
MOV=r"D:\Games\LOSTARK\EFGame\Movies"
OUT=r"D:\ClaudeWork\movietest\cmd848_verified"
os.makedirs(OUT,exist_ok=True)
files=["423O4VO3SDPBBZIKRA3OXZBRI.ipk","534P5WP4TEQCC0JLSB4P305P0J.ipk",
"756R2LF6VGSEE2LNUD6R9M2G0MKK.ipk","867SEGO7WHTFF3MOVE7S3AEV8LGZO.ipk",
"978T90T8XIUGG4NPWF8T2U020M4TUN.ipk","B9AVB2VAZKWII6PRYHAV46IYPAPKJ6WO.ipk",
"CABWC3WB0LXJJ7QSZIBWA7CW7QBWLK7XP.ipk","ECDY9SMD2NZLL9SU1KDYGT9N7TRRDNNM9ZR.ipk",
"ECDYE5YD2NZLL9SU1KDY79L1SDS1Q5YK1L1.ipk","FDEZLNVE3O0MMATV2LEZAHL2FSN6VETONA0S.ipk",
"GEF0G70F4P1NNBUW3MF091797TB01UF0POB1T.ipk","756R9UD6VGSEE2LNUD6RM77M202L.ipk",
"ECDYG1KD2NZLL9SU1KDYTEET979SDNNM9ZR.ipk","FDEZF6ZE3O0MMATV2LEZDAFZATEZ2RTZL2M2.ipk"]
def fd(raw):
    key=T.recover_key(raw)
    a=np.frombuffer(raw,dtype=np.uint8).copy()
    a^=np.tile(np.frombuffer(key,dtype=np.uint8),len(a)//48+1)[:len(a)]
    return a.tobytes()
man=[]
for f in files:
    raw=open(os.path.join(MOV,f),'rb').read(); n=len(raw)
    dec=fd(raw)
    fr=struct.unpack_from('<I',dec,0x08)[0]
    w=struct.unpack_from('<I',dec,0x14)[0]; h=struct.unpack_from('<I',dec,0x18)[0]
    offs=[struct.unpack_from('<I',dec,0x2c+i*4)[0] for i in range(fr+1)]
    bad=[i for i in range(1,fr) if offs[i]>offs[i+1]]
    stem=f[:8]
    open(os.path.join(OUT,stem+".bk2"),'wb').write(dec)
    salt=f[8:20]
    line=f"{stem}\t{dec[:4].decode('latin1')}\t{w}x{h}\tf={fr}\tbad={len(bad)}\tsalt={salt}"
    man.append(line); print(line,flush=True)
open(os.path.join(OUT,"manifest.txt"),'w',encoding='utf-8').write("\n".join(man)+"\n")
print(f"\n== 14개 저장 -> {OUT}")
