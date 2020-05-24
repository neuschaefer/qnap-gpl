/* app-openpgp.c - The OpenPGP card application.
 *	Copyright (C) 2003, 2004, 2005 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * $Id: app-openpgp.c,v 1.1.1.1 2010/06/04 10:11:01 richardchang Exp $
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#if GNUPG_MAJOR_VERSION == 1
/* This is used with GnuPG version < 1.9.  The code has been source
   copied from the current GnuPG >= 1.9  and is maintained over
   there. */
#include "options.h"
#include "errors.h"
#include "memory.h"
#include "util.h"
#include "cardglue.h"
#else /* GNUPG_MAJOR_VERSION != 1 */
#include "scdaemon.h"
#endif /* GNUPG_MAJOR_VERSION != 1 */

#include "i18n.h"
#include "iso7816.h"
#include "app-common.h"
#include "tlv.h"


static struct {
  int tag;
  int constructed;
  int get_from;  /* Constructed DO with this DO or 0 for direct access. */
  int binary;
  int dont_cache;
  int flush_on_error;
  int get_immediate_in_v11; /* Enable a hack to bypass the cache of
                               this data object if it is used in 1.1
                               and later versions of the card.  This
                               does not work with composite DO and is
                               currently only useful for the CHV
                               status bytes. */
  char *desc;
} data_objects[] = {
  { 0x005E, 0,    0, 1, 0, 0, 0, "Login Data" },
  { 0x5F50, 0,    0, 0, 0, 0, 0, "URL" },
  { 0x0065, 1,    0, 1, 0, 0, 0, "Cardholder Related Data"},
  { 0x005B, 0, 0x65, 0, 0, 0, 0, "Name" },
  { 0x5F2D, 0, 0x65, 0, 0, 0, 0, "Language preferences" },
  { 0x5F35, 0, 0x65, 0, 0, 0, 0, "Sex" },
  { 0x006E, 1,    0, 1, 0, 0, 0, "Application Related Data" },
  { 0x004F, 0, 0x6E, 1, 0, 0, 0, "AID" },
  { 0x0073, 1,    0, 1, 0, 0, 0, "Discretionary Data Objects" },
  { 0x0047, 0, 0x6E, 1, 1, 0, 0, "Card Capabilities" },
  { 0x00C0, 0, 0x6E, 1, 1, 0, 0, "Extended Card Capabilities" },
  { 0x00C1, 0, 0x6E, 1, 1, 0, 0, "Algorithm Attributes Signature" },
  { 0x00C2, 0, 0x6E, 1, 1, 0, 0, "Algorithm Attributes Decryption" },
  { 0x00C3, 0, 0x6E, 1, 1, 0, 0, "Algorithm Attributes Authentication" },
  { 0x00C4, 0, 0x6E, 1, 0, 1, 1, "CHV Status Bytes" },
  { 0x00C5, 0, 0x6E, 1, 0, 0, 0, "Fingerprints" },
  { 0x00C6, 0, 0x6E, 1, 0, 0, 0, "CA Fingerprints" },
  { 0x00CD, 0, 0x6E, 1, 0, 0, 0, "Generation time" },
  { 0x007A, 1,    0, 1, 0, 0, 0, "Security Support Template" },
  { 0x0093, 0, 0x7A, 1, 1, 0, 0, "Digital Signature Counter" },
  { 0x0101, 0,    0, 0, 0, 0, 0, "Private DO 1"},
  { 0x0102, 0,    0, 0, 0, 0, 0, "Private DO 2"},
  { 0x0103, 0,    0, 0, 0, 0, 0, "Private DO 3"},
  { 0x0104, 0,    0, 0, 0, 0, 0, "Private DO 4"},
  { 0 }
};


/* One cache item for DOs.  */
struct cache_s {
  struct cache_s *next;
  int tag;
  size_t length;
  unsigned char data[1];
};


/* Object with application (i.e. OpenPGP card) specific data.  */
struct app_local_s {
  /* A linked list with cached DOs.  */
  struct cache_s *cache;
  
  /* Keep track of the public keys.  */
  struct
  {
    int read_done;   /* True if we have at least tried to read them.  */
    unsigned char *key; /* This is a malloced buffer with a canonical
                           encoded S-expression encoding a public
                           key. Might be NULL if key is not
                           available.  */
    size_t keylen;      /* The length of the above S-expression.  Thsi
                           is usullay only required for corss checks
                           because the length of an S-expression is
                           implicitly available.  */
  } pk[3];

  /* Keep track of card capabilities.  */
  struct 
  {
    unsigned int get_challenge:1;
    unsigned int key_import:1;
    unsigned int change_force_chv:1;
    unsigned int private_dos:1;
  } extcap;

  /* Flags used to control the application.  */
  struct
  {
    unsigned int no_sync:1;   /* Do not sync CHV1 and CHV2 */
    unsigned int def_chv2:1;  /* Use 123456 for CHV2.  */
  } flags;
};



/***** Local prototypes  *****/
static unsigned long convert_sig_counter_value (const unsigned char *value,
                                                size_t valuelen);
static unsigned long get_sig_counter (app_t app);





/* Deconstructor. */
static void
do_deinit (app_t app)
{
  if (app && app->app_local)
    {
      struct cache_s *c, *c2;
      int i;

      for (c = app->app_local->cache; c; c = c2)
        {
          c2 = c->next;
          xfree (c);
        }

      for (i=0; i < DIM (app->app_local->pk); i++)
        {
          xfree (app->app_local->pk[i].key);
          app->app_local->pk[i].read_done = 0;
        }
      xfree (app->app_local);
      app->app_local = NULL;
    }
}


/* Wrapper around iso7816_get_data which first tries to get the data
   from the cache.  With GET_IMMEDIATE passed as true, the cache is
   bypassed. */
static gpg_error_t
get_cached_data (app_t app, int tag, 
                 unsigned char **result, size_t *resultlen,
                 int get_immediate)
{
  gpg_error_t err;
  int i;
  unsigned char *p;
  size_t len;
  struct cache_s *c;

  *result = NULL;
  *resultlen = 0;

  if (!get_immediate)
    {
      for (c=app->app_local->cache; c; c = c->next)
        if (c->tag == tag)
          {
            if(c->length)
              {
                p = xtrymalloc (c->length);
                if (!p)
                  return gpg_error (gpg_err_code_from_errno (errno));
                memcpy (p, c->data, c->length);
                *result = p;
              }
            
            *resultlen = c->length;
            
            return 0;
          }
    }
  
  err = iso7816_get_data (app->slot, tag, &p, &len);
  if (err)
    return err;
  *result = p;
  *resultlen = len;

  /* Check whether we should cache this object. */
  if (get_immediate)
    return 0;

  for (i=0; data_objects[i].tag; i++)
    if (data_objects[i].tag == tag)
      {
        if (data_objects[i].dont_cache)
          return 0;
        break;
      }

  /* Okay, cache it. */
  for (c=app->app_local->cache; c; c = c->next)
    assert (c->tag != tag);
  
  c = xtrymalloc (sizeof *c + len);
  if (c)
    {
      memcpy (c->data, p, len);
      c->length = len;
      c->tag = tag;
      c->next = app->app_local->cache;
      app->app_local->cache = c;
    }

  return 0;
}

/* Remove DO at TAG from the cache. */
static void
flush_cache_item (app_t app, int tag)
{
  struct cache_s *c, *cprev;
  int i;

  if (!app->app_local)
    return;

  for (c=app->app_local->cache, cprev=NULL; c ; cprev=c, c = c->next)
    if (c->tag == tag)
      {
        if (cprev)
          cprev->next = c->next;
        else
          app->app_local->cache = c->next;
        xfree (c);

        for (c=app->app_local->cache; c ; c = c->next)
          {
            assert (c->tag != tag); /* Oops: duplicated entry. */
          }
        return;
      }

  /* Try again if we have an outer tag. */
  for (i=0; data_objects[i].tag; i++)
    if (data_objects[i].tag == tag && data_objects[i].get_from
        && data_objects[i].get_from != tag)
      flush_cache_item (app, data_objects[i].get_from);
}

/* Flush all entries from the cache which might be out of sync after
   an error. */
static void
flush_cache_after_error (app_t app)
{
  int i;

  for (i=0; data_objects[i].tag; i++)
    if (data_objects[i].flush_on_error)
      flush_cache_item (app, data_objects[i].tag);
}


/* Flush the entire cache. */
static void
flush_cache (app_t app)
{
  if (app && app->app_local)
    {
      struct cache_s *c, *c2;

      for (c = app->app_local->cache; c; c = c2)
        {
          c2 = c->next;
          xfree (c);
        }
      app->app_local->cache = NULL;
    }
}


/* Get the DO identified by TAG from the card in SLOT and return a
   buffer with its content in RESULT and NBYTES.  The return value is
   NULL if not found or a pointer which must be used to release the
   buffer holding value. */
static void *
get_one_do (app_t app, int tag, unsigned char **result, size_t *nbytes,
            int *r_rc)
{
  int rc, i;
  unsigned char *buffer;
  size_t buflen;
  unsigned char *value;
  size_t valuelen;
  int dummyrc;

  if (!r_rc)
    r_rc = &dummyrc;

  *result = NULL;
  *nbytes = 0;
  *r_rc = 0;
  for (i=0; data_objects[i].tag && data_objects[i].tag != tag; i++)
    ;

  if (app->card_version > 0x0100 && data_objects[i].get_immediate_in_v11)
    {
      rc = iso7816_get_data (app->slot, tag, &buffer, &buflen);
      if (rc)
        {
          *r_rc = rc;
          return NULL;
        }
      *result = buffer;
      *nbytes = buflen;
      return buffer;
    }

  value = NULL;
  rc = -1;
  if (data_objects[i].tag && data_objects[i].get_from)
    {
      rc = get_cached_data (app, data_objects[i].get_from,
                            &buffer, &buflen,
                            (data_objects[i].dont_cache 
                             || data_objects[i].get_immediate_in_v11));
      if (!rc)
        {
          const unsigned char *s;

          s = find_tlv_unchecked (buffer, buflen, tag, &valuelen);
          if (!s)
            value = NULL; /* not found */
          else if (valuelen > buflen - (s - buffer))
            {
              log_error ("warning: constructed DO too short\n");
              value = NULL;
              xfree (buffer); buffer = NULL;
            }
          else
            value = buffer + (s - buffer);
        }
    }

  if (!value) /* Not in a constructed DO, try simple. */
    {
      rc = get_cached_data (app, tag, &buffer, &buflen,
                            (data_objects[i].dont_cache 
                             || data_objects[i].get_immediate_in_v11));
      if (!rc)
        {
          value = buffer;
          valuelen = buflen;
        }
    }

  if (!rc)
    {
      *nbytes = valuelen;
      *result = value;
      return buffer;
    }
  *r_rc = rc;
  return NULL;
}


static void
dump_all_do (int slot)
{
  int rc, i, j;
  unsigned char *buffer;
  size_t buflen;
  
  for (i=0; data_objects[i].tag; i++)
    {
      if (data_objects[i].get_from)
        continue;

      rc = iso7816_get_data (slot, data_objects[i].tag, &buffer, &buflen);
      if (gpg_err_code (rc) == GPG_ERR_NO_OBJ)
        ;
      else if (rc) 
        log_info ("DO `%s' not available: %s\n",
                  data_objects[i].desc, gpg_strerror (rc));
      else
        {
          if (data_objects[i].binary)
            {
              log_info ("DO `%s': ", data_objects[i].desc);
              log_printhex ("", buffer, buflen);
            }
          else
            log_info ("DO `%s': `%.*s'\n",
                      data_objects[i].desc,
                      (int)buflen, buffer); /* FIXME: sanitize */

          if (data_objects[i].constructed)
            {
              for (j=0; data_objects[j].tag; j++)
                {
                  const unsigned char *value;
                  size_t valuelen;
                  
                  if (j==i || data_objects[i].tag != data_objects[j].get_from)
                    continue;
                  value = find_tlv_unchecked (buffer, buflen,
                                              data_objects[j].tag, &valuelen);
                  if (!value)
                    ; /* not found */
                  else if (valuelen > buflen - (value - buffer))
                    log_error ("warning: constructed DO too short\n");
                  else
                    {
                      if (data_objects[j].binary)
                        {
                          log_info ("DO `%s': ", data_objects[j].desc);
                          log_printhex ("", value, valuelen);
                        }
                      else
                        log_info ("DO `%s': `%.*s'\n",
                                  data_objects[j].desc,
                                  (int)valuelen, value); /* FIXME: sanitize */
                    }
                }
            }
        }
      xfree (buffer); buffer = NULL;
    }
}


/* Count the number of bits, assuming the A represents an unsigned big
   integer of length LEN bytes. */
static unsigned int
count_bits (const unsigned char *a, size_t len)
{
  unsigned int n = len * 8;
  int i;

  for (; len && !*a; len--, a++, n -=8)
    ;
  if (len)
    {
      for (i=7; i && !(*a & (1<<i)); i--)
        n--;
    }
  return n;
}

/* GnuPG makes special use of the login-data DO, this fucntion parses
   the login data to store the flags for later use.  It may be called
   at any time and should be called after changing the login-data DO.

   Everything up to a LF is considered a mailbox or account name.  If
   the first LF is followed by DC4 (0x14) control sequence are
   expected up to the next LF.  Control sequences are separated by FS
   (0x28) and consist of key=value pairs.  There is one key defined:

    F=<flags>

    Were FLAGS is a plain hexadecimal number representing flag values.
    The lsb is here the rightmost bit.  Defined flags bits are:

      Bit 0 = CHV1 and CHV2 are not syncronized
      Bit 1 = CHV2 has been been set to the default PIN of "123456"
              (this implies that bit 0 is also set).

*/
static void
parse_login_data (app_t app)
{
  unsigned char *buffer, *p;
  size_t buflen, len;
  void *relptr;

  /* Set defaults.  */
  app->app_local->flags.no_sync = 0;
  app->app_local->flags.def_chv2 = 0;

  /* Read the DO.  */
  relptr = get_one_do (app, 0x005E, &buffer, &buflen, NULL);
  if (!relptr)
    return; /* Ooops. */
  for (; buflen; buflen--, buffer++)
    if (*buffer == '\n')
      break;
  if (buflen < 2 || buffer[1] != '\x14')
    return; /* No control sequences.  */
  buflen--;
  buffer++;
  do
    {
      buflen--;
      buffer++;
      if (buflen > 1 && *buffer == 'F' && buffer[1] == '=')
        {
          /* Flags control sequence found.  */
          int lastdig = 0;

          /* For now we are only interested in the last digit, so skip
             any leading digits but bail out on invalid characters. */
          for (p=buffer+2, len = buflen-2; len && hexdigitp (p); p++, len--)
            lastdig = xtoi_1 (p);
          if (len && !(*p == '\n' || *p == '\x18'))
            goto next;  /* Invalid characters in field.  */
          app->app_local->flags.no_sync = !!(lastdig & 1);
          app->app_local->flags.def_chv2 = (lastdig & 3) == 3;
        }
    next:
      for (; buflen && *buffer != '\x18'; buflen--, buffer++)
        if (*buffer == '\n')
          buflen = 1; 
    }
  while (buflen);

  xfree (relptr);
}

/* Note, that FPR must be at least 20 bytes. */
static gpg_error_t 
store_fpr (int slot, int keynumber, u32 timestamp,
           const unsigned char *m, size_t mlen,
           const unsigned char *e, size_t elen, 
           unsigned char *fpr, unsigned int card_version)
{
  unsigned int n, nbits;
  unsigned char *buffer, *p;
  int rc;
  
  for (; mlen && !*m; mlen--, m++) /* strip leading zeroes */
    ;
  for (; elen && !*e; elen--, e++) /* strip leading zeroes */
    ;

  n = 6 + 2 + mlen + 2 + elen;
  p = buffer = xtrymalloc (3 + n);
  if (!buffer)
    return gpg_error_from_errno (errno);
  
  *p++ = 0x99;     /* ctb */
  *p++ = n >> 8;   /* 2 byte length header */
  *p++ = n;
  *p++ = 4;        /* key packet version */
  *p++ = timestamp >> 24;
  *p++ = timestamp >> 16;
  *p++ = timestamp >>  8;
  *p++ = timestamp;
  *p++ = 1; /* RSA */
  nbits = count_bits (m, mlen);
  *p++ = nbits >> 8;
  *p++ = nbits;
  memcpy (p, m, mlen); p += mlen;
  nbits = count_bits (e, elen);
  *p++ = nbits >> 8;
  *p++ = nbits;
  memcpy (p, e, elen); p += elen;
    
  gcry_md_hash_buffer (GCRY_MD_SHA1, fpr, buffer, n+3);

  xfree (buffer);

  rc = iso7816_put_data (slot, (card_version > 0x0007? 0xC7 : 0xC6)
                               + keynumber, fpr, 20);
  if (rc)
    log_error (_("failed to store the fingerprint: %s\n"),gpg_strerror (rc));

  if (!rc && card_version > 0x0100)
    {
      unsigned char buf[4];

      buf[0] = timestamp >> 24;
      buf[1] = timestamp >> 16;
      buf[2] = timestamp >>  8;
      buf[3] = timestamp;

      rc = iso7816_put_data (slot, 0xCE + keynumber, buf, 4);
      if (rc)
        log_error (_("failed to store the creation date: %s\n"),
                   gpg_strerror (rc));
    }

  return rc;
}

       
static void
send_fpr_if_not_null (ctrl_t ctrl, const char *keyword,
                      int number, const unsigned char *fpr)
{                      
  int i;
  char buf[41];
  char numbuf[25];

  for (i=0; i < 20 && !fpr[i]; i++)
    ;
  if (i==20)
    return; /* All zero. */
  for (i=0; i< 20; i++)
    sprintf (buf+2*i, "%02X", fpr[i]);
  if (number == -1)
    *numbuf = 0; /* Don't print the key number */
  else
    sprintf (numbuf, "%d", number);
  send_status_info (ctrl, keyword,
                    numbuf, (size_t)strlen(numbuf),
                    buf, (size_t)strlen (buf), NULL, 0);
}

static void
send_fprtime_if_not_null (ctrl_t ctrl, const char *keyword,
                          int number, const unsigned char *stamp)
{                      
  char numbuf1[50], numbuf2[50];
  unsigned long value;

  value = (stamp[0] << 24) | (stamp[1]<<16) | (stamp[2]<<8) | stamp[3];
  if (!value)
    return;
  sprintf (numbuf1, "%d", number);
  sprintf (numbuf2, "%lu", value);
  send_status_info (ctrl, keyword,
                    numbuf1, (size_t)strlen(numbuf1),
                    numbuf2, (size_t)strlen(numbuf2), NULL, 0);
}

static void
send_key_data (ctrl_t ctrl, const char *name, 
               const unsigned char *a, size_t alen)
{
  char *p, *buf = xmalloc (alen*2+1);
  
  for (p=buf; alen; a++, alen--, p += 2)
    sprintf (p, "%02X", *a);

  send_status_info (ctrl, "KEY-DATA",
                    name, (size_t)strlen(name), 
                    buf, (size_t)strlen (buf),
                    NULL, 0);
  xfree (buf);
}

/* Implement the GETATTR command.  This is similar to the LEARN
   command but returns just one value via the status interface. */
static gpg_error_t 
do_getattr (app_t app, ctrl_t ctrl, const char *name)
{
  static struct {
    const char *name;
    int tag;
    int special;
  } table[] = {
    { "DISP-NAME",    0x005B },
    { "LOGIN-DATA",   0x005E },
    { "DISP-LANG",    0x5F2D },
    { "DISP-SEX",     0x5F35 },
    { "PUBKEY-URL",   0x5F50 },
    { "KEY-FPR",      0x00C5, 3 },
    { "KEY-TIME",     0x00CD, 4 },
    { "CA-FPR",       0x00C6, 3 },
    { "CHV-STATUS",   0x00C4, 1 }, 
    { "SIG-COUNTER",  0x0093, 2 },
    { "SERIALNO",     0x004F, -1 },
    { "AID",          0x004F },
    { "EXTCAP",       0x0000, -2 },
    { "PRIVATE-DO-1", 0x0101 },
    { "PRIVATE-DO-2", 0x0102 },
    { "PRIVATE-DO-3", 0x0103 },
    { "PRIVATE-DO-4", 0x0104 },
    { NULL, 0 }
  };
  int idx, i, rc;
  void *relptr;
  unsigned char *value;
  size_t valuelen;

  for (idx=0; table[idx].name && strcmp (table[idx].name, name); idx++)
    ;
  if (!table[idx].name)
    return gpg_error (GPG_ERR_INV_NAME); 
  
  if (table[idx].special == -1)
    {
      /* The serial number is very special.  We could have used the
         AID DO to retrieve it, but we have it already in the app
         context and the stamp argument is required anyway which we
         can't by other means. The AID DO is available anyway but not
         hex formatted. */
      char *serial;
      time_t stamp;
      char tmp[50];

      if (!app_get_serial_and_stamp (app, &serial, &stamp))
        {
          sprintf (tmp, "%lu", (unsigned long)stamp);
          send_status_info (ctrl, "SERIALNO",
                            serial, strlen (serial),
                            tmp, strlen (tmp),
                            NULL, 0);
          xfree (serial);
        }
      return 0;
    }
  if (table[idx].special == -2)
    {
      char tmp[50];

      sprintf (tmp, "gc=%d ki=%d fc=%d pd=%d", 
               app->app_local->extcap.get_challenge,
               app->app_local->extcap.key_import,
               app->app_local->extcap.change_force_chv,
               app->app_local->extcap.private_dos);
      send_status_info (ctrl, table[idx].name, tmp, strlen (tmp), NULL, 0);
      return 0;
    }

  relptr = get_one_do (app, table[idx].tag, &value, &valuelen, &rc);
  if (relptr)
    {
      if (table[idx].special == 1)
        {
          char numbuf[7*23];
          
          for (i=0,*numbuf=0; i < valuelen && i < 7; i++)
            sprintf (numbuf+strlen (numbuf), " %d", value[i]); 
          send_status_info (ctrl, table[idx].name,
                            numbuf, strlen (numbuf), NULL, 0);
        }
      else if (table[idx].special == 2)
        {
          char numbuf[50];

          sprintf (numbuf, "%lu", convert_sig_counter_value (value, valuelen));
          send_status_info (ctrl, table[idx].name,
                            numbuf, strlen (numbuf), NULL, 0);
        }
      else if (table[idx].special == 3)
        {
          if (valuelen >= 60)
            for (i=0; i < 3; i++)
              send_fpr_if_not_null (ctrl, table[idx].name, i+1, value+i*20);
        }
      else if (table[idx].special == 4)
        {
          if (valuelen >= 12)
            for (i=0; i < 3; i++)
              send_fprtime_if_not_null (ctrl, table[idx].name, i+1, value+i*4);
        }
      else
        send_status_info (ctrl, table[idx].name, value, valuelen, NULL, 0);

      xfree (relptr);
    }
  return rc;
}

/* Retrieve the fingerprint from the card inserted in SLOT and write
   the according hex representation to FPR.  Caller must have provide
   a buffer at FPR of least 41 bytes.  Returns 0 on success or an
   error code. */
#if GNUPG_MAJOR_VERSION > 1
static gpg_error_t
retrieve_fpr_from_card (app_t app, int keyno, char *fpr)
{
  gpg_error_t err = 0;
  void *relptr;
  unsigned char *value;
  size_t valuelen;
  int i;

  assert (keyno >=0 && keyno <= 2);

  relptr = get_one_do (app, 0x00C5, &value, &valuelen, NULL);
  if (relptr && valuelen >= 60)
    {
      for (i = 0; i < 20; i++)
        sprintf (fpr + (i * 2), "%02X", value[(keyno*20)+i]);
    }
  else
    err = gpg_error (GPG_ERR_NOT_FOUND);
  xfree (relptr);
  return err;
}
#endif /*GNUPG_MAJOR_VERSION > 1*/


/* Retrieve the public key material for the RSA key, whose fingerprint
   is FPR, from gpg output, which can be read through the stream FP.
   The RSA modulus will be stored at the address of M and MLEN, the
   public exponent at E and ELEN.  Returns zero on success, an error
   code on failure.  Caller must release the allocated buffers at M
   and E if the function returns success.  */
#if GNUPG_MAJOR_VERSION > 1
static gpg_error_t
retrieve_key_material (FILE *fp, const char *hexkeyid,
		       const unsigned char **m, size_t *mlen,
		       const unsigned char **e, size_t *elen)
{
  gcry_error_t err = 0;
  char *line = NULL;    /* read_line() buffer. */
  size_t line_size = 0; /* Helper for for read_line. */
  int found_key = 0;    /* Helper to find a matching key. */
  unsigned char *m_new = NULL;
  unsigned char *e_new = NULL;
  size_t m_new_n = 0;
  size_t e_new_n = 0;

  /* Loop over all records until we have found the subkey
     corresponsing to the fingerprint. Inm general the first record
     should be the pub record, but we don't rely on that.  Given that
     we only need to look at one key, it is sufficient to compare the
     keyid so that we don't need to look at "fpr" records. */
  for (;;)
    {
      char *p;
      char *fields[6];
      int nfields;
      size_t max_length;
      gcry_mpi_t mpi;
      int i;

      max_length = 4096;
      i = read_line (fp, &line, &line_size, &max_length);
      if (!i)
        break; /* EOF. */
      if (i < 0)
	{
	  err = gpg_error_from_errno (errno);
	  goto leave; /* Error. */
	}
      if (!max_length)
        {
          err = gpg_error (GPG_ERR_TRUNCATED);
          goto leave;  /* Line truncated - we better stop processing.  */
        }

      /* Parse the line into fields. */
      for (nfields=0, p=line; p && nfields < DIM (fields); nfields++)
        {
          fields[nfields] = p;
          p = strchr (p, ':');
          if (p)
            *(p++) = 0;
        }
      if (!nfields)
        continue; /* No fields at all - skip line.  */

      if (!found_key)
        {
          if ( (!strcmp (fields[0], "sub") || !strcmp (fields[0], "pub") )
               && nfields > 4 && !strcmp (fields[4], hexkeyid))
            found_key = 1;
          continue;
      	}
      
      if ( !strcmp (fields[0], "sub") || !strcmp (fields[0], "pub") )
        break; /* Next key - stop.  */

      if ( strcmp (fields[0], "pkd") )
        continue; /* Not a key data record.  */
      i = 0; /* Avoid erroneous compiler warning. */
      if ( nfields < 4 || (i = atoi (fields[1])) < 0 || i > 1
           || (!i && m_new) || (i && e_new))
        {
          err = gpg_error (GPG_ERR_GENERAL);
          goto leave; /* Error: Invalid key data record or not an RSA key.  */
        }
      
      err = gcry_mpi_scan (&mpi, GCRYMPI_FMT_HEX, fields[3], 0, NULL);
      if (err)
        mpi = NULL;
      else if (!i)
        err = gcry_mpi_aprint (GCRYMPI_FMT_STD, &m_new, &m_new_n, mpi);
      else
        err = gcry_mpi_aprint (GCRYMPI_FMT_STD, &e_new, &e_new_n, mpi);
      gcry_mpi_release (mpi);
      if (err)
        goto leave;
    }
  
  if (m_new && e_new)
    {
      *m = m_new;
      *mlen = m_new_n;
      m_new = NULL;
      *e = e_new;
      *elen = e_new_n;
      e_new = NULL;
    }
  else
    err = gpg_error (GPG_ERR_GENERAL);

 leave:
  xfree (m_new);
  xfree (e_new);
  xfree (line);
  return err;
}
#endif /*GNUPG_MAJOR_VERSION > 1*/


/* Get the public key for KEYNO and store it as an S-expresion with
   the APP handle.  On error that field gets cleared.  If we already
   know about the public key we will just return.  Note that this does
   not mean a key is available; this is soley indicated by the
   presence of the app->app_local->pk[KEYNO-1].key field.

   Note that GnuPG 1.x does not need this and it would be too time
   consuming to send it just for the fun of it. However, given that we
   use the same code in gpg 1.4, we can't use the gcry S-expresion
   here but need to open encode it. */
#if GNUPG_MAJOR_VERSION > 1
static gpg_error_t
get_public_key (app_t app, int keyno)
{
  gpg_error_t err = 0;
  unsigned char *buffer;
  const unsigned char *keydata, *m, *e;
  size_t buflen, keydatalen, mlen, elen;
  unsigned char *mbuf = NULL;
  unsigned char *ebuf = NULL;
  unsigned char *keybuf = NULL;
  unsigned char *keybuf_p;

  if (keyno < 1 || keyno > 3)
    return gpg_error (GPG_ERR_INV_ID);
  keyno--;

  /* Already cached? */
  if (app->app_local->pk[keyno].read_done)
    return 0;

  xfree (app->app_local->pk[keyno].key);
  app->app_local->pk[keyno].key = NULL;
  app->app_local->pk[keyno].keylen = 0;

  if (app->card_version > 0x0100)
    {
      /* We may simply read the public key out of these cards.  */
      err = iso7816_read_public_key (app->slot, 
                                    keyno == 0? "\xB6" :
                                    keyno == 1? "\xB8" : "\xA4",
                                    2,  
                                    &buffer, &buflen);
      if (err)
        {
          log_error (_("reading public key failed: %s\n"), gpg_strerror (err));
          goto leave;
        }

      keydata = find_tlv (buffer, buflen, 0x7F49, &keydatalen);
      if (!keydata)
        {
          err = gpg_error (GPG_ERR_CARD);
          log_error (_("response does not contain the public key data\n"));
          goto leave;
        }
 
      m = find_tlv (keydata, keydatalen, 0x0081, &mlen);
      if (!m)
        {
          err = gpg_error (GPG_ERR_CARD);
          log_error (_("response does not contain the RSA modulus\n"));
          goto leave;
        }
      

      e = find_tlv (keydata, keydatalen, 0x0082, &elen);
      if (!e)
        {
          err = gpg_error (GPG_ERR_CARD);
          log_error (_("response does not contain the RSA public exponent\n"));
          goto leave;
        }

      /* Prepend numbers with a 0 if needed.  */
      if (mlen && (*m & 0x80))
        {
          mbuf = xtrymalloc ( mlen + 1);
          if (!mbuf)
            {
              err = gpg_error_from_errno (errno);
              goto leave;
            }
          *mbuf = 0;
          memcpy (mbuf+1, m, mlen);
          mlen++;
          m = mbuf;
        }
      if (elen && (*e & 0x80))
        {
          ebuf = xtrymalloc ( elen + 1);
          if (!ebuf)
            {
              err = gpg_error_from_errno (errno);
              goto leave;
            }
          *ebuf = 0;
          memcpy (ebuf+1, e, elen);
          elen++;
          e = ebuf;
        }

    }
  else
    {
      /* Due to a design problem in v1.0 cards we can't get the public
         key out of these cards without doing a verify on CHV3.
         Clearly that is not an option and thus we try to locate the
         key using an external helper.

	 The helper we use here is gpg itself, which should know about
	 the key in any case.  */

      char fpr[41];
      char *hexkeyid;
      char *command = NULL;
      FILE *fp;
      int ret;

      buffer = NULL; /* We don't need buffer.  */

      err = retrieve_fpr_from_card (app, keyno, fpr);
      if (err)
	{
	  log_error ("error while retrieving fpr from card: %s\n",
		     gpg_strerror (err));
	  goto leave;
	}
      hexkeyid = fpr + 24;

      ret = asprintf (&command,
		      "gpg --list-keys --with-colons --with-key-data '%s'",
		      fpr);
      if (ret < 0)
	{
	  err = gpg_error_from_errno (errno);
	  goto leave;
	}

      fp = popen (command, "r");
      free (command);
      if (!fp)
	{
	  err = gpg_error_from_errno (errno);
	  log_error ("running gpg failed: %s\n", gpg_strerror (err));
	  goto leave;
	}

      err = retrieve_key_material (fp, hexkeyid, &m, &mlen, &e, &elen);
      fclose (fp);
      if (err)
	{
	  log_error ("error while retrieving key material through pipe: %s\n",
                     gpg_strerror (err));
	  goto leave;
	}
    }

  /* Allocate a buffer to construct the S-expression.  */
  /* FIXME: We should provide a generalized S-expression creation
     mechanism. */
  keybuf = xtrymalloc (50 + 2*35 + mlen + elen + 1);
  if (!keybuf)
    {
      err = gpg_error_from_errno (errno);
      goto leave;
    }
  
  sprintf (keybuf, "(10:public-key(3:rsa(1:n%u:", (unsigned int) mlen);
  keybuf_p = keybuf + strlen (keybuf);
  memcpy (keybuf_p, m, mlen);
  keybuf_p += mlen;
  sprintf (keybuf_p, ")(1:e%u:", (unsigned int)elen);
  keybuf_p += strlen (keybuf_p);
  memcpy (keybuf_p, e, elen);
  keybuf_p += elen;
  strcpy (keybuf_p, ")))");
  keybuf_p += strlen (keybuf_p);
  
  app->app_local->pk[keyno].key = keybuf;
  app->app_local->pk[keyno].keylen = (keybuf_p - keybuf);

 leave:
  /* Set a flag to indicate that we tried to read the key.  */
  app->app_local->pk[keyno].read_done = 1;

  xfree (buffer);
  xfree (mbuf);
  xfree (ebuf);
  return 0;
}
#endif /* GNUPG_MAJOR_VERSION > 1 */



/* Send the KEYPAIRINFO back. KEYNO needs to be in the range [1,3].
   This is used by the LEARN command. */
static gpg_error_t
send_keypair_info (app_t app, ctrl_t ctrl, int keyno)
{
  gpg_error_t err = 0;
  /* Note that GnuPG 1.x does not need this and it would be too time
     consuming to send it just for the fun of it. */
#if GNUPG_MAJOR_VERSION > 1
  unsigned char grip[20];
  char gripstr[41];
  char idbuf[50];
  int i;

  err = get_public_key (app, keyno);
  if (err)
    goto leave;
  
  assert (keyno >= 1 && keyno <= 3);
  if (!app->app_local->pk[keyno-1].key)
    goto leave; /* No such key - ignore. */

  err = keygrip_from_canon_sexp (app->app_local->pk[keyno-1].key,
                                 app->app_local->pk[keyno-1].keylen,
                                 grip);
  if (err)
    goto leave;
  
  for (i=0; i < 20; i++)
    sprintf (gripstr+i*2, "%02X", grip[i]);

  sprintf (idbuf, "OPENPGP.%d", keyno);
  send_status_info (ctrl, "KEYPAIRINFO", 
                    gripstr, 40, 
                    idbuf, strlen (idbuf), 
                    NULL, (size_t)0);

 leave:
#endif /* GNUPG_MAJOR_VERSION > 1 */

  return err; 
}


/* Handle the LEARN command for OpenPGP.  */
static gpg_error_t
do_learn_status (app_t app, ctrl_t ctrl)
{
  do_getattr (app, ctrl, "EXTCAP");
  do_getattr (app, ctrl, "DISP-NAME");
  do_getattr (app, ctrl, "DISP-LANG");
  do_getattr (app, ctrl, "DISP-SEX");
  do_getattr (app, ctrl, "PUBKEY-URL");
  do_getattr (app, ctrl, "LOGIN-DATA");
  do_getattr (app, ctrl, "KEY-FPR");
  if (app->card_version > 0x0100)
    do_getattr (app, ctrl, "KEY-TIME");
  do_getattr (app, ctrl, "CA-FPR");
  do_getattr (app, ctrl, "CHV-STATUS");
  do_getattr (app, ctrl, "SIG-COUNTER");
  if (app->app_local->extcap.private_dos)
    {
      do_getattr (app, ctrl, "PRIVATE-DO-1");
      do_getattr (app, ctrl, "PRIVATE-DO-2");
      if (app->did_chv2)
        do_getattr (app, ctrl, "PRIVATE-DO-3");
      if (app->did_chv3)
        do_getattr (app, ctrl, "PRIVATE-DO-4");
    }
  send_keypair_info (app, ctrl, 1);
  send_keypair_info (app, ctrl, 2);
  send_keypair_info (app, ctrl, 3);
  return 0;
}


/* Handle the READKEY command for OpenPGP.  On success a canonical
   encoded S-expression with the public key will get stored at PK and
   its length (for assertions) at PKLEN; the caller must release that
   buffer. On error PK and PKLEN are not changed and an error code is
   returned.  */
static gpg_error_t
do_readkey (app_t app, const char *keyid, unsigned char **pk, size_t *pklen)
{
#if GNUPG_MAJOR_VERSION > 1
  gpg_error_t err;
  int keyno;
  unsigned char *buf;

  if (!strcmp (keyid, "OPENPGP.1"))
    keyno = 1;
  else if (!strcmp (keyid, "OPENPGP.2"))
    keyno = 2;
  else if (!strcmp (keyid, "OPENPGP.3"))
    keyno = 3;
  else
    return gpg_error (GPG_ERR_INV_ID);

  err = get_public_key (app, keyno);
  if (err)
    return err;

  buf = app->app_local->pk[keyno-1].key;
  if (!buf)
    return gpg_error (GPG_ERR_NO_PUBKEY);
  *pk = buf;
  *pklen = app->app_local->pk[keyno-1].keylen;;
  return 0;
#else
  return gpg_error (GPG_ERR_NOT_IMPLEMENTED);
#endif
}



/* Verify CHV2 if required.  Depending on the configuration of the
   card CHV1 will also be verified. */
static gpg_error_t
verify_chv2 (app_t app,
             gpg_error_t (*pincb)(void*, const char *, char **),
             void *pincb_arg)
{
  int rc = 0;

  if (!app->did_chv2) 
    {
      char *pinvalue;

      rc = pincb (pincb_arg, "PIN", &pinvalue); 
      if (rc)
        {
          log_info (_("PIN callback returned error: %s\n"), gpg_strerror (rc));
          return rc;
        }

      if (strlen (pinvalue) < 6)
        {
          log_error (_("PIN for CHV%d is too short;"
                       " minimum length is %d\n"), 2, 6);
          xfree (pinvalue);
          return gpg_error (GPG_ERR_BAD_PIN);
        }

      rc = iso7816_verify (app->slot, 0x82, pinvalue, strlen (pinvalue));
      if (rc)
        {
          log_error (_("verify CHV%d failed: %s\n"), 2, gpg_strerror (rc));
          xfree (pinvalue);
          flush_cache_after_error (app);
          return rc;
        }
      app->did_chv2 = 1;

      if (!app->did_chv1 && !app->force_chv1)
        {
          rc = iso7816_verify (app->slot, 0x81, pinvalue, strlen (pinvalue));
          if (gpg_err_code (rc) == GPG_ERR_BAD_PIN)
            rc = gpg_error (GPG_ERR_PIN_NOT_SYNCED);
          if (rc)
            {
              log_error (_("verify CHV%d failed: %s\n"), 1, gpg_strerror (rc));
              xfree (pinvalue);
              flush_cache_after_error (app);
              return rc;
            }
          app->did_chv1 = 1;
        }
      xfree (pinvalue);
    }
  return rc;
}

/* Verify CHV3 if required. */
static gpg_error_t
verify_chv3 (app_t app,
             gpg_error_t (*pincb)(void*, const char *, char **),
             void *pincb_arg)
{
  int rc = 0;

#if GNUPG_MAJOR_VERSION != 1
  if (!opt.allow_admin)
    {
      log_info (_("access to admin commands is not configured\n"));
      return gpg_error (GPG_ERR_EACCES);
    }
#endif
      
  if (!app->did_chv3) 
    {
      char *pinvalue;
      void *relptr;
      unsigned char *value;
      size_t valuelen;

      relptr = get_one_do (app, 0x00C4, &value, &valuelen, NULL);
      if (!relptr || valuelen < 7)
        {
          log_error (_("error retrieving CHV status from card\n"));
          xfree (relptr);
          return gpg_error (GPG_ERR_CARD);
        }
      if (value[6] == 0)
        {
          log_info (_("card is permanently locked!\n"));
          xfree (relptr);
          return gpg_error (GPG_ERR_BAD_PIN);
        }

      log_info(_("%d Admin PIN attempts remaining before card"
                 " is permanently locked\n"), value[6]);
      xfree (relptr);

      /* TRANSLATORS: Do not translate the "|A|" prefix but
         keep it at the start of the string.  We need this elsewhere
         to get some infos on the string. */
      rc = pincb (pincb_arg, _("|A|Admin PIN"), &pinvalue); 
      if (rc)
        {
          log_info (_("PIN callback returned error: %s\n"), gpg_strerror (rc));
          return rc;
        }

      if (strlen (pinvalue) < 8)
        {
          log_error (_("PIN for CHV%d is too short;"
                       " minimum length is %d\n"), 3, 8);
          xfree (pinvalue);
          return gpg_error (GPG_ERR_BAD_PIN);
        }

      rc = iso7816_verify (app->slot, 0x83, pinvalue, strlen (pinvalue));
      xfree (pinvalue);
      if (rc)
        {
          log_error (_("verify CHV%d failed: %s\n"), 3, gpg_strerror (rc));
          flush_cache_after_error (app);
          return rc;
        }
      app->did_chv3 = 1;
    }
  return rc;
}


/* Handle the SETATTR operation. All arguments are already basically
   checked. */
static gpg_error_t 
do_setattr (app_t app, const char *name,
            gpg_error_t (*pincb)(void*, const char *, char **),
            void *pincb_arg,
            const unsigned char *value, size_t valuelen)
{
  gpg_error_t rc;
  int idx;
  static struct {
    const char *name;
    int tag;
    int need_chv;
    int special;
  } table[] = {
    { "DISP-NAME",    0x005B, 3 },
    { "LOGIN-DATA",   0x005E, 3, 2 },
    { "DISP-LANG",    0x5F2D, 3 },
    { "DISP-SEX",     0x5F35, 3 },
    { "PUBKEY-URL",   0x5F50, 3 },
    { "CHV-STATUS-1", 0x00C4, 3, 1 },
    { "CA-FPR-1",     0x00CA, 3 },
    { "CA-FPR-2",     0x00CB, 3 },
    { "CA-FPR-3",     0x00CC, 3 },
    { "PRIVATE-DO-1", 0x0101, 2 },
    { "PRIVATE-DO-2", 0x0102, 3 },
    { "PRIVATE-DO-3", 0x0103, 2 },
    { "PRIVATE-DO-4", 0x0104, 3 },
    { NULL, 0 }
  };


  for (idx=0; table[idx].name && strcmp (table[idx].name, name); idx++)
    ;
  if (!table[idx].name)
    return gpg_error (GPG_ERR_INV_NAME); 

  switch (table[idx].need_chv)
    {
    case 2:
      rc = verify_chv2 (app, pincb, pincb_arg);
      break;
    case 3:
      rc = verify_chv3 (app, pincb, pincb_arg);
      break;
    default:
      rc = 0;
    }
  if (rc)
    return rc;

  /* Flush the cache before writing it, so that the next get operation
     will reread the data from the card and thus get synced in case of
     errors (e.g. data truncated by the card). */
  flush_cache_item (app, table[idx].tag);
  rc = iso7816_put_data (app->slot, table[idx].tag, value, valuelen);
  if (rc)
    log_error ("failed to set `%s': %s\n", table[idx].name, gpg_strerror (rc));

  if (table[idx].special == 1)
    app->force_chv1 = (valuelen && *value == 0);
  else if (table[idx].special == 2)
    parse_login_data (app);

  return rc;
}


/* Handle the PASSWD command. */
static gpg_error_t 
do_change_pin (app_t app, ctrl_t ctrl,  const char *chvnostr, int reset_mode,
               gpg_error_t (*pincb)(void*, const char *, char **),
               void *pincb_arg)
{
  int rc = 0;
  int chvno = atoi (chvnostr);
  char *pinvalue;

  if (reset_mode && chvno == 3)
    {
      rc = gpg_error (GPG_ERR_INV_ID);
      goto leave;
    }
  else if (reset_mode || chvno == 3)
    {
      /* we always require that the PIN is entered. */
      app->did_chv3 = 0;
      rc = verify_chv3 (app, pincb, pincb_arg);
      if (rc)
        goto leave;
    }
  else if (chvno == 1 || chvno == 2)
    {
      /* CHV1 and CVH2 should always have the same value, thus we
         enforce it here.  */
      int save_force = app->force_chv1;

      app->force_chv1 = 0;
      app->did_chv1 = 0;
      app->did_chv2 = 0;
      rc = verify_chv2 (app, pincb, pincb_arg);
      app->force_chv1 = save_force;
      if (rc)
        goto leave;
    }
  else
    {
      rc = gpg_error (GPG_ERR_INV_ID);
      goto leave;
    }

  if (chvno == 3)
    app->did_chv3 = 0;
  else
    app->did_chv1 = app->did_chv2 = 0;

  /* TRANSLATORS: Do not translate the "|*|" prefixes but
     keep it at the start of the string.  We need this elsewhere
     to get some infos on the string. */
  rc = pincb (pincb_arg, chvno == 3? _("|AN|New Admin PIN") : _("|N|New PIN"), 
              &pinvalue); 
  if (rc)
    {
      log_error (_("error getting new PIN: %s\n"), gpg_strerror (rc));
      goto leave;
    }

  if (reset_mode)
    {
      rc = iso7816_reset_retry_counter (app->slot, 0x81,
                                        pinvalue, strlen (pinvalue));
      if (!rc)
        rc = iso7816_reset_retry_counter (app->slot, 0x82,
                                          pinvalue, strlen (pinvalue));
    }
  else
    {
      if (chvno == 1 || chvno == 2)
        {
          rc = iso7816_change_reference_data (app->slot, 0x81, NULL, 0,
                                              pinvalue, strlen (pinvalue));
          if (!rc)
            rc = iso7816_change_reference_data (app->slot, 0x82, NULL, 0,
                                                pinvalue, strlen (pinvalue));
        }
      else
        rc = iso7816_change_reference_data (app->slot, 0x80 + chvno, NULL, 0,
                                            pinvalue, strlen (pinvalue));
    }
  xfree (pinvalue);
  if (rc)
    flush_cache_after_error (app);

 leave:
  return rc;
}


/* Check whether a key already exists.  KEYIDX is the index of the key
   (0..2).  If FORCE is TRUE a diagnositic will be printed but no
   error returned if the key already exists. */
static gpg_error_t
does_key_exist (app_t app, int keyidx, int force)
{
  const unsigned char *fpr;
  unsigned char *buffer;
  size_t buflen, n;
  int i;

  assert (keyidx >=0 && keyidx <= 2);

  if (iso7816_get_data (app->slot, 0x006E, &buffer, &buflen))
    {
      log_error (_("error reading application data\n"));
      return gpg_error (GPG_ERR_GENERAL);
    }
  fpr = find_tlv (buffer, buflen, 0x00C5, &n);
  if (!fpr || n < 60)
    {
      log_error (_("error reading fingerprint DO\n"));
      xfree (buffer);
      return gpg_error (GPG_ERR_GENERAL);
    }
  fpr += 20*keyidx;
  for (i=0; i < 20 && !fpr[i]; i++)
    ;
  xfree (buffer);
  if (i!=20 && !force)
    {
      log_error (_("key already exists\n"));
      return gpg_error (GPG_ERR_EEXIST);
    }
  else if (i!=20)
    log_info (_("existing key will be replaced\n"));
  else
    log_info (_("generating new key\n"));
  return 0;
}



/* Handle the WRITEKEY command for OpenPGP.  This function expects a
   canonical encoded S-expression with the secret key in KEYDATA and
   its length (for assertions) in KEYDATALEN.  KEYID needs to be the
   usual keyid which for OpenPGP is the string "OPENPGP.n" with
   n=1,2,3.  Bit 0 of FLAGS indicates whether an existing key shall
   get overwritten.  PINCB and PINCB_ARG are the usual arguments for
   the pinentry callback.  */
static gpg_error_t
do_writekey (app_t app, ctrl_t ctrl,
             const char *keyid, unsigned int flags,
             gpg_error_t (*pincb)(void*, const char *, char **),
             void *pincb_arg,
             const unsigned char *keydata, size_t keydatalen)
{
  gpg_error_t err;
  int force = (flags & 1);
  int keyno;
  const unsigned char *buf, *tok;
  size_t buflen, toklen;
  int depth, last_depth1, last_depth2;
  const unsigned char *rsa_n = NULL;
  const unsigned char *rsa_e = NULL;
  const unsigned char *rsa_p = NULL;
  const unsigned char *rsa_q = NULL;
  size_t rsa_n_len, rsa_e_len, rsa_p_len, rsa_q_len;
  unsigned int nbits;
  unsigned char *template = NULL;
  unsigned char *tp;
  size_t template_len;
  unsigned char fprbuf[20];
  u32 created_at = 0;

  if (!strcmp (keyid, "OPENPGP.1"))
    keyno = 0;
  else if (!strcmp (keyid, "OPENPGP.2"))
    keyno = 1;
  else if (!strcmp (keyid, "OPENPGP.3"))
    keyno = 2;
  else
    return gpg_error (GPG_ERR_INV_ID);
  
  err = does_key_exist (app, keyno, force);
  if (err)
    return err;


  /* 
     Parse the S-expression
   */
  buf = keydata;
  buflen = keydatalen;
  depth = 0;
  if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
    goto leave;
  if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
    goto leave;
  if (!tok || toklen != 11 || memcmp ("private-key", tok, toklen))
    {
      if (!tok)
        ;
      else if (toklen == 21 && !memcmp ("protected-private-key", tok, toklen))
        log_info ("protected-private-key passed to writekey\n");
      else if (toklen == 20 && !memcmp ("shadowed-private-key", tok, toklen))
        log_info ("shadowed-private-key passed to writekey\n");
      err = gpg_error (GPG_ERR_BAD_SECKEY);
      goto leave;
    }
  if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
    goto leave;
  if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
    goto leave;
  if (!tok || toklen != 3 || memcmp ("rsa", tok, toklen))
    {
      err = gpg_error (GPG_ERR_WRONG_PUBKEY_ALGO);
      goto leave;
    }
  last_depth1 = depth;
  while (!(err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen))
         && depth && depth >= last_depth1)
    {
      if (tok)
        {
          err = gpg_error (GPG_ERR_UNKNOWN_SEXP);
          goto leave;
        }
      if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
        goto leave;
      if (tok && toklen == 1)
        {
          const unsigned char **mpi;
          size_t *mpi_len;

          switch (*tok)
            {
            case 'n': mpi = &rsa_n; mpi_len = &rsa_n_len; break; 
            case 'e': mpi = &rsa_e; mpi_len = &rsa_e_len; break; 
            case 'p': mpi = &rsa_p; mpi_len = &rsa_p_len; break; 
            case 'q': mpi = &rsa_q; mpi_len = &rsa_q_len;break; 
            default: mpi = NULL;  mpi_len = NULL; break;
            }
          if (mpi && *mpi)
            {
              err = gpg_error (GPG_ERR_DUP_VALUE);
              goto leave;
            }
          if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
            goto leave;
          if (tok && mpi)
            {
              /* Strip off leading zero bytes and save. */
              for (;toklen && !*tok; toklen--, tok++)
                ;
              *mpi = tok;
              *mpi_len = toklen;
            }
        }
      /* Skip until end of list. */
      last_depth2 = depth;
      while (!(err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen))
             && depth && depth >= last_depth2)
        ;
      if (err)
        goto leave;
    }
  /* Parse other attributes. */
  last_depth1 = depth;
  while (!(err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen))
         && depth && depth >= last_depth1)
    {
      if (tok)
        {
          err = gpg_error (GPG_ERR_UNKNOWN_SEXP);
          goto leave;
        }
      if ((err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen)))
        goto leave;
      if (tok && toklen == 10 && !memcmp ("created-at", tok, toklen))
        {
          if ((err = parse_sexp (&buf,&buflen,&depth,&tok,&toklen)))
            goto leave;
          if (tok)
            {
              for (created_at=0; toklen && *tok && *tok >= '0' && *tok <= '9';
                   tok++, toklen--)
                created_at = created_at*10 + (*tok - '0');
            }
        }
      /* Skip until end of list. */
      last_depth2 = depth;
      while (!(err = parse_sexp (&buf, &buflen, &depth, &tok, &toklen))
             && depth && depth >= last_depth2)
        ;
      if (err)
        goto leave;
    }


  /* Check that we have all parameters and that they match the card
     description. */
  if (!created_at)
    {
      log_error (_("creation timestamp missing\n"));
      err = gpg_error (GPG_ERR_INV_VALUE);
      goto leave;
    }
  nbits = rsa_n? count_bits (rsa_n, rsa_n_len) : 0;
  if (nbits != 1024)
    {
      log_error (_("RSA modulus missing or not of size %d bits\n"), 1024);
      err = gpg_error (GPG_ERR_BAD_SECKEY);
      goto leave;
    }
  nbits = rsa_e? count_bits (rsa_e, rsa_e_len) : 0;
  if (nbits < 2 || nbits > 32)
    {
      log_error (_("RSA public exponent missing or larger than %d bits\n"),
                 32);
      err = gpg_error (GPG_ERR_BAD_SECKEY);
      goto leave;
    }
  nbits = rsa_p? count_bits (rsa_p, rsa_p_len) : 0;
  if (nbits != 512)
    {
      log_error (_("RSA prime %s missing or not of size %d bits\n"), "P", 512);
      err = gpg_error (GPG_ERR_BAD_SECKEY);
      goto leave;
    }
  nbits = rsa_q? count_bits (rsa_q, rsa_q_len) : 0;
  if (nbits != 512)
    {
      log_error (_("RSA prime %s missing or not of size %d bits\n"), "Q", 512);
      err = gpg_error (GPG_ERR_BAD_SECKEY);
      goto leave;
    }
  

  /* Build the private key template as described in section 4.3.3.6 of
     the OpenPGP card specs:
         0xC0   <length> public exponent
         0xC1   <length> prime p 
         0xC2   <length> prime q 
  */
  assert (rsa_e_len <= 4);
  template_len = (1 + 1 + 4
                  + 1 + 1 + rsa_p_len
                  + 1 + 1 + rsa_q_len);
  template = tp = xtrymalloc_secure (template_len);
  if (!template)
    {
      err = gpg_error_from_errno (errno);
      goto leave;
    }
  *tp++ = 0xC0;
  *tp++ = 4;
  memcpy (tp, rsa_e, rsa_e_len);
  if (rsa_e_len < 4)
    {
      /* Right justify E. */
      memmove (tp+4-rsa_e_len, tp, rsa_e_len);
      memset (tp, 0, 4-rsa_e_len);
    }                 
  tp += 4;

  *tp++ = 0xC1;
  *tp++ = rsa_p_len;
  memcpy (tp, rsa_p, rsa_p_len);
  tp += rsa_p_len;

  *tp++ = 0xC2;
  *tp++ = rsa_q_len;
  memcpy (tp, rsa_q, rsa_q_len);
  tp += rsa_q_len;

  assert (tp - template == template_len);


  /* Obviously we need to remove the cached public key.  */
  xfree (app->app_local->pk[keyno].key);
  app->app_local->pk[keyno].key = NULL;
  app->app_local->pk[keyno].keylen = 0;
  app->app_local->pk[keyno].read_done = 0;

  /* Prepare for storing the key.  */
  err = verify_chv3 (app, pincb, pincb_arg);
  if (err)
    goto leave;

  /* Store the key. */
  err = iso7816_put_data (app->slot,
                         (app->card_version > 0x0007? 0xE0 : 0xE9) + keyno,
                         template, template_len);
  if (err)
    {
      log_error (_("failed to store the key: %s\n"), gpg_strerror (err));
      goto leave;
    }
 
  err = store_fpr (app->slot, keyno, created_at,
                  rsa_n, rsa_n_len, rsa_e, rsa_e_len,
                  fprbuf, app->card_version);
  if (err)
    goto leave;


 leave:
  xfree (template);
  return err;
}


/* Handle the GENKEY command. */
static gpg_error_t 
do_genkey (app_t app, ctrl_t ctrl,  const char *keynostr, unsigned int flags,
          gpg_error_t (*pincb)(void*, const char *, char **),
          void *pincb_arg)
{
  int rc;
  char numbuf[30];
  unsigned char fprbuf[20];
  const unsigned char *keydata, *m, *e;
  unsigned char *buffer = NULL;
  size_t buflen, keydatalen, mlen, elen;
  time_t created_at;
  int keyno = atoi (keynostr);
  int force = (flags & 1);
  time_t start_at;

  if (keyno < 1 || keyno > 3)
    return gpg_error (GPG_ERR_INV_ID);
  keyno--;

  /* We flush the cache to increase the traffic before a key
     generation.  This _might_ help a card to gather more entropy. */
  flush_cache (app);

  /* Obviously we need to remove the cached public key.  */
  xfree (app->app_local->pk[keyno].key);
  app->app_local->pk[keyno].key = NULL;
  app->app_local->pk[keyno].keylen = 0;
  app->app_local->pk[keyno].read_done = 0;

  /* Check whether a key already exists.  */
  rc = does_key_exist (app, keyno, force);
  if (rc)
    return rc;

  /* Prepare for key generation by verifying the ADmin PIN.  */
  rc = verify_chv3 (app, pincb, pincb_arg);
  if (rc)
    goto leave;
   
#if 1
  log_info (_("please wait while key is being generated ...\n"));
  start_at = time (NULL);
  rc = iso7816_generate_keypair 
#else
#warning key generation temporary replaced by reading an existing key.
  rc = iso7816_read_public_key
#endif
                              (app->slot, 
                                 keyno == 0? "\xB6" :
                                 keyno == 1? "\xB8" : "\xA4",
                                 2,
                                 &buffer, &buflen);
  if (rc)
    {
      rc = gpg_error (GPG_ERR_CARD);
      log_error (_("generating key failed\n"));
      goto leave;
    }
  log_info (_("key generation completed (%d seconds)\n"),
            (int)(time (NULL) - start_at));
  keydata = find_tlv (buffer, buflen, 0x7F49, &keydatalen);
  if (!keydata)
    {
      rc = gpg_error (GPG_ERR_CARD);
      log_error (_("response does not contain the public key data\n"));
      goto leave;
    }
 
  m = find_tlv (keydata, keydatalen, 0x0081, &mlen);
  if (!m)
    {
      rc = gpg_error (GPG_ERR_CARD);
      log_error (_("response does not contain the RSA modulus\n"));
      goto leave;
    }
/*    log_printhex ("RSA n:", m, mlen); */
  send_key_data (ctrl, "n", m, mlen);

  e = find_tlv (keydata, keydatalen, 0x0082, &elen);
  if (!e)
    {
      rc = gpg_error (GPG_ERR_CARD);
      log_error (_("response does not contain the RSA public exponent\n"));
      goto leave;
    }
/*    log_printhex ("RSA e:", e, elen); */
  send_key_data (ctrl, "e", e, elen);

  created_at = gnupg_get_time ();
  sprintf (numbuf, "%lu", (unsigned long)created_at);
  send_status_info (ctrl, "KEY-CREATED-AT",
                    numbuf, (size_t)strlen(numbuf), NULL, 0);

  rc = store_fpr (app->slot, keyno, (u32)created_at,
                  m, mlen, e, elen, fprbuf, app->card_version);
  if (rc)
    goto leave;
  send_fpr_if_not_null (ctrl, "KEY-FPR", -1, fprbuf);


 leave:
  xfree (buffer);
  return rc;
}


static unsigned long
convert_sig_counter_value (const unsigned char *value, size_t valuelen)
{
  unsigned long ul;

  if (valuelen == 3 )
    ul = (value[0] << 16) | (value[1] << 8) | value[2];
  else
    {
      log_error (_("invalid structure of OpenPGP card (DO 0x93)\n"));
      ul = 0;
    }
  return ul;
}

static unsigned long
get_sig_counter (app_t app)
{
  void *relptr;
  unsigned char *value;
  size_t valuelen;
  unsigned long ul;

  relptr = get_one_do (app, 0x0093, &value, &valuelen, NULL);
  if (!relptr)
    return 0;
  ul = convert_sig_counter_value (value, valuelen);
  xfree (relptr);
  return ul;
}

static gpg_error_t
compare_fingerprint (app_t app, int keyno, unsigned char *sha1fpr)
{
  const unsigned char *fpr;
  unsigned char *buffer;
  size_t buflen, n;
  int rc, i;
  
  assert (keyno >= 1 && keyno <= 3);

  rc = get_cached_data (app, 0x006E, &buffer, &buflen, 0);
  if (rc)
    {
      log_error (_("error reading application data\n"));
      return gpg_error (GPG_ERR_GENERAL);
    }
  fpr = find_tlv (buffer, buflen, 0x00C5, &n);
  if (!fpr || n != 60)
    {
      xfree (buffer);
      log_error (_("error reading fingerprint DO\n"));
      return gpg_error (GPG_ERR_GENERAL);
    }
  fpr += (keyno-1)*20;
  for (i=0; i < 20; i++)
    if (sha1fpr[i] != fpr[i])
      {
        xfree (buffer);
        return gpg_error (GPG_ERR_WRONG_SECKEY);
      }
  xfree (buffer);
  return 0;
}


  /* If a fingerprint has been specified check it against the one on
     the card.  This is allows for a meaningful error message in case
     the key on the card has been replaced but the shadow information
     known to gpg was not updated.  If there is no fingerprint we
     assume that this is okay. */
static gpg_error_t
check_against_given_fingerprint (app_t app, const char *fpr, int keyno)
{
  unsigned char tmp[20];
  const char *s;
  int n;

  for (s=fpr, n=0; hexdigitp (s); s++, n++)
    ;
  if (n != 40)
    return gpg_error (GPG_ERR_INV_ID);
  else if (!*s)
    ; /* okay */
  else
    return gpg_error (GPG_ERR_INV_ID);

  for (s=fpr, n=0; n < 20; s += 2, n++)
        tmp[n] = xtoi_2 (s);
  return compare_fingerprint (app, keyno, tmp);
}



/* Compute a digital signature on INDATA which is expected to be the
   raw message digest. For this application the KEYIDSTR consists of
   the serialnumber and the fingerprint delimited by a slash.

   Note that this fucntion may return the error code
   GPG_ERR_WRONG_CARD to indicate that the card currently present does
   not match the one required for the requested action (e.g. the
   serial number does not match). */
static gpg_error_t 
do_sign (app_t app, const char *keyidstr, int hashalgo,
         gpg_error_t (*pincb)(void*, const char *, char **),
         void *pincb_arg,
         const void *indata, size_t indatalen,
         unsigned char **outdata, size_t *outdatalen )
{
  static unsigned char sha1_prefix[15] = /* Object ID is 1.3.14.3.2.26 */
  { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03,
    0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };
  static unsigned char rmd160_prefix[15] = /* Object ID is 1.3.36.3.2.1 */
  { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x24, 0x03,
    0x02, 0x01, 0x05, 0x00, 0x04, 0x14 };
  int rc;
  unsigned char data[35];
  unsigned char tmp_sn[20]; /* actually 16 but we use it also for the fpr. */
  const char *s;
  int n;
  const char *fpr = NULL;
  unsigned long sigcount;

  if (!keyidstr || !*keyidstr)
    return gpg_error (GPG_ERR_INV_VALUE);
  if (indatalen == 20)
    ;
  else if (indatalen == (15 + 20) && hashalgo == GCRY_MD_SHA1
           && !memcmp (indata, sha1_prefix, 15))
    ;
  else if (indatalen == (15 + 20) && hashalgo == GCRY_MD_RMD160
           && !memcmp (indata, rmd160_prefix, 15))
    ;
  else
    {
      log_error(_("card does not support digest algorithm %s\n"),
		digest_algo_to_string(hashalgo));
      return gpg_error (GPG_ERR_INV_VALUE);
    }

  /* Check whether an OpenPGP card of any version has been requested. */
  if (strlen (keyidstr) < 32 || strncmp (keyidstr, "D27600012401", 12))
    return gpg_error (GPG_ERR_INV_ID);
  
  for (s=keyidstr, n=0; hexdigitp (s); s++, n++)
    ;
  if (n != 32)
    return gpg_error (GPG_ERR_INV_ID);
  else if (!*s)
    ; /* no fingerprint given: we allow this for now. */
  else if (*s == '/')
    fpr = s + 1; 
  else
    return gpg_error (GPG_ERR_INV_ID);

  for (s=keyidstr, n=0; n < 16; s += 2, n++)
    tmp_sn[n] = xtoi_2 (s);

  if (app->serialnolen != 16)
    return gpg_error (GPG_ERR_INV_CARD);
  if (memcmp (app->serialno, tmp_sn, 16))
    return gpg_error (GPG_ERR_WRONG_CARD);

  /* If a fingerprint has been specified check it against the one on
     the card.  This is allows for a meaningful error message in case
     the key on the card has been replaced but the shadow information
     known to gpg was not updated.  If there is no fingerprint, gpg
     will detect a bogus signature anyway due to the
     verify-after-signing feature. */
  rc = fpr? check_against_given_fingerprint (app, fpr, 1) : 0;
  if (rc)
    return rc;

  if (hashalgo == GCRY_MD_SHA1)
    memcpy (data, sha1_prefix, 15);
  else if (hashalgo == GCRY_MD_RMD160)
    memcpy (data, rmd160_prefix, 15);
  else 
    return gpg_error (GPG_ERR_UNSUPPORTED_ALGORITHM);
  memcpy (data+15, indata, indatalen);

  sigcount = get_sig_counter (app);
  log_info (_("signatures created so far: %lu\n"), sigcount);

  if (!app->did_chv1 || app->force_chv1 ) 
    {
      char *pinvalue;

      {
        char *prompt;
#define PROMPTSTRING  _("||Please enter the PIN%%0A[sigs done: %lu]")

        prompt = malloc (strlen (PROMPTSTRING) + 50);
        if (!prompt)
          return gpg_error_from_errno (errno);
        sprintf (prompt, PROMPTSTRING, sigcount);
        rc = pincb (pincb_arg, prompt, &pinvalue); 
        free (prompt);
#undef PROMPTSTRING
      }
      if (rc)
        {
          log_info (_("PIN callback returned error: %s\n"), gpg_strerror (rc));
          return rc;
        }

      if (strlen (pinvalue) < 6)
        {
          log_error (_("PIN for CHV%d is too short;"
                       " minimum length is %d\n"), 1, 6);
          xfree (pinvalue);
          return gpg_error (GPG_ERR_BAD_PIN);
        }

      rc = iso7816_verify (app->slot, 0x81, pinvalue, strlen (pinvalue));
      if (rc)
        {
          log_error (_("verify CHV%d failed: %s\n"), 1, gpg_strerror (rc));
          xfree (pinvalue);
          flush_cache_after_error (app);
          return rc;
        }
      app->did_chv1 = 1;
      if (!app->did_chv2)
        {
          /* We should also verify CHV2. */
          rc = iso7816_verify (app->slot, 0x82, pinvalue, strlen (pinvalue));
          if (gpg_err_code (rc) == GPG_ERR_BAD_PIN)
            rc = gpg_error (GPG_ERR_PIN_NOT_SYNCED);
          if (rc)
            {
              log_error (_("verify CHV%d failed: %s\n"), 2, gpg_strerror (rc));
              xfree (pinvalue);
              flush_cache_after_error (app);
              return rc;
            }
          app->did_chv2 = 1;
        }
      xfree (pinvalue);
    }

  rc = iso7816_compute_ds (app->slot, data, 35, outdata, outdatalen);
  return rc;
}

/* Compute a digital signature using the INTERNAL AUTHENTICATE command
   on INDATA which is expected to be the raw message digest. For this
   application the KEYIDSTR consists of the serialnumber and the
   fingerprint delimited by a slash.  Optionally the id OPENPGP.3 may
   be given.

   Note that this fucntion may return the error code
   GPG_ERR_WRONG_CARD to indicate that the card currently present does
   not match the one required for the requested action (e.g. the
   serial number does not match). */
static gpg_error_t 
do_auth (app_t app, const char *keyidstr,
         gpg_error_t (*pincb)(void*, const char *, char **),
         void *pincb_arg,
         const void *indata, size_t indatalen,
         unsigned char **outdata, size_t *outdatalen )
{
  int rc;
  unsigned char tmp_sn[20]; /* actually 16 but we use it also for the fpr. */
  const char *s;
  int n;
  const char *fpr = NULL;

  if (!keyidstr || !*keyidstr)
    return gpg_error (GPG_ERR_INV_VALUE);
  if (indatalen > 50) /* For a 1024 bit key. */
    return gpg_error (GPG_ERR_INV_VALUE);

  /* Check whether an OpenPGP card of any version has been requested. */
  if (!strcmp (keyidstr, "OPENPGP.3"))
    ;
  else if (strlen (keyidstr) < 32 || strncmp (keyidstr, "D27600012401", 12))
    return gpg_error (GPG_ERR_INV_ID);
  else
    {
      for (s=keyidstr, n=0; hexdigitp (s); s++, n++)
        ;
      if (n != 32)
        return gpg_error (GPG_ERR_INV_ID);
      else if (!*s)
        ; /* no fingerprint given: we allow this for now. */
      else if (*s == '/')
        fpr = s + 1; 
      else
        return gpg_error (GPG_ERR_INV_ID);

      for (s=keyidstr, n=0; n < 16; s += 2, n++)
        tmp_sn[n] = xtoi_2 (s);
      
      if (app->serialnolen != 16)
        return gpg_error (GPG_ERR_INV_CARD);
      if (memcmp (app->serialno, tmp_sn, 16))
        return gpg_error (GPG_ERR_WRONG_CARD);
    }

  /* If a fingerprint has been specified check it against the one on
     the card.  This is allows for a meaningful error message in case
     the key on the card has been replaced but the shadow information
     known to gpg was not updated.  If there is no fingerprint, gpg
     will detect a bogus signature anyway due to the
     verify-after-signing feature. */
  rc = fpr? check_against_given_fingerprint (app, fpr, 3) : 0;
  if (rc)
    return rc;

  rc = verify_chv2 (app, pincb, pincb_arg);
  if (!rc)
    rc = iso7816_internal_authenticate (app->slot, indata, indatalen,
                                        outdata, outdatalen);
  return rc;
}


static gpg_error_t 
do_decipher (app_t app, const char *keyidstr,
             gpg_error_t (*pincb)(void*, const char *, char **),
             void *pincb_arg,
             const void *indata, size_t indatalen,
             unsigned char **outdata, size_t *outdatalen )
{
  int rc;
  unsigned char tmp_sn[20]; /* actually 16 but we use it also for the fpr. */
  const char *s;
  int n;
  const char *fpr = NULL;

  if (!keyidstr || !*keyidstr || !indatalen)
    return gpg_error (GPG_ERR_INV_VALUE);

  /* Check whether an OpenPGP card of any version has been requested. */
  if (strlen (keyidstr) < 32 || strncmp (keyidstr, "D27600012401", 12))
    return gpg_error (GPG_ERR_INV_ID);
  
  for (s=keyidstr, n=0; hexdigitp (s); s++, n++)
    ;
  if (n != 32)
    return gpg_error (GPG_ERR_INV_ID);
  else if (!*s)
    ; /* no fingerprint given: we allow this for now. */
  else if (*s == '/')
    fpr = s + 1; 
  else
    return gpg_error (GPG_ERR_INV_ID);

  for (s=keyidstr, n=0; n < 16; s += 2, n++)
    tmp_sn[n] = xtoi_2 (s);

  if (app->serialnolen != 16)
    return gpg_error (GPG_ERR_INV_CARD);
  if (memcmp (app->serialno, tmp_sn, 16))
    return gpg_error (GPG_ERR_WRONG_CARD);

  /* If a fingerprint has been specified check it against the one on
     the card.  This is allows for a meaningful error message in case
     the key on the card has been replaced but the shadow information
     known to gpg was not updated.  If there is no fingerprint, the
     decryption will won't produce the right plaintext anyway. */
  rc = fpr? check_against_given_fingerprint (app, fpr, 2) : 0;
  if (rc)
    return rc;

  rc = verify_chv2 (app, pincb, pincb_arg);
  if (!rc)
    rc = iso7816_decipher (app->slot, indata, indatalen, 0,
                           outdata, outdatalen);
  return rc;
}


/* Perform a simple verify operation for CHV1 and CHV2, so that
   further operations won't ask for CHV2 and it is possible to do a
   cheap check on the PIN: If there is something wrong with the PIN
   entry system, only the regular CHV will get blocked and not the
   dangerous CHV3.  KEYIDSTR is the usual card's serial number; an
   optional fingerprint part will be ignored.

   There is a special mode if the keyidstr is "<serialno>[CHV3]" with
   the "[CHV3]" being a literal string:  The Admin Pin is checked if
   and only if the retry counter is still at 3. */
static gpg_error_t 
do_check_pin (app_t app, const char *keyidstr,
              gpg_error_t (*pincb)(void*, const char *, char **),
              void *pincb_arg)
{
  unsigned char tmp_sn[20]; 
  const char *s;
  int n;
  int admin_pin = 0;

  if (!keyidstr || !*keyidstr)
    return gpg_error (GPG_ERR_INV_VALUE);

  /* Check whether an OpenPGP card of any version has been requested. */
  if (strlen (keyidstr) < 32 || strncmp (keyidstr, "D27600012401", 12))
    return gpg_error (GPG_ERR_INV_ID);
  
  for (s=keyidstr, n=0; hexdigitp (s); s++, n++)
    ;
  if (n != 32)
    return gpg_error (GPG_ERR_INV_ID);
  else if (!*s)
    ; /* No fingerprint given: we allow this for now. */
  else if (*s == '/')
    ; /* We ignore a fingerprint. */
  else if (!strcmp (s, "[CHV3]") )
    admin_pin = 1;
  else
    return gpg_error (GPG_ERR_INV_ID);

  for (s=keyidstr, n=0; n < 16; s += 2, n++)
    tmp_sn[n] = xtoi_2 (s);

  if (app->serialnolen != 16)
    return gpg_error (GPG_ERR_INV_CARD);
  if (memcmp (app->serialno, tmp_sn, 16))
    return gpg_error (GPG_ERR_WRONG_CARD);

  /* Yes, there is a race conditions: The user might pull the card
     right here and we won't notice that.  However this is not a
     problem and the check above is merely for a graceful failure
     between operations. */

  if (admin_pin)
    {
      void *relptr;
      unsigned char *value;
      size_t valuelen;
      int count;
      
      relptr = get_one_do (app, 0x00C4, &value, &valuelen, NULL);
      if (!relptr || valuelen < 7)
        {
          log_error (_("error retrieving CHV status from card\n"));
          xfree (relptr);
          return gpg_error (GPG_ERR_CARD);
        }
      count = value[6];
      xfree (relptr);

      if (!count)
        {
          log_info (_("card is permanently locked!\n"));
          return gpg_error (GPG_ERR_BAD_PIN);
        }
      else if (value[6] < 3)
        {
          log_info (_("verification of Admin PIN is currently prohibited "
                      "through this command\n"));
          return gpg_error (GPG_ERR_GENERAL);
        }

      app->did_chv3 = 0; /* Force verification.  */
      return verify_chv3 (app, pincb, pincb_arg);
    }
  else
    return verify_chv2 (app, pincb, pincb_arg);
}




/* Select the OpenPGP application on the card in SLOT.  This function
   must be used before any other OpenPGP application functions. */
gpg_error_t
app_select_openpgp (app_t app)
{
  static char const aid[] = { 0xD2, 0x76, 0x00, 0x01, 0x24, 0x01 };
  int slot = app->slot;
  int rc;
  unsigned char *buffer;
  size_t buflen;
  void *relptr;
  
  rc = iso7816_select_application (slot, aid, sizeof aid);
  if (!rc)
    {
      unsigned int manufacturer;

      app->apptype = "OPENPGP";

      app->did_chv1 = 0;
      app->did_chv2 = 0;
      app->did_chv3 = 0;
      app->app_local = NULL;

      /* The OpenPGP card returns the serial number as part of the
         AID; because we prefer to use OpenPGP serial numbers, we
         replace a possibly already set one from a EF.GDO with this
         one.  Note, that for current OpenPGP cards, no EF.GDO exists
         and thus it won't matter at all. */
      rc = iso7816_get_data (slot, 0x004F, &buffer, &buflen);
      if (rc)
        goto leave;
      if (opt.verbose)
        {
          log_info ("AID: ");
          log_printhex ("", buffer, buflen);
        }

      app->card_version = buffer[6] << 8;
      app->card_version |= buffer[7];
      manufacturer = (buffer[8]<<8 | buffer[9]);

      xfree (app->serialno);
      app->serialno = buffer;
      app->serialnolen = buflen;
      buffer = NULL;
      app->app_local = xtrycalloc (1, sizeof *app->app_local);
      if (!app->app_local)
        {
          rc = gpg_error (gpg_err_code_from_errno (errno));
          goto leave;
        }

      relptr = get_one_do (app, 0x00C4, &buffer, &buflen, NULL);
      if (!relptr)
        {
          log_error (_("can't access %s - invalid OpenPGP card?\n"),
                     "CHV Status Bytes");
          goto leave;
        }
      app->force_chv1 = (buflen && *buffer == 0);
      xfree (relptr);

      relptr = get_one_do (app, 0x00C0, &buffer, &buflen, NULL);
      if (!relptr)
        {
          log_error (_("can't access %s - invalid OpenPGP card?\n"),
                     "Extended Capability Flags" );
          goto leave;
        }
      if (buflen)
        {
          app->app_local->extcap.get_challenge    = !!(*buffer & 0x40);
          app->app_local->extcap.key_import       = !!(*buffer & 0x20);
          app->app_local->extcap.change_force_chv = !!(*buffer & 0x10);
          app->app_local->extcap.private_dos      = !!(*buffer & 0x08);
        }
      xfree (relptr);
      
      /* Some of the first cards accidently don't set the
         CHANGE_FORCE_CHV bit but allow it anyway. */
      if (app->card_version <= 0x0100 && manufacturer == 1)
        app->app_local->extcap.change_force_chv = 1;

      parse_login_data (app);

      if (opt.verbose > 1)
        dump_all_do (slot);

      app->fnc.deinit = do_deinit;
      app->fnc.learn_status = do_learn_status;
      app->fnc.readkey = do_readkey;
      app->fnc.getattr = do_getattr;
      app->fnc.setattr = do_setattr;
      app->fnc.writekey = do_writekey;
      app->fnc.genkey = do_genkey;
      app->fnc.sign = do_sign;
      app->fnc.auth = do_auth;
      app->fnc.decipher = do_decipher;
      app->fnc.change_pin = do_change_pin;
      app->fnc.check_pin = do_check_pin;
   }

leave:
  if (rc)
    do_deinit (app);
  return rc;
}



