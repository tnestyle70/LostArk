// 이 CPP는 실행 함수를 만드는 용도가 아니라 Client 전용 미리 컴파일된 헤더(PCH)를 생성하는 빌드 입력이다.
// 번역 단위는 이 CPP에 전처리 과정에서 포함된 헤더와 매크로까지 합친 컴파일 입력을 말한다.
// Tools/Build/CppCompilation.props의 /FI 설정이 PCH.h를 소스 앞에 강제로 포함한다.
// Client.vcxproj는 이 CPP에만 PrecompiledHeader=Create(/Yc)를 설정해 그 처리 결과를 Client.pch에 저장한다.
// /FI는 헤더 포함, /Yc는 PCH 생성이라는 별개 역할이다.
// 나머지 PCH 사용 CPP는 /Yu로 이 결과를 재사용한다. 매 실행마다 읽는 게임 데이터나 객체 원형은 아니다.
// PCH.h에는 vector, map, memory 등 안정적인 표준 헤더가 들어 있고 Client/Engine 게임 헤더는 없다.
// PCH는 컴파일 비용을 줄이는 장치이므로 각 소스의 실제 타입/API 의존 include를 대신하는 규칙으로 쓰지 않는다.
// 현재 x64에서 LostArkUsePch=true일 때 이 생성 단위가 필요하다. 파일만 삭제하면 프로젝트의 입력이 없어져 빌드가 깨진다.
// PCH를 끄는 별도 빌드는 LostArkUsePch=false로 생산자와 소비자 설정을 함께 조정한다. 파일 삭제가 그 설정 변경을 대신하지 않는다.
