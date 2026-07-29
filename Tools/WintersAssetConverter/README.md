# Winters Asset Converter

팀에서 동일한 W-format 바이너리(`.wmesh`, `.wmat`, `.wskel`, `.wanim`)를 만드는 도구입니다.

## 공통 런타임 경로

변환된 결과는 다음 루트 아래에서 에셋 종류와 고유 ID로 관리합니다.

```text
Client/Bin/Resources/LostArk/
├─ AssetCatalog.json
├─ Character/<AssetId>/
├─ Map/<AssetId>/
├─ Effect/<AssetId>/
└─ LoL/<AssetId>/
```

사람별 이니셜 폴더는 런타임 루트에 만들지 않습니다. 작업자 구분이 필요하면 원본 FBX 작업 공간에서만 구분하고, 최종 바이너리는 위 공통 구조로 합칩니다.

## 실행 파일

```text
Tools/WintersAssetConverter/Bin/WintersAssetConverter.exe
```

동일 폴더의 `assimp-vc143-mt.dll`, `poly2tri.dll`, `minizip.dll`, `zlib1.dll`,
`kubazip.dll`, `pugixml.dll`이 반드시 필요합니다. Visual C++ 2015-2022 x64 런타임도 설치되어 있어야 합니다.

## 기본 명령

```powershell
# 정적/스키닝 메시와 재질
.\Tools\WintersAssetConverter\Bin\WintersAssetConverter.exe mesh <input.fbx> -o <output.wmesh>

# 스켈레톤
.\Tools\WintersAssetConverter\Bin\WintersAssetConverter.exe skel <input.fbx> -o <output.wskel>

# 애니메이션
.\Tools\WintersAssetConverter\Bin\WintersAssetConverter.exe anim <input.fbx> --skel <output.wskel> -o <anims-directory>

# 바이너리 정보 확인
.\Tools\WintersAssetConverter\Bin\WintersAssetConverter.exe info <asset-file>
```

`Client/Bin/Resources`는 Git이 아니라 공용 Drive 배포 대상입니다. Converter 실행 파일과 설명서만 Git/LFS로 공유합니다.
