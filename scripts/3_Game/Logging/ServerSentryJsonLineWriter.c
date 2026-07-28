/**
	\file ServerSentryJsonLineWriter.c
	\brief Provides logging support for JSON line writer.
*/

/**
	\brief Provides ServerSentry behavior for JSON line writer.
*/
class ServerSentryJsonLineWriter
{
    /**
    	\brief Instance state for root directory path.
    */
    protected string m_RootDirectoryPath;
    /**
    	\brief Instance state for directory path.
    */
    protected string m_DirectoryPath;
    /**
    	\brief Instance state for current file path.
    */
    protected string m_CurrentFilePath;
    /**
    	\brief Instance state for current bucket.
    */
    protected string m_CurrentBucket;
    /**
    	\brief Instance state for initialized.
    */
    protected bool m_Initialized;

    /**
    	\brief Initializes a new ServerSentryJsonLineWriter instance.
    	\param directoryPath \p directory path used by the operation.
    */
    void ServerSentryJsonLineWriter(string directoryPath = "")
    {
        m_RootDirectoryPath = ServerSentryDefaults.RootDirectory();

        if (directoryPath == "")
        {
            m_DirectoryPath = ServerSentryDefaults.EventDirectory();
        }
        else
        {
            m_DirectoryPath = directoryPath;
        }

        m_CurrentFilePath = "";
        m_CurrentBucket = "";
        m_Initialized = false;
    }

    /**
    	\brief Writes event.
    	\param ev \p ev used by the operation.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool WriteEvent(ServerSentryEvent ev)
    {
        if (!ev)
        {
            return false;
        }

        string json;
        string error;

        if (!JsonFileLoader<ServerSentryEvent>.MakeData(ev, json, error, false) || json == "")
        {
            Print("[ServerSentry] Failed to serialize event - " + error);
            return false;
        }

        array<string> lines = new array<string>();
        lines.Insert(json);
        return WriteLines(lines);
    }

    /**
    	\brief Writes lines.
    	\param lines \p collection of lines values.
    	\return \p true when the operation succeeds; otherwise false.
    */
    bool WriteLines(array<string> lines)
    {
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            string countText = "null";
            if (lines)
            {
                countText = lines.Count().ToString();
            }
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.enter] lines=" + countText);
        }

        if (!lines || lines.Count() == 0)
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.return] empty");
            }
            return true;
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.beforeEnsureFileReady]");
        }

        if (!EnsureFileReady())
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.ensureFileReadyFailed]");
            }
            return false;
        }

        string filePath = GetActiveFilePath();
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.beforeOpen] file=" + filePath);
        }

        FileHandle file = OpenFile(filePath, FileMode.APPEND);
        if (!file)
        {
            Print("[ServerSentry] Failed to open event file for append: " + filePath);
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.openFailed] file=" + filePath);
            }
            return false;
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.openOk]");
        }

        int written = 0;
        foreach (string line: lines)
        {
            if (line != "")
            {
                FPrintln(file, line);
                written++;
            }
        }

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.beforeClose] written=" + written.ToString());
        }

        CloseFile(file);

        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.WriteLines.done] file=" + filePath + " written=" + written.ToString());
        }
        return true;
    }

    /**
    	\brief Ensures file ready.
    	\return \p true when the operation succeeds; otherwise false.
    */
    protected bool EnsureFileReady()
    {
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.enter] initialized=" + BoolToDebugText(m_Initialized) + " root=" + m_RootDirectoryPath + " dir=" + m_DirectoryPath);
        }

        if (m_Initialized)
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.return] already initialized");
            }
            return true;
        }

        if (!FileExist(m_RootDirectoryPath))
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.createRoot] " + m_RootDirectoryPath);
            }

            if (!MakeDirectory(m_RootDirectoryPath))
            {
                Print("[ServerSentry] Failed to create root directory: " + m_RootDirectoryPath);
                return false;
            }
        }
        else if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.rootExists] " + m_RootDirectoryPath);
        }

        if (!FileExist(m_DirectoryPath))
        {
            if (ServerSentryDefaults.DebugInventoryMoveEvents())
            {
                Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.createEventDir] " + m_DirectoryPath);
            }

            if (!MakeDirectory(m_DirectoryPath))
            {
                Print("[ServerSentry] Failed to create event directory: " + m_DirectoryPath);
                return false;
            }
        }
        else if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.eventDirExists] " + m_DirectoryPath);
        }

        m_Initialized = true;
        if (ServerSentryDefaults.DebugInventoryMoveEvents())
        {
            Print("[ServerSentry][InventoryDebug][JsonLineWriter.EnsureFileReady.done]");
        }
        return true;
    }

    /**
    	\brief Returns active file path.
    	\return \p The resulting string value.
    */
    protected string GetActiveFilePath()
    {
        string bucket = BuildUtcHourBucket();
        if (bucket != m_CurrentBucket || m_CurrentFilePath == "")
        {
            m_CurrentBucket = bucket;
            m_CurrentFilePath = m_DirectoryPath + "/events_" + bucket + ".jsonl";
        }

        return m_CurrentFilePath;
    }

    /**
    	\brief Builds utc hour bucket.
    	\return \p The resulting string value.
    */
    protected string BuildUtcHourBucket()
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;

        GetYearMonthDayUTC(year, month, day);
        GetHourMinuteSecondUTC(hour, minute, second);

        return year.ToStringLen(4) + month.ToStringLen(2) + day.ToStringLen(2) + "_" + hour.ToStringLen(2);
    }


    /**
    	\brief Processes the bool to debug text operation.
    	\param value \p value to process or assign.
    	\return \p The resulting string value.
    */
    protected string BoolToDebugText(bool value)
    {
        if (value)
        {
            return "true";
        }

        return "false";
    }

    /**
    	\brief Returns file path.
    	\return \p The resulting string value.
    */
    string GetFilePath()
    {
        return GetActiveFilePath();
    }
};
