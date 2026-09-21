# Native Screen Post 재질 곡선 연결

## G01. 변경할 경로

`Effect_ArtistMaterial.h`의 `Build_ArtistMaterialTrackBindings`는 현재 source Mesh/LocalDecal만 허용한다. 기존 native Screen Post와 같은 조건의 SCREEN_POST carrier도 named material parameter track을 받을 수 있게 한다. registry parameter 이름·row/lane·curve·source clock 검증은 그대로 유지한다.

`Effect_DocumentRenderer_Rendering.cpp`의 `Build_NativeScreenPost`는 준비된 정적 parameter packet만 snapshot에 복사한다. 준비된 Artist binding이 있으면 기존 `Apply_ArtistMaterialTrackSamples`로 `Frame.fSampleTimeSeconds`를 샘플링하여 snapshot을 완성한다. 기존 helper가 source time origin을 한 번 더하므로 element-local time이나 origin을 별도로 중복 적용하지 않는다. 실패는 snapshot material을 생성하지 않고 명확한 오류를 반환한다. capture 전용 분기와 track이 없는 기존 native screen post는 보존한다.

## G02. 검증

기존 인코딩·줄바꿈 및 무관한 변경을 유지하며 기준본을 `out/RaidRepair20260921/glass/validation/backup`에 보관한다. 새 제품 파일·프로젝트 등록·Data 변경은 없다. 전체 Rendering TU scratch compile과 실제 admission/sample helper native 검사로 source clock, scalar lanes, 기본값 보존과 실패 rollback을 확인한다. root가 제공하는 native 2627 후보가 완성되면 실제 codec/playback 및 screen-post shader FXC까지 확인한다. GUI/Product build/publish는 부모 작업자가 소유한다.
