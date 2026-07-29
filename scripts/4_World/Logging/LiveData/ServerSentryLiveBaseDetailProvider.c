/**
    \file ServerSentryLiveBaseDetailProvider.c
    \brief Defines the extension contract for Live Map base details.
*/

/**
    \brief Base class for integrations that add or override Live Map base details.
*/
class ServerSentryLiveBaseDetailProvider
{
    /**
        \brief Returns whether this provider applies to the supplied base object.
        \param entity \p live world entity represented by the DTO.
        \param dto \p current base DTO including all Vanilla details.
        \return \p true when ApplyDetails should be called; otherwise false.
    */
    bool Supports(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
        return true;
    }

    /**
        \brief Adds, overrides, or removes details for the supplied base object.
        \param entity \p live world entity represented by the DTO.
        \param dto \p mutable base DTO including all Vanilla details.
    */
    void ApplyDetails(EntityAI entity, ServerSentryLiveBaseDto dto)
    {
    }
}
