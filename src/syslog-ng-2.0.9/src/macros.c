/*
 * Copyright (c) 2002-2007 BalaBit IT Ltd, Budapest, Hungary                    
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Note that this permission is granted for only version 2 of the GPL.
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "macros.h"
#include "logmsg.h"
#include "syslog-names.h"
#include "messages.h"
#include "misc.h"

#include "filter.h"

#include <time.h>
#include <string.h>

struct macro_def
{
  char *name;
  int id;
}
macros[] =
{
        { "FACILITY", M_FACILITY },
        { "FACILITY_NUM", M_FACILITY_NUM },
        { "PRIORITY", M_LEVEL },
        { "LEVEL", M_LEVEL },
        { "LEVEL_NUM", M_LEVEL_NUM },
        { "TAG", M_TAG },
        { "PRI", M_PRI },

        { "DATE", M_DATE },
        { "FULLDATE", M_FULLDATE },
        { "ISODATE", M_ISODATE },
        { "STAMP", M_STAMP },
        { "YEAR", M_YEAR },
        { "MONTH", M_MONTH },
        { "DAY", M_DAY },
        { "HOUR", M_HOUR },
        { "MIN", M_MIN },
        { "SEC", M_SEC },
        { "WEEKDAY", M_WEEKDAY },
        { "WEEK", M_WEEK },
        { "UNIXTIME", M_UNIXTIME },
        { "TZOFFSET", M_TZOFFSET },
        { "TZ", M_TZ },

        { "R_DATE", M_DATE_RECVD },
        { "R_FULLDATE", M_FULLDATE_RECVD },
        { "R_ISODATE", M_ISODATE_RECVD },
        { "R_STAMP", M_STAMP_RECVD },
        { "R_YEAR", M_YEAR_RECVD },
        { "R_MONTH", M_MONTH_RECVD },
        { "R_DAY", M_DAY_RECVD },
        { "R_HOUR", M_HOUR_RECVD },
        { "R_MIN", M_MIN_RECVD },
        { "R_SEC", M_SEC_RECVD },
        { "R_WEEKDAY", M_WEEKDAY_RECVD },
        { "R_WEEK", M_WEEK_RECVD },
        { "R_UNIXTIME", M_UNIXTIME_RECVD },
        { "R_TZOFFSET", M_TZOFFSET_RECVD },
        { "R_TZ", M_TZ_RECVD },

        { "S_DATE", M_DATE_STAMP },
        { "S_FULLDATE", M_FULLDATE_STAMP },
        { "S_ISODATE", M_ISODATE_STAMP },
        { "S_STAMP", M_STAMP_STAMP },
        { "S_YEAR", M_YEAR_STAMP },
        { "S_MONTH", M_MONTH_STAMP },
        { "S_DAY", M_DAY_STAMP },
        { "S_HOUR", M_HOUR_STAMP },
        { "S_MIN", M_MIN_STAMP },
        { "S_SEC", M_SEC_STAMP },
        { "S_WEEKDAY", M_WEEKDAY_STAMP },
        { "S_WEEK", M_WEEK_STAMP },
        { "S_UNIXTIME", M_UNIXTIME_STAMP },
        { "S_TZOFFSET", M_TZOFFSET_STAMP },
        { "S_TZ", M_TZ_STAMP },

        { "HOST_FROM", M_HOST_FROM },
        { "FULLHOST_FROM", M_FULLHOST_FROM },
        { "HOST", M_HOST },
        { "FULLHOST", M_FULLHOST },

        { "PROGRAM", M_PROGRAM },
        { "PID", M_PID },
        { "MSG", M_MESSAGE },
        { "MSGONLY", M_MSGONLY },
        { "MESSAGE", M_MESSAGE },
        { "SOURCEIP", M_SOURCE_IP }
};

GHashTable *macro_hash;

static void
result_append(GString *result, guchar *str, gsize len, gboolean escape)
{
  gint i;
  
  if (escape)
    {
      for (i = 0; i < len; i++)
        {
          if (str[i] == '\'' || str[i] == '"' || str[i] == '\\')
            {
              g_string_append_c(result, '\\');
              g_string_append_c(result, str[i]);
            }
          else if (str[i] < ' ')
            {
              g_string_sprintfa(result, "\\%03o", str[i]);
            }
          else
            g_string_append_c(result, str[i]);
        }
    }
  else
    g_string_append_len(result, str, len);
    
}

gboolean
log_macro_expand(GString *result, gint id, guint32 flags, gint ts_format, glong zone_offset, gint frac_digits, LogMessage *msg)
{
  if (id >= M_MATCH_REF_OFS)
    {
      gint ndx = id - M_MATCH_REF_OFS;
      /* match reference */
      if (ndx < msg->num_re_matches)
        result_append(result, msg->re_matches[ndx], strlen(msg->re_matches[ndx]), !!(flags & MF_ESCAPE_RESULT));
      
      return TRUE;
    }
  switch (id)
    {
    case M_FACILITY:
      {
        /* facility */
        const char *n;
        
        n = syslog_name_lookup_name_by_value(msg->pri & LOG_FACMASK, sl_facilities);
        if (n)
          {
            g_string_append(result, n);
          }
        else
          {
            g_string_sprintfa(result, "%x", (msg->pri & LOG_FACMASK) >> 3);
          }
        break;
      }
    case M_FACILITY_NUM:
      {
        g_string_sprintfa(result, "%d", (msg->pri & LOG_FACMASK) >> 3);
        break;
      }
    case M_LEVEL:
      {
        /* level */
        const char *n;
        
        n = syslog_name_lookup_name_by_value(msg->pri & LOG_PRIMASK, sl_levels);
        if (n)
          {
            g_string_append(result, n);
          }
        else
          {
            g_string_sprintfa(result, "%d", msg->pri & LOG_PRIMASK);
          }

        break;
      }
    case M_LEVEL_NUM:
      {
        g_string_sprintfa(result, "%d", (msg->pri & LOG_PRIMASK));
        break;
      }
    case M_TAG:
      {
        g_string_sprintfa(result, "%02x", msg->pri);
        break;
      }
    case M_PRI:
      {
        g_string_sprintfa(result, "%d", msg->pri);
        break;
      }
    case M_FULLHOST_FROM:
    case M_FULLHOST:
      {
        GString *host = (id == M_FULLHOST ? &msg->host : &msg->host_from);
        /* full hostname */
        result_append(result, host->str, host->len, !!(flags & MF_ESCAPE_RESULT));
        break;
      }
    case M_HOST_FROM:
    case M_HOST:
      {
        GString *host = (id == M_HOST ? &msg->host : &msg->host_from);
        /* host */
        gchar *p1 = memchr(host->str, '@', host->len);
        gchar *p2;
        int remaining, length;

        if (p1)
          p1++;
        else
          p1 = host->str;
        remaining = host->len - (p1 - host->str);
        p2 = memchr(p1, '/', remaining);
        length = p2 ? p2 - p1 
                    : host->len - (p1 - host->str);
        result_append(result, p1, length, !!(flags & MF_ESCAPE_RESULT));
        break;
      }
    case M_PROGRAM:
      {
        /* program */
        if (msg->program.len)
          {
            result_append(result, msg->program.str, msg->program.len, !!(flags & MF_ESCAPE_RESULT));
          }
        break;
      }
    case M_PID:
      {
        /* PID */
        if (msg->msg.len)
          {
            gchar *start, *end;
            
            start = strchr(msg->msg.str, '[');
            if (start)
              {
                start++;
                end = strchr(start, ']');
                if (end)
                  {
                    result_append(result, start, end-start, !!(flags & MF_ESCAPE_RESULT));
                  }
              }
          }
        break;
      }
    case M_DATE:
    case M_FULLDATE:
    case M_ISODATE:
    case M_STAMP:
    case M_WEEKDAY:
    case M_WEEK:
    case M_YEAR:
    case M_MONTH:
    case M_DAY:
    case M_HOUR:
    case M_MIN:
    case M_SEC:
    case M_TZOFFSET:
    case M_TZ:
    case M_UNIXTIME:

    case M_DATE_RECVD:
    case M_FULLDATE_RECVD:
    case M_ISODATE_RECVD:
    case M_STAMP_RECVD:
    case M_WEEKDAY_RECVD:
    case M_WEEK_RECVD:
    case M_YEAR_RECVD:
    case M_MONTH_RECVD:
    case M_DAY_RECVD:
    case M_HOUR_RECVD:
    case M_MIN_RECVD:
    case M_SEC_RECVD:
    case M_TZOFFSET_RECVD:
    case M_TZ_RECVD:
    case M_UNIXTIME_RECVD:

    case M_DATE_STAMP:
    case M_FULLDATE_STAMP:
    case M_ISODATE_STAMP:
    case M_STAMP_STAMP:
    case M_WEEKDAY_STAMP:
    case M_WEEK_STAMP:
    case M_YEAR_STAMP:
    case M_MONTH_STAMP:
    case M_DAY_STAMP:
    case M_HOUR_STAMP:
    case M_MIN_STAMP:
    case M_SEC_STAMP:
    case M_TZOFFSET_STAMP:
    case M_TZ_STAMP:
    case M_UNIXTIME_STAMP:
      {
        /* year, month, day */
        struct tm *tm;
        gchar buf[64];
        gint length;
        time_t t;
        LogStamp *stamp;
        glong zone_ofs;

        if (id >= M_DATE_RECVD && id <= M_UNIXTIME_RECVD)
          {
            stamp = &msg->recvd;
            id = id - (M_DATE_RECVD - M_DATE);
          }
        else if (id >= M_DATE_STAMP && id <= M_UNIXTIME_STAMP)
          {
            stamp = &msg->stamp;
            id = id - (M_DATE_STAMP - M_DATE);
          }
        else
          {
            if (flags & MF_STAMP_RECVD)
              stamp = &msg->recvd;
            else
              stamp = &msg->stamp;
          }

        /* try to use the following zone values in order:
         *   destination specific timezone, if one is specified
         *   message specific timezone, if one is specified
         *   local timezone
         */
        zone_ofs = (zone_offset != -1 ? zone_offset : (stamp->zone_offset != -1 ? stamp->zone_offset : get_local_timezone_ofs(stamp->time.tv_sec)));
        t = stamp->time.tv_sec + zone_ofs;
        tm = gmtime(&t);

        switch (id)
          {
          case M_WEEKDAY:
            length = strftime(buf, sizeof(buf), "%a", tm);
            g_string_append_len(result, buf, length);
            break;
          case M_WEEK:
            length = strftime(buf, sizeof(buf), "%W", tm);
            g_string_append_len(result, buf, length);
            break;
          case M_YEAR:
            g_string_sprintfa(result, "%04d", tm->tm_year + 1900);
            break;
          case M_MONTH:
            g_string_sprintfa(result, "%02d", tm->tm_mon + 1);
            break;
          case M_DAY:
            g_string_sprintfa(result, "%02d", tm->tm_mday);
            break;
          case M_HOUR:
            g_string_sprintfa(result, "%02d", tm->tm_hour);
            break;
          case M_MIN:
            g_string_sprintfa(result, "%02d", tm->tm_min);
            break;
          case M_SEC:
            g_string_sprintfa(result, "%02d", tm->tm_sec);
            break;
          case M_DATE:
          case M_STAMP:
          case M_ISODATE:
          case M_FULLDATE:
          case M_UNIXTIME:
            {
              GString *s = g_string_sized_new(0);
              gint format = id == M_DATE ? TS_FMT_BSD : 
                            id == M_ISODATE ? TS_FMT_ISO :
                            id == M_FULLDATE ? TS_FMT_FULL :
                            id == M_UNIXTIME ? TS_FMT_UNIX :
                            ts_format;
              
              log_stamp_format(stamp, s, format, zone_offset, frac_digits);
              g_string_append_len(result, s->str, s->len);
              g_string_free(s, TRUE);
              break;
            }
          case M_TZ:
          case M_TZOFFSET:
            length = format_zone_info(buf, sizeof(buf), zone_ofs);
            g_string_append_len(result, buf, length);
            break;
          }
        break;
      }
    case M_MESSAGE:
      /* message */
      result_append(result, msg->msg.str, msg->msg.len, !!(flags & MF_ESCAPE_RESULT));
      break;
    case M_MSGONLY:
      {
        gchar *colon;
        gint ofs;
        
        colon = memchr(msg->msg.str, ':', msg->msg.len);
        if (!colon)
          {
            ofs = 0;
          }
        else
          {
            colon++;
            while (*colon && *colon == ' ')
              colon++;
            ofs = colon - msg->msg.str;
          }
        result_append(result, msg->msg.str + ofs, msg->msg.len - ofs, !!(flags & MF_ESCAPE_RESULT));
        break;
      }
    case M_SOURCE_IP:
      {
        gchar *ip;
        
        if (msg->saddr && g_sockaddr_inet_check(msg->saddr)) 
          {
            gchar buf[16];
            
            g_inet_ntoa(buf, sizeof(buf), ((struct sockaddr_in *) &msg->saddr->sa)->sin_addr);
            ip = buf;
          }
        else 
          {
            ip = "127.0.0.1";
          }
        result_append(result, ip, strlen(ip), !!(flags & MF_ESCAPE_RESULT));
        break;
      }
    default:
      g_assert_not_reached();
      break;
    }
  return TRUE;
}

guint
log_macro_lookup(gchar *macro, gint len)
{
  gchar buf[256];
  
  g_strlcpy(buf, macro, MIN(sizeof(buf), len+1));
  if (!macro_hash)
    {
      int i;
      macro_hash = g_hash_table_new(g_str_hash, g_str_equal);
      for (i = 0; i < sizeof(macros) / sizeof(macros[0]); i++)
        {
          g_hash_table_insert(macro_hash, macros[i].name,
                              GINT_TO_POINTER(macros[i].id));
        }
    }
  return GPOINTER_TO_INT(g_hash_table_lookup(macro_hash, buf));

}

