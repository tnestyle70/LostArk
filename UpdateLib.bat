// 명령어			옵션				원본 파일의 위치			사본 파일을 저장할 위치

xcopy			/y/s			.\Engine\Public\*.*				.\EngineSDK\inc\
xcopy			/y				.\Engine\Bin\*.lib				.\EngineSDK\lib\
xcopy			/y				.\Engine\ThirdPartyLib\*.lib		.\EngineSDK\lib\
xcopy			/y				.\Engine\Bin\*.dll				.\Client\bin\

xcopy			/y				.\Engine\Bin\ShaderFiles\*.*	.\EngineSDK\hlsl\
xcopy			/y				.\EngineSDK\hlsl\				.\Client\bin\ShaderFiles\
