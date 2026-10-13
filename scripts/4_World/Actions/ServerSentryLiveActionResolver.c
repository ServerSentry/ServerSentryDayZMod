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
            return vehicle;
        }

        return null;
    }

    /**
    	\brief Resolves and validates a tracked vehicle target.
    */
    static Transport FindVehicle(ServerSentryModActionContext ctx)
    {
        if (!ctx)
        {
            return null;
        }

        Transport vehicle = ServerSentryLiveApi.FindLiveVehicleById(ctx.objectId);
        if (vehicle)
        {
            return vehicle;
        }

        return Transport.Cast(FindObject(ctx));
    }

    /**
    	\brief Resolves and validates a tracked flag-pole target.
    */
    static EntityAI FindFlagPole(ServerSentryModActionContext ctx)
    {
        EntityAI targetObject = FindObject(ctx);
        if (IsFlagPole(targetObject))
        {
            return targetObject;
        }

        return null;
    }

    /**
    	\brief Returns whether an entity is a territory flag / flag pole rather than a deployment kit.
    */
    static bool IsFlagPole(EntityAI entity)
    {
        if (!entity)
        {
            return false;
        }

        string className = entity.GetType();
        string normalized = className;
        normalized.ToLower();

        if (normalized.IndexOf("kit") > -1)
        {
            return false;
        }

        if (normalized == "territoryflag")
        {
            return true;
        }

        if (normalized == "flagpole")
        {
            return true;
        }

        if (normalized.IndexOf("territoryflag") > -1)
        {
            return true;
        }

        if (normalized.IndexOf("flagpole") > -1)
        {
            return true;
        }

        if (normalized.IndexOf("flag_pole") > -1)
        {
            return true;
        }

        if (normalized.IndexOf("baseflag") > -1)
        {
            return true;
        }

        if (normalized.IndexOf("flag") > -1)
        {
            if (normalized.IndexOf("territory") > -1)
            {
                return true;
            }
        }

        return false;
    }
}
