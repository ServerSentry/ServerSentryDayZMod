/**
	\file ServerSentryVectorDto.c
	\brief Defines the Vector DTO logging data model.
*/

/**
	\brief Data transfer object for Vector.
*/
class ServerSentryVectorDto
{
    /**
    	\brief Stores the x.
    */
    float x;
    /**
    	\brief Stores the y.
    */
    float y;
    /**
    	\brief Stores the z.
    */
    float z;

    /**
    	\brief Stores the zone.
    */
    string zone;

    /**
    	\brief Initializes a new ServerSentryVectorDto instance.
    */
    void ServerSentryVectorDto()
    {
        x = 0;
        y = 0;
        z = 0;
        zone = "";
    }

    /**
    	\brief Creates from vector.
    	\param pos \p pos used by the operation.
    */
    void FromVector(vector pos)
    {
        x = pos[0];
        y = pos[1];
        z = pos[2];
    }
};
