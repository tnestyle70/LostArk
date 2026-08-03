#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CBody_Valtan final : public CPartObject
{
public:
	typedef struct tagBodyValtanDesc : public CPartObject::PARTOBJECT_DESC
	{
		/* Which level registered the shader and model prototypes. Defaults to
		the level that has always owned them so existing callers keep working. */
		uint32_t iPrototypeLevelIndex = { ETOUI(LEVEL::ASSET_TEST) };
	} BODY_VALTAN_DESC;

private:
	CBody_Valtan(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CBody_Valtan();

public:
	/* The body owns clip playback: it rewinds the track and reports when a
	non-looping clip has run out. CValtan only names the clip it wants. */
	bool_t Set_Animation(
		const char_t* pAnimationName,
		bool_t isLoop);
	bool_t Is_AnimationFinished() const {
		return m_isAnimationFinished;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	bool_t m_isAnimationFinished = { false };

private:
	HRESULT Ready_Components(uint32_t iPrototypeLevelIndex);
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CBody_Valtan> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
