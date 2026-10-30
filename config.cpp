class CfgPatches
{
	class ServerSentryScripts
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = 
		{
			"DZ_Data",
			"DZ_Scripts",
			"JM_CF_Scripts"
		};
	};
};

class CfgMods
{
	class ServerSentry
	{
		dir = "ServerSentryDayZMod";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Server Sentry";
		credits = "";
		author = "Krypton91";
		authorID = "0";
		version = "1.4.8";
		storageVersion = 1;
		extra = 0;
		type = "mod";
		inputs = "ServerSentryDayZMod/Inputs.xml";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"ServerSentryDayZMod/scripts/3_Game"};
			};
		
			class worldScriptModule
			{
				value = "";
				files[] = {"ServerSentryDayZMod/scripts/4_World"};
			};

			class missionScriptModule
			{
				value = "";
				files[] = {"ServerSentryDayZMod/scripts/5_Mission"};
			};
		};
	};
};
