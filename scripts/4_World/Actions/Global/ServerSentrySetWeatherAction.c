/**
	\file ServerSentrySetWeatherAction.c
	\brief Implements live-action support for Set weather action.
*/

/**
	\brief Implements the Set weather ServerSentry action.
*/
class ServerSentrySetWeatherAction: ServerSentryModActionHandler
{
    /**
    	\brief Returns the metadata descriptor published for this live action.
    	\return \p The resulting live mod action descriptor DTO instance, or null when unavailable.
    */
    override ServerSentryLiveModActionDescriptorDto GetDescriptor()
    {
        ServerSentryLiveModActionDescriptorDto d = new ServerSentryLiveModActionDescriptorDto();
        d.id = "server.weather.set";
        d.targetType = "server";
        d.label = "Set Weather";
        d.icon = "CloudSun";
        d.description = "Changes server weather values for overcast, rain, fog and wind. Set all values to 0 for clean weather.";
        d.scopes.Insert("live");

        ServerSentryLiveModActionInputDto overcast = ServerSentryLiveModActionInputFactory.Number("overcast", "Overcast", false, 0, 1, 0.05, "0");
        overcast.helpText = "0 = clear sky, 1 = fully overcast.";
        d.inputs.Insert(overcast);

        ServerSentryLiveModActionInputDto rain = ServerSentryLiveModActionInputFactory.Number("rain", "Rain", false, 0, 1, 0.05, "0");
        rain.helpText = "0 = no rain, 1 = heavy rain.";
        d.inputs.Insert(rain);

        ServerSentryLiveModActionInputDto fog = ServerSentryLiveModActionInputFactory.Number("fog", "Fog", false, 0, 1, 0.05, "0");
        fog.helpText = "0 = no fog, 1 = dense fog.";
        d.inputs.Insert(fog);

        ServerSentryLiveModActionInputDto windSpeed = ServerSentryLiveModActionInputFactory.Number("windSpeed", "Wind speed", false, 0, 60, 0.1, "0");
        windSpeed.helpText = "0 = calm wind. DayZ may internally clamp extremely low wind to a tiny minimum.";
        d.inputs.Insert(windSpeed);

        ServerSentryLiveModActionInputDto windDirectionDegrees = ServerSentryLiveModActionInputFactory.Number("windDirectionDegrees", "Wind direction", false, 0, 360, 1, "0");
        windDirectionDegrees.helpText = "Wind direction in degrees from 0 to 360.";
        d.inputs.Insert(windDirectionDegrees);

        ServerSentryLiveModActionInputDto transitionSeconds = ServerSentryLiveModActionInputFactory.Number("transitionSeconds", "Transition seconds", false, 0, 3600, 1, "0");
        transitionSeconds.helpText = "How long the weather transition should take. Use 0 for immediate changes.";
        d.inputs.Insert(transitionSeconds);

        return d; 
    }

    /**
    	\brief Executes the live action using the supplied request context.
    	\param ctx \p action context containing request, target, and input data.
    	\param message \p receives a human-readable result or error message.
    	\return \p true when the operation succeeds; otherwise false.
    */
    override bool Execute(ServerSentryModActionContext ctx, out string message)
    {
        Weather weather = GetGame().GetWeather();
        if (!weather)
        {
            message = "Weather controller is not available.";
            return false;
        }

        float overcast = Clamp01(ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "overcast", 0));
        float rain = Clamp01(ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "rain", 0));
        float fog = Clamp01(ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "fog", 0));
        float transitionSeconds = ClampFloat(ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "transitionSeconds", 0), 0, 3600);

        // -1 means "not provided". A frontend value of 0 must stay a real value.
        float windSpeed = ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "windSpeed", -1);
        float windDirectionDegrees = ServerSentryLiveActionInput.GetFloat(ctx.inputJson, "windDirectionDegrees", -1);
        bool applyWindSpeed = windSpeed >= 0;
        bool applyWindDirection = windDirectionDegrees >= 0;

        bool cleanWeather = IsZero(overcast) && IsZero(rain) && IsZero(fog);
        if (applyWindSpeed && !IsZero(windSpeed))
        {
            cleanWeather = false;
        }

        // Hand control to scripted weather and stop automatic forecast changes from immediately undoing the action.
        weather.MissionWeather(true);
        weather.SetWeatherUpdateFreeze(true);

        float holdUntil = weather.GetTime() + 3600;

        if (cleanWeather)
        {
            // Rain/snow can otherwise linger because they depend on overcast thresholds and forecast timing.
            weather.SetRainThresholds(1, 1, 0);
            weather.SetSnowfallThresholds(1, 1, 0);
            weather.SetStorm(0, 1, 999999);
            weather.SuppressLightningSimulation(true);
        }
        else
        {
            // Allow rain when the requested weather actually contains rain.
            weather.SetRainThresholds(0, 1, 0);
            weather.SuppressLightningSimulation(false);
        }

        ApplyExactPhenomenon(weather.GetOvercast(), overcast, transitionSeconds, holdUntil);
        ApplyExactPhenomenon(weather.GetRain(), rain, transitionSeconds, holdUntil);
        ApplyExactPhenomenon(weather.GetFog(), fog, transitionSeconds, holdUntil);

        if (cleanWeather)
        {
            // Clean weather should also remove snowfall and dynamic volumetric fog when the map supports it.
            ApplyExactPhenomenon(weather.GetSnowfall(), 0, 0, holdUntil);

            if (weather.IsDynVolFogEnabled())
            {
                weather.SetDynVolFogDistanceDensity(0, 0);
                weather.SetDynVolFogHeightDensity(0, 0);
                weather.SetDynVolFogHeightBias(0, 0);
            }
        }

        string windMessage = "";
        if (applyWindSpeed)
        {
            windSpeed = ClampFloat(windSpeed, 0, 60);

            weather.GetWindMagnitude().SetForecastChangeLimits(0, 0);
            weather.GetWindMagnitude().SetNextChange(holdUntil);

            if (IsZero(windSpeed))
            {
                // DayZ wind can have a tiny internal minimum, but this forces the forecast/function to calm.
                weather.SetWindFunctionParams(0, 0, 0);
                weather.GetWindMagnitude().Set(0, 0, 0);
                weather.SetWindMaximumSpeed(0);
            }
            else
            {
                if (windSpeed > weather.GetWindMaximumSpeed())
                {
                    weather.SetWindMaximumSpeed(windSpeed);
                }

                weather.GetWindMagnitude().Set(windSpeed, transitionSeconds, 0);
            }

            windMessage = windMessage + ", windSpeed=" + windSpeed.ToString();
        }

        if (applyWindDirection)
        {
            windDirectionDegrees = NormalizeDegrees360(windDirectionDegrees);
            float windAngle = DegreesToWeatherAngle(windDirectionDegrees);
            weather.GetWindDirection().SetForecastChangeLimits(0, 0);
            weather.GetWindDirection().SetNextChange(holdUntil);
            weather.GetWindDirection().Set(windAngle, transitionSeconds, 0);
            windMessage = windMessage + ", windDirectionDegrees=" + windDirectionDegrees.ToString();
        }

        message = "Changed weather: overcast=" + overcast.ToString() + ", rain=" + rain.ToString() + ", fog=" + fog.ToString() + windMessage + ", transitionSeconds=" + transitionSeconds.ToString();

        if (cleanWeather)
        {
            message = message + " (clean weather forced)";
        }

        message = message + ".";

        ServerSentryLiveApi.WriteSnapshot();
        return true;
    }

    /**
    	\brief Applies exact phenomenon.
    	\param phenomenon \p phenomenon used by the operation.
    	\param value \p value to process or assign.
    	\param transitionSeconds \p transition seconds used by the operation.
    	\param holdUntil \p hold until used by the operation.
    */
    protected void ApplyExactPhenomenon(WeatherPhenomenon phenomenon, float value, float transitionSeconds, float holdUntil)
    {
        phenomenon.SetLimits(value, value);
        phenomenon.SetForecastChangeLimits(0, 0);
        phenomenon.SetNextChange(holdUntil);
        phenomenon.Set(value, transitionSeconds, 0);
    }

    /**
    	\brief Returns whether zero.
    	\param value \p value to process or assign.
    	\return \p true when the condition is satisfied; otherwise false.
    */
    protected bool IsZero(float value)
    {
        return value <= 0.0001 && value >= -0.0001;
    }

    /**
    	\brief Processes the clamp 01 operation.
    	\param value \p value to process or assign.
    	\return \p The resulting floating-point value.
    */
    protected float Clamp01(float value)
    {
        return ClampFloat(value, 0, 1);
    }

    /**
    	\brief Processes the clamp float operation.
    	\param value \p value to process or assign.
    	\param minValue \p min value used by the operation.
    	\param maxValue \p max value used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected float ClampFloat(float value, float minValue, float maxValue)
    {
        if (value < minValue)
        {
            return minValue;
        }

        if (value > maxValue)
        {
            return maxValue;
        }

        return value;
    }

    /**
    	\brief Normalizes degrees 360.
    	\param degrees \p degrees used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected float NormalizeDegrees360(float degrees)
    {
        while (degrees < 0)
        {
            degrees = degrees + 360;
        }

        while (degrees >= 360)
        {
            degrees = degrees - 360;
        }

        return degrees;
    }

    /**
    	\brief Processes the degrees to weather angle operation.
    	\param degrees \p degrees used by the operation.
    	\return \p The resulting floating-point value.
    */
    protected float DegreesToWeatherAngle(float degrees)
    {
        float normalized = NormalizeDegrees360(degrees);
        if (normalized > 180)
        {
            normalized = normalized - 360;
        }

        return normalized * 3.14159265 / 180.0;
    }
}
