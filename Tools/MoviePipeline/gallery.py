import struct, sys, os, json, subprocess, imageio_ffmpeg
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ipk_to_bk2 import recover_key
FF=imageio_ffmpeg.get_ffmpeg_exe()
Ksmelt=recover_key(open(r"D:\Games\LOSTARK\EFGame\Movies\F9P2M1G2NIHG31G2P0P2DEF.ipk",'rb').read())
u=lambda v:struct.pack('<I',v)
MOV=r"D:\Games\LOSTARK\EFGame\Movies"
OUT=r"D:\ClaudeWork\movietest\gallery"; os.makedirs(OUT,exist_ok=True)

def decrypt(path):
    raw=open(path,'rb').read(); n=len(raw)
    if n<0x200: return None
    delta=[None]*16
    for k,b in enumerate(u(n-8)): delta[(4+k)%16]=raw[4+k]^b^Ksmelt[(4+k)%48]
    fr=w=h=None
    for _ in range(10):
        Kf=bytes((Ksmelt[i]^delta[i%16]) if delta[i%16] is not None else 0 for i in range(48))
        d=bytes(raw[i]^Kf[i%48] for i in range(0x40))
        f2=struct.unpack_from('<I',d,8)[0]; w2=struct.unpack_from('<I',d,0x14)[0]; h2=struct.unpack_from('<I',d,0x18)[0]
        cr=[(4,u(n-8)),(0x1c,u(10000000)),(0x20,u(333333)),(0x28,u(0))]
        if 0<f2<50000: cr+=[(8,u(f2)),(0x10,u(f2)),(0x2c,u((0x2c+(f2+1)*4)|1))]
        if 0<w2<=4096: cr.append((0x14,u(w2)))
        if 0<h2<=4096: cr.append((0x18,u(h2)))
        nd=list(delta)
        for off,pt in cr:
            for k,b in enumerate(pt): nd[(off+k)%16]=raw[off+k]^b^Ksmelt[(off+k)%48]
        if nd==delta and 0<f2<50000: fr,w,h=f2,w2,h2; break
        delta=nd; fr,w,h=f2,w2,h2
    if not(fr and 0<fr<50000 and 0<w<=4096 and 0<h<=4096): return None
    Kf=bytes(Ksmelt[i]^delta[i%16] for i in range(48))
    a=np.frombuffer(raw,dtype=np.uint8)
    key=np.frombuffer((Kf*((n//48)+2))[:n],dtype=np.uint8)
    dec=(a^key).tobytes()
    if dec[:3] not in (b'BIK',b'KB2'): return None
    return dec,w,h,fr

files=[f for f in os.listdir(MOV) if f.lower().endswith('.ipk')]
gal=[]; tmp=os.path.join(OUT,'_t.bik')
for i,f in enumerate(files):
    try: r=decrypt(os.path.join(MOV,f))
    except Exception: r=None
    if not r: continue
    dec,w,h,fr=r
    if w*h<700*400: continue
    open(tmp,'wb').write(dec)
    png=os.path.join(OUT,f"{f[:20]}_{w}x{h}_f{fr}.png")
    subprocess.run([FF,'-y','-ss',str(fr/2/30.0),'-i',tmp,'-frames:v','1',png],capture_output=True)
    if not os.path.exists(png):
        subprocess.run([FF,'-y','-i',tmp,'-frames:v','1',png],capture_output=True)
    if os.path.exists(png): gal.append((f,w,h,fr))
    if i%40==0: print(f"  {i}/{len(files)} 갤러리{len(gal)}",flush=True)
if os.path.exists(tmp): os.remove(tmp)
json.dump([list(g) for g in gal],open(os.path.join(OUT,'gallery.json'),'w'),ensure_ascii=False)
print(f"완료 {len(gal)}개")
