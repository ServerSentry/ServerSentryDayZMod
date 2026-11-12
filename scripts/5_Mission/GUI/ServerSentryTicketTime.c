/** Formats UTC ticket timestamps using the player's current system time-zone offset. */
class ServerSentryTicketTime
{
    static string LocalDisplay(string timestamp)
    {
        int year, month, day, utcYear, utcMonth, utcDay;
        int hour, minute, second, utcHour, utcMinute, utcSecond;
        GetYearMonthDay(year, month, day);
        GetYearMonthDayUTC(utcYear, utcMonth, utcDay);
        GetHourMinuteSecond(hour, minute, second);
        GetHourMinuteSecondUTC(utcHour, utcMinute, utcSecond);
        int offset = (hour - utcHour) * 60 + minute - utcMinute;
        int localDate = year * 10000 + month * 100 + day;
        int utcDate = utcYear * 10000 + utcMonth * 100 + utcDay;
        if (localDate > utcDate) offset += 1440;
        if (localDate < utcDate) offset -= 1440;
        return AtOffset(timestamp, offset, year, month, day);
    }

    static string AtOffset(string timestamp, int offsetMinutes, int todayYear, int todayMonth, int todayDay)
    {
        if (timestamp.Length() < 19) return "";
        int year = timestamp.Substring(0, 4).ToInt();
        int month = timestamp.Substring(5, 2).ToInt();
        int day = timestamp.Substring(8, 2).ToInt();
        int hour = timestamp.Substring(11, 2).ToInt();
        int minute = timestamp.Substring(14, 2).ToInt();
        if (year < 1 || month < 1 || month > 12 || day < 1 || day > DaysInMonth(year, month) || hour > 23 || minute > 59) return "";
        int time = hour * 60 + minute + offsetMinutes;
        if (time < 0)
        {
            time += 1440;
            day--;
            if (day == 0)
            {
                month--;
                if (month == 0) { month = 12; year--; }
                day = DaysInMonth(year, month);
            }
        }
        else if (time >= 1440)
        {
            time -= 1440;
            day++;
            if (day > DaysInMonth(year, month))
            {
                day = 1;
                month++;
                if (month == 13) { month = 1; year++; }
            }
        }
        int localHour = time / 60;
        int localMinute = time % 60;
        string result = localHour.ToStringLen(2) + ":" + localMinute.ToStringLen(2);
        if (year == todayYear && month == todayMonth && day == todayDay) return result;
        string date = day.ToStringLen(2) + "." + month.ToStringLen(2) + ".";
        if (year != todayYear) date += year.ToString();
        return date + " " + result;
    }

    protected static int DaysInMonth(int year, int month)
    {
        if (month == 2)
        {
            if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) return 29;
            return 28;
        }
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
}
