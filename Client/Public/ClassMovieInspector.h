#pragma once

#include "ClassMovieInspection.h"
#include <array>
#include <string>

namespace Client
{
// Only panel preferences live here. Selection, visibility and drafts belong to Movie.
class CClassMovieInspector final
{
public:
    void Render(const CLASS_MOVIE_INSPECTION_CALLBACKS& callbacks,
        const std::string& classId, bool loop, bool unappliedRowDraft = false);

private:
    std::array<char, 192> m_Search{};
    bool m_VisibleAtCursorOnly = false;
    bool m_ShowDeleted = true;
    std::string m_Status;
};
}
