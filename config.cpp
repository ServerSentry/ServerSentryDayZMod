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
		dir = "ServerSentryMod";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Server Sentry";
		credits = "";
		author = "Krypton91";
		authorID = "0";
		version = "1.4.8";
		extra = 0;
		type = "mod";
		inputs = "ServerSentryMod/Inputs.xml";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/defines","ServerSentryMod/scripts/3_Game"};
			};
		
			class worldScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/defines","ServerSentryMod/scripts/4_World"};
			};

			class missionScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/defines","ServerSentryMod/scripts/5_Mission"};
			};
		};
	};
};
