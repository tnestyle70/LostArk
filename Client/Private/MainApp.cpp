#include "MainApp.h"
#include "GameInstance.h"

#include "Level_Loading.h"

Client::CMainApp::CMainApp()
{


    /* wireframe or solid, cullmode -> cw, ccw */
   // D3D11_RASTERIZER_DESC           RSDesc{};
   // ComPtr<ID3D11RasterizerState>    pRSState = {};

   // if (FAILED(m_pDevice->CreateRasterizerState(&RSDesc, &pRSState)))
   //     return;

    
   // m_pContext->RSSetState(pRSState.Get());

   // /* ±Ì¿Ã ∫Ò±≥ x or o, ±Ì¿Ã ±‚∑œ x or o */
   // D3D11_DEPTH_STENCIL_DESC
   // m_pContext->OMSetDepthStencilState();

   // /* ∫Ì∑ªµ˘ø° ¥Î«— º≥¡§. */
   // D3D11_BLEND_DESC
   // m_pContext->OMSetBlendState();
   // 
   // /*
   // m_pDevice->SetSamplerState(D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
   // m_pDevice->SetSamplerState(D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   // m_pDevice->SetSamplerState(D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);*/

   ////D3D11_SAMPLER_DESC

   // //D3D11_SAMPLER_DESC      SamplerDesc{};
   // //ID3D11SamplerState* pSamplerState = {};

   // //m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState);

   // 
    


}

Client::CMainApp::~CMainApp()
{
    Free();
}

HRESULT CMainApp::Initialize()
{
    ENGINE_DESC         EngineDesc{};
    EngineDesc.hInstance = g_hInst;
    EngineDesc.hWnd = g_hWnd;
    EngineDesc.eWinMode = WINMODE::WIN;
    EngineDesc.iNumLevels = ETOUI(LEVEL::END);
    EngineDesc.iWinSizeX = g_iWinSizeX;
    EngineDesc.iWinSizeY = g_iWinSizeY;

    if (FAILED(CGameInstance::Get().Initialize_Engine(EngineDesc, m_pDevice, m_pContext)))
        return E_FAIL;

    if (FAILED(Ready_Gara()))
        return E_FAIL;

    if (FAILED(Ready_Fonts()))
        return E_FAIL;

    if (FAILED(Ready_Prototype_For_Static()))
        return E_FAIL;

    if (FAILED(Start_Level(LEVEL::LOGO)))
        return E_FAIL;

    return S_OK;
}

void CMainApp::Update(f32_t fTimeDelta)
{
    CGameInstance::Get().Update_Engine(fTimeDelta);
}

HRESULT CMainApp::Render()
{
    float4_t        vClearColor = { 0.f, 0.f, 1.f, 1.f };

    if (FAILED(CGameInstance::Get().Render_Begin(&vClearColor)))
        return E_FAIL;

    if (FAILED(CGameInstance::Get().Render()))
        return E_FAIL;

    CGameInstance::Get().Draw_Text(TEXT("Font_Default"), TEXT("«—±€ ¿Ã¥Ÿ12abd"), float2_t(0.f, 0.f));


    if (FAILED(CGameInstance::Get().Render_End()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Ready_Gara()
{   
    uint32_t        iByte = {};
    HANDLE          hFile = CreateFile(TEXT("../Bin/DataFiles/Navigation.dat"), GENERIC_WRITE, 0 ,nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    float3_t          vPoints[3] = {};

    vPoints[0] = float3_t(0.f, 0.f, 10.f);
    vPoints[1] = float3_t(10.f, 0.f, 0.f);
    vPoints[2] = float3_t(0.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(0.f, 0.f, 10.f);
    vPoints[1] = float3_t(10.f, 0.f, 10.f);
    vPoints[2] = float3_t(10.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(0.f, 0.f, 20.f);
    vPoints[1] = float3_t(10.f, 0.f, 10.f);
    vPoints[2] = float3_t(0.f, 0.f, 10.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    vPoints[0] = float3_t(10.f, 0.f, 10.f);
    vPoints[1] = float3_t(20.f, 0.f, 0.f);
    vPoints[2] = float3_t(10.f, 0.f, 0.f);
    WriteFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

    CloseHandle(hFile);

    ComPtr<ID3D11Texture2D>         pTexture2D = { nullptr };

    D3D11_TEXTURE2D_DESC            TextureDesc{};

    TextureDesc.Width = 256;
    TextureDesc.Height = 256;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;  
    TextureDesc.Usage = D3D11_USAGE_STAGING;
    TextureDesc.BindFlags = 0;
    TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    TextureDesc.MiscFlags = 0;

    shared_ptr<uint32_t[]>      pPixels = make_shared<uint32_t[]>(TextureDesc.Width * TextureDesc.Height);

    pPixels[0] = 0xffffffff;

    D3D11_SUBRESOURCE_DATA      InitialDesc{};
    InitialDesc.pSysMem = pPixels.get();
    InitialDesc.SysMemPitch = TextureDesc.Width * sizeof(uint32_t);

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, &InitialDesc, &pTexture2D)))
        return E_FAIL;

    D3D11_MAPPED_SUBRESOURCE            SubResource{};

    m_pContext->Map(pTexture2D.Get(), 0, D3D11_MAP_READ_WRITE, 0, &SubResource);

    auto        pLockPixels = static_cast<uint32_t*>(SubResource.pData);

    for (uint32_t i = 0; i < 256; i++)
    {
        for (uint32_t j = 0; j < 256; j++)
        {
            uint32_t        iIndex = i * 256 + j;

            /* a b g r */
            if(j < 128)
                pLockPixels[iIndex] = 0xffffffff;
            else
                pLockPixels[iIndex] = 0xff000000;
        }
    }







    if (FAILED(DirectX::SaveDDSTextureToFile(m_pContext.Get(), pTexture2D.Get(), TEXT("../Bin/Resources/Textures/Terrain/MyMask.dds"))))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Ready_Fonts()
{
    /*
MakeSpriteFont "≥ÿΩºlv1∞ÌµÒ Bold" /FontSize:20 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 161ex.spritefont
*/


    if (FAILED(CGameInstance::Get().Add_Font(TEXT("Font_Default"), TEXT("../Bin/Resources/Fonts/161ex.spritefont"))))
        return E_FAIL;
       

    return S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
    /* For.Prototype_Component_Shader_VtxTex */

    if(FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements))))
        return E_FAIL;

    /* For.Prototype_Component_VIBuffer_Rect */
    if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        return E_FAIL;



    return S_OK;
}

HRESULT CMainApp::Start_Level(LEVEL eStartLevelID)
{
    if (FAILED(CGameInstance::Get().Change_Level(ETOUI(LEVEL::LOADING),
        CLevel_Loading::Create(m_pDevice, m_pContext, eStartLevelID))))
        return E_FAIL;

    return S_OK;
}

unique_ptr<CMainApp> CMainApp::Create()
{
    auto pInstance = unique_ptr<CMainApp>(new CMainApp());

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CMainApp");
        return nullptr;
    }

    return pInstance;
}

void CMainApp::Free()
{
    CGameInstance::Get().Release_Engine();

}
