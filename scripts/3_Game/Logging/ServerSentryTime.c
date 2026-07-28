/**
	\file ServerSentryTime.c
	\brief Provides logging support for Time.
*/

/**
	\brief Provides ServerSentry behavior for Time.
*/
class ServerSentryTime
{
    /**
    	\brief Processes the now utc operation.
    	\return \p The resulting string value.
    */
    static string NowUtc()
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;

        GetYearMonthDayUTC(year, month, day);
        GetHourMinuteSecondUTC(hour, minute, second);

        return string.Format("%1-%2-%3T%4:%5:%6Z", year.ToStringLen(4), month.ToStringLen(2), day.ToStringLen(2), hour.ToStringLen(2), minute.ToStringLen(2), second.ToStringLen(2));
    }
};
