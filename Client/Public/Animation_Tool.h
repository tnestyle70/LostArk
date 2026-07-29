#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CAnimation_Tool final
{
public:
	void Render();

private:
	shared_ptr<Engine::CModel> Resolve_Model() const;
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);

private:
	char m_Filter[128]{};
	bool_t m_bLoop = true;
};

NS_END
