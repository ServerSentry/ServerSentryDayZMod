/**
	\file ServerSentryLiveActionResolver.c
	\brief Implements live-action support for Live action resolver.
*/

/**
	\brief Resolves player and world-object targets for live actions.
*/
class ServerSentryLiveActionResolver
{
    /**
    	\brief Resolves the target player from the request identifiers.
    	\param ctx \p action context containing request, target, and input data.
    	\return \p The resulting player base instance, or null when unavailable.
    */
    static PlayerBase FindPlayer(ServerSentryModActionContext ctx)
    {
        if (!ctx)
        {
            return null;
        }

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        foreach (Man man: players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (!player)
            {
                continue;
            }

            if (ctx.objectId != "" && player.ToString() == ctx.objectId)
            {
                return player;
            }

            PlayerIdentity identity = player.GetIdentity();
            if (!identity)
            {
                continue;
            }

            if (ctx.steamId != "" && identity.GetPlainId() == ctx.steamId)
            {
                return player;
            }

            if (ctx.bohemiaId != "" && identity.GetId() == ctx.bohemiaId)
            {
                return player;
            }
        }

        return null;
    }

    /**
    	\brief Resolves the target world object from the request identifiers.
    	\param ctx \p action context containing request, target, and input data.
    	\return \p The resulting entity ai instance, or null when unavailable.
    */
    static EntityAI FindObject(ServerSentryModActionContext ctx)
    {
        if (!ctx)
        {
            return null;
        }

        PlayerBase player = FindPlayer(ctx);
        if (player)
        {
            return player;
        }

        EntityAI baseObject = ServerSentryLiveApi.FindLiveBaseObjectById(ctx.objectId);
        if (baseObject)
        {
            return baseObject;
        }

        Transport vehicle = ServerSentryLiveApi.FindLiveVehicleById(ctx.objectId);
        if (vehicle)
        {
            return EntityAI.Cast(vehicle);
        }

        return null;
    }
}
