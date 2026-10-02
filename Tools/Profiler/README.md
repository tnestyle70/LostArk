# 저장된 Profiler JSON 분석

`analyze_capture.py`는 Client를 실행하지 않고 `LostArkProfilerCapture.v3`의 선택 프레임을 재집계한다.
Python 표준 라이브러리만 사용한다. 출력은 기존 파일을 덮어쓰지 않는 별도 JSON이다.

```powershell
python Tools/Profiler/analyze_capture.py '<baseline.json>' '<candidate.json>' --output out/ProfilerComparison/run-01.json
python Tools/Profiler/analyze_capture.py '<movie.json>' --first-frame 12384 --last-frame 12403 --output out/ProfilerComparison/movie-segment-01.json
python -m unittest discover -s Tools/Profiler -p 'test_*.py'
```

`python`이 Windows Store 별칭이면 설치된 Python 실행 파일의 절대 경로를 사용한다.
입력 SHA-256, 선택 frame 범위, nearest-rank P50/P95/P99, 전체 interval과 CPU frame,
유효 GPU frame/pass, 겹친 GPU scope 합집합, inclusive CPU/cpuWork, counters와 누락을 기록한다.
두 입력은 저장 시점 metadata 차이와 관찰된 평균 interval 비율도 출력한다. 같은 metadata만으로
과거 camera path, tool window, warmup, 전원 상태가 같다고 승인하지 않는다.

CPU·cpuWork의 부모와 자식은 중첩된다. GPU pending은0ms가 아니며 CPU/GPU 경과를 더하지 않는다.
이 도구는 raw scope의 Self를 계산하지 않는다. 누락된 자식을 모르는 상태에서 가짜 exclusive
병목을 만들지 않기 위해서다. raw scope 합계는 partial frame에서 하한일 수 있다.
`frameIntervalMs`는 Begin(N-1)→Begin(N), CPU scope는 N의 실행이므로 단일 spike를 결합할 때 주의한다.

전후 성능 검증은 같은 장면·camera·해상도·화질·빌드·도구창·Detailed 설정으로 진행한다.
현재 F7은 Debug 전용이다. Capture를 끈 뒤 GPU pending 회수를 기다리고 저장한다.
Frames 선택창 밖의 초기 구간은 저장되지 않을 수 있다. 저장본에 없는 최저FPS를 추정하지 않는다.

구조와 구현 정본: [프레임 통합 PLAN](../../.md/GB/10-02/2026-10-02_FRAME_PIPELINE_OPTIMIZATION_IMPLEMENTATION_PLAN.md).
