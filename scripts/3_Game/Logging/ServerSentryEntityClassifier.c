/**
	\file ServerSentryEntityClassifier.c
	\brief Provides logging support for Entity classifier.
*/

/**
	\brief Provides ServerSentry behavior for Entity classifier.
*/
class ServerSentryEntityClassifier
{
    /**
    	\brief Processes the fill player operation.
    	\param actor \p actor used by the operation.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    */
    static void FillPlayer(ServerSentryActor actor, DayZPlayer player, PlayerIdentity identity)
    {
        if (!actor)
        {
            return;
        }

        if (player)
        {
            actor.className = player.GetType();
        }

        actor.type = ResolvePlayerLikeType(player, identity);
        actor.entityKind = ResolveEntityKind(actor.type, actor.className);

        if (identity)
        {
            actor.steamId = identity.GetPlainId();
            actor.name = identity.GetName();
            actor.bohemiaId = identity.GetId();
        }
        else
        {
            //AI
            actor.steamId = "76561111111111111";
            actor.name = "(AI) " + player.GetType();
            actor.bohemiaId = "!!AI!!";
        }

        if (actor.name == "" && player)
        {
            actor.name = player.GetType();
        }
    }

    /**
    	\brief Processes the fill entity operation.
    	\param actor \p actor used by the operation.
    	\param entity \p entity processed by the operation.
    */
    static void FillEntity(ServerSentryActor actor, EntityAI entity)
    {
        if (!actor)
        {
            return;
        }

        if (!entity)
        {
            actor.type = "unknown";
            actor.entityKind = "unknown";
            return;
        }

        actor.className = entity.GetType();

        DayZPlayer player = DayZPlayer.Cast(entity);
        if (player)
        {
            FillPlayer(actor, player, player.GetIdentity());
            return;
        }

        actor.type = ResolveTypeFromClassName(actor.className);
        actor.entityKind = ResolveEntityKind(actor.type, actor.className);
        actor.name = actor.className;
    }

    /**
    	\brief Applies type hint.
    	\param actor \p actor used by the operation.
    	\param entity \p entity processed by the operation.
    	\param entityType \p entity type used by the operation.
    */
    static void ApplyTypeHint(ServerSentryActor actor, EntityAI entity, string entityType)
    {
        if (!actor || entityType == "")
        {
            return;
        }

        if (entity && actor.className == "")
        {
            actor.className = entity.GetType();
        }

        actor.type = entityType;
        actor.entityKind = ResolveEntityKind(entityType, actor.className);

        if (actor.name == "" && actor.className != "")
        {
            actor.name = actor.className;
        }
    }

    /**
    	\brief Processes the fill environment operation.
    	\param actor \p actor used by the operation.
    	\param name \p name used by the operation.
    */
    static void FillEnvironment(ServerSentryActor actor, string name)
    {
        if (!actor)
        {
            return;
        }

        actor.type = "environment";
        actor.entityKind = "environment";
        actor.className = "";
        actor.name = name;
    }

    /**
    	\brief Resolves player like type.
    	\param player \p player entity associated with the operation.
    	\param identity \p player identity associated with the operation.
    	\return \p The resulting string value.
    */
    static string ResolvePlayerLikeType(DayZPlayer player, PlayerIdentity identity)
    {
        if (identity)
        {
            return "player";
        }

        if (player)
        {
            return "ai";
        }

        return "unknown";
    }

    /**
    	\brief Resolves type from class name.
    	\param className \p class name used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveTypeFromClassName(string className)
    {
        if (className == "")
        {
            return "unknown";
        }

        if (IsZombieClass(className))
        {
            return "zombie";
        }

        if (IsAnimalClass(className))
        {
            return "animal";
        }

        if (IsAiClass(className))
        {
            return "ai";
        }

        return "object";
    }

    /**
    	\brief Resolves entity kind.
    	\param entityType \p entity type used by the operation.
    	\param className \p class name used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveEntityKind(string entityType, string className)
    {
        if (entityType == "player")
        {
            return "human_player";
        }

        if (entityType == "ai")
        {
            return "human_ai";
        }

        if (entityType == "zombie")
        {
            return "infected";
        }

        if (entityType == "animal")
        {
            return ResolveAnimalKind(className);
        }

        if (entityType == "environment")
        {
            return "environment";
        }

        return entityType;
    }

    /**
    	\brief Returns whether ai class.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsAiClass(string className)
    {
        return className.Contains("eAI_") || className.Contains("SurvivorM_") || className.Contains("SurvivorF_");
    }

    /**
    	\brief Returns whether zombie class.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsZombieClass(string className)
    {
        return className.Contains("Zmb") || className.Contains("Zombie") || className.Contains("Infected");
    }

    /**
    	\brief Returns whether animal class.
    	\param className \p class name used by the operation.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    static bool IsAnimalClass(string className)
    {
        return className.Contains("Animal_") || className.Contains("CanisLupus") || className.Contains("UrsusArctos") || className.Contains("SusDomesticus") || className.Contains("CervusElaphus") || className.Contains("CapreolusCapreolus") || className.Contains("RangiferTarandus") || className.Contains("CapraHircus") || className.Contains("OvisAries") || className.Contains("BosTaurus") || className.Contains("GallusGallus");
    }

    /**
    	\brief Resolves animal kind.
    	\param className \p class name used by the operation.
    	\return \p The resulting string value.
    */
    static string ResolveAnimalKind(string className)
    {
        if (className.Contains("CanisLupus"))
        {
            return "wolf";
        }

        if (className.Contains("UrsusArctos"))
        {
            return "bear";
        }

        if (className.Contains("SusDomesticus"))
        {
            return "boar";
        }

        if (className.Contains("CervusElaphus") || className.Contains("CapreolusCapreolus") || className.Contains("RangiferTarandus"))
        {
            return "deer";
        }

        if (className.Contains("CapraHircus"))
        {
            return "goat";
        }

        if (className.Contains("OvisAries"))
        {
            return "sheep";
        }

        if (className.Contains("BosTaurus"))
        {
            return "cow";
        }

        if (className.Contains("GallusGallus"))
        {
            return "chicken";
        }

        return "animal";
    }
};
