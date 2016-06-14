/***************************************************************************
 *
 * Copyright (c) 2000-2015 BalaBit IT Ltd, Budapest, Hungary
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Notes:
 *   Based on the code by: Viktor Peter Kovacs <vps__@freemail.hu>
 *
 ***************************************************************************/

#include "http.h"

#include <zorp/log.h>
#include <zorp/parse.h>
#include <ctype.h>

/*LOG
 This message indicates that the server sent an invalid HTTP
 header.
 */
const char* INVALID_HTTP_HEADER = "Invalid HTTP header";

/*LOG
 This message indicates that Zorp fetched an invalid header from the server.
 It is likely caused by a buggy server.
 */
const char* FIRST_HEADER_STARTS_WITH_WHITE_SPACE = "First header starts with white space";

/*LOG
  This message indicates that the server tried to send more header
  lines, than the allowed maximum.  Check the max_header_lines
  attribute.
*/
const char* TOO_MANY_HEADER_LINES_MAX_HEADER_LINES_D = "Too many header lines; max_header_lines='%d'";
/*LOG
  This message indicates that Zorp was unable to fetch headers
  from the server.  Check the permit_null_response attribute.
*/
const char* ERROR_READING_FROM_PEER_WHILE_FETCHING_HEADERS = "Error reading from peer while fetching headers";

/* these headers are not allowed to be duplicated, the first header is
 * used, the rest is dropped. All headers that is likely to be used
 * for an access control decision should be added here
 */
static const gchar *smuggle_headers[] =
  {
    "Content-Length",      /* makes complete request smuggling possible */
    "Transfer-Encoding",   /* trick the proxy to use a different transfer-encoding than the server */
    "Content-Type",        /* different content-types */
    "Host",                /* different hostname in URL, e.g. http://goodsite.org/index.html instead of http://evilsite.org/index.html */
    "Connection",          /* different connection mode, might cause the connection to stall */
    "Proxy-Connection",    /* -"- */
    "Authorization",       /* different credentials (username/password) */
    "Proxy-Authorization"  /* -"- */
  };

class HeaderParser: LineParser {
public:
    HeaderParser(HttpProxy* proxy, ZEndpoint side);
    gboolean fetchall();
    void newLine(const gchar* line, gint bufferLength) override;
    ~HeaderParser();
private:
    enum HttpHeaderStatus {
        normalLine,
        lastLine,
        skipLine,
        continuationLine
    };
    ZEndpoint side;
    HttpProxy *self;
    HttpHeaders *headers;
    guint count;
    HttpHeader *last_hdr;
    HttpHeader *newHeader;
    const char* WHITESPACE = " \t";
    const char* BLACKLIST="()<>@,;:\\\"/[]?={} \t";
    GString *name=NULL;
    GString *value=NULL;
    HttpHeaderStatus http_fetch_one_header_line ();
    HttpHeaderStatus http_header_parse_one_line ();
    HttpHeader* http_initial_create_header ();
};

gboolean
http_header_equal(gconstpointer k1, gconstpointer k2)
{
  return g_ascii_strcasecmp(static_cast<const gchar *>(k1), static_cast<const gchar *>(k2)) == 0;
}

guint
http_header_hash(gconstpointer key)
{
  const char *p = static_cast<const char *>(key);
  guint h = toupper(*p);

  if (h)
    for (p += 1; *p != '\0'; p++)
      h = (h << 5) - h + toupper(*p);

  return h;
}

static void
http_header_free(HttpHeader *h)
{
  g_string_free(h->name, TRUE);
  g_string_free(h->value, TRUE);
  g_free(h);
}

void
http_log_headers(HttpProxy *self, ZEndpoint side, const gchar *tag)
{
  HttpHeaders *hdrs = &self->headers[side];

  if ((side == EP_CLIENT && z_log_enabled(HTTP_REQUEST, 7)) ||
      (side == EP_SERVER && z_log_enabled(HTTP_RESPONSE, 7)))
    {
      GList *l = g_list_last(hdrs->list);

      while (l)
        {
          HttpHeader *hdr = (HttpHeader *) l->data;

          if (hdr->present)
            {
              if (side == EP_CLIENT)
                /*NOLOG*/
                z_proxy_log(self, HTTP_REQUEST, 7, "Request %s header; hdr='%s', value='%s'", tag,
                            hdr->name->str, hdr->value->str);
              else
                /*NOLOG*/
                z_proxy_log(self, HTTP_RESPONSE, 7, "Response %s header; hdr='%s', value='%s'", tag,
                            hdr->name->str, hdr->value->str);
            }

          l = g_list_previous(l);
        }
    }
}

HttpHeader*
http_initial_create_header (const gchar* name, gint name_len, gint value_len, gchar* value)
{
    HttpHeader *h;
    h = g_new0(HttpHeader, 1);
    h->name = g_string_sized_new (name_len + 1);
    g_string_assign_len (h->name, name, name_len);
    h->value = g_string_sized_new (value_len + 1);
    g_string_assign_len (h->value, value, value_len);
    h->present = TRUE;
    return h;
}

bool
find_in_smuggle_headers (HttpHeader* h)
{
    guint i;
    for (i = 0; i < sizeof(smuggle_headers) / sizeof(smuggle_headers[0]); i++)
        if (strcmp (smuggle_headers[i], h->name->str) == 0)
            return true;
    return false;
}

HttpHeader*
http_finalize_header (HttpHeaders* hdrs, HttpHeader* h)
{
    HttpHeader* orig;
    if (!http_lookup_header (hdrs, h->name->str, &orig))
    {
        hdrs->list = g_list_prepend (hdrs->list, h);
        g_hash_table_insert (hdrs->hash, h->name->str, hdrs->list);
    }
    else
    {
        bool found = find_in_smuggle_headers (h);
        if (false == found)
        {
            hdrs->list = g_list_prepend (hdrs->list, h);
        }
        else
        {
            z_log(NULL, HTTP_VIOLATION, 3,
                  "Possible smuggle attack, removing header duplication; header='%.*s', value='%.*s', prev_value='%.*s'", h->name->len,
                  h->name->str, h->value->len, h->value->str, (gint ) orig->value->len, orig->value->str);
            http_header_free (h);
            h = NULL;
        }
    }
    return h;
}

/* duplicated headers are simply put on the list and not inserted into
   the hash, thus looking up a header by name always results the first
   added header */

HttpHeader *
http_add_header(HttpHeaders *hdrs, const gchar *name, gint name_len, gchar *value, gint value_len)
{
  HttpHeader *h;

  h = http_initial_create_header (name, name_len, value_len, value);
  return http_finalize_header (hdrs, h);
}

static gboolean
http_clear_header(gpointer key G_GNUC_UNUSED,
                  gpointer value G_GNUC_UNUSED,
                  gpointer user_data G_GNUC_UNUSED)
{
  return TRUE;
}

void
http_clear_headers(HttpHeaders *hdrs)
{
  GList *l;

  for (l = hdrs->list; l; l = l->next)
    http_header_free(static_cast<HttpHeader *>(l->data));

  g_list_free(hdrs->list);
  hdrs->list = NULL;
  g_string_truncate(hdrs->flat, 0);
  g_hash_table_foreach_remove(hdrs->hash, http_clear_header, NULL);
}

gboolean
http_lookup_header(HttpHeaders *headers, const gchar *what, HttpHeader **p)
{
  GList *l;

  l = static_cast<GList *>(g_hash_table_lookup(headers->hash, what));

  if (l)
    {
      *p = static_cast<HttpHeader *>(l->data);
      return TRUE;
    }

  *p = NULL;
  return FALSE;
}

static void
http_insert_headers(gchar *key, ZPolicyObj *f, HttpHeaders *hdrs)
{
  guint filter_type;
  gchar *value;

  if (!z_policy_tuple_get_verdict(f, &filter_type))
    {
      /* filter has no type field */
      return;
    }

  switch (filter_type)
    {
    case HTTP_HDR_INSERT:
    case HTTP_HDR_REPLACE:

      if (!z_policy_var_parse(f, "(is)", &filter_type, &value))
        {
          /* error parsing HTTP_INSERT or HTTP_REPLACE rule */
          return;
        }

      http_add_header(hdrs, key, strlen(key), value, strlen(value));
      break;
    default:
      break;
    }
}

static gboolean
http_check_header_charset(HttpProxy *self, gchar *header, guint flags, const gchar **reason)
{
  gint i;

  *reason = FALSE;

  if (flags & HTTP_HDR_FF_ANY)
    return TRUE;

  if (flags & HTTP_HDR_FF_URL)
    {
      HttpURL url;
      gboolean success;

      http_init_url(&url);
      success = http_parse_url(&url, self->permit_unicode_url, self->permit_invalid_hex_escape, TRUE, header, reason);
      http_destroy_url(&url);

      return success;
    }

  for (i = 0; header[i]; i++)
    {
      gboolean ok;
      guchar c = header[i];

      ok = FALSE;

      if ((flags & HTTP_HDR_CF_ALPHA) &&
          ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_NUMERIC) &&
               (c >= '0' && c <= '9'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_SPACE) &&
               (c == ' '))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_COMMA) &&
               (c == ','))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_DOT) &&
               (c == '.'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_BRACKET) &&
               (c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_EQUAL) &&
               (c == '='))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_DASH) &&
               (c == '-'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_SLASH) &&
               (c == '/'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_COLON) &&
               (c == ':'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_SEMICOLON) &&
               (c == ';'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_AT) &&
               (c == '@'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_UNDERLINE) &&
               (c == '_'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_AND) &&
               (c == '&'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_BACKSLASH) &&
               (c == '\\'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_ASTERIX) &&
               (c == '*'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_DOLLAR) &&
               (c == '$'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_HASHMARK) &&
               (c == '#'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_PLUS) &&
               (c == '+'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_QUOTE) &&
               (c == '"' || c == '\''))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_QUESTIONMARK) &&
               (c == '?'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_PERCENT) &&
               (c == '%'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_TILDE) &&
               (c == '~'))
        ok = TRUE;
      else if ((flags & HTTP_HDR_CF_EXCLAM) &&
               (c == '!'))
        ok = TRUE;

      if (!ok)
        {
          *reason = "Invalid character found";
          return FALSE;
        }
    }

  return TRUE;
}

gboolean
http_filter_headers(HttpProxy *self, ZEndpoint side, HttpHeaderFilter filter)
{
  HttpHeaders *headers = &self->headers[side];
  GHashTable *hash = (side == EP_CLIENT) ? self->request_header_policy : self->response_header_policy;
  gint action;
  GList *l;

  z_proxy_enter(self);
  l = headers->list;

  while (l)
    {
      HttpHeader *h = (HttpHeader *) l->data;
      ZPolicyObj *f;

      if (filter)
        action = filter(self, h->name, h->value);
      else
        action = HTTP_HDR_ACCEPT;

      g_string_assign_len(self->current_header_name, h->name->str, h->name->len);
      g_string_assign_len(self->current_header_value, h->value->str, h->value->len);

      f = static_cast<ZPolicyObj *>(g_hash_table_lookup(hash, self->current_header_name->str));

      if (!f)
        f = static_cast<ZPolicyObj *>(g_hash_table_lookup(hash, "*"));

      if (f)
        {
          guint filter_type;
          ZPolicyObj *handler, *res;
          gchar *name, *value;

          z_policy_lock(self->super.thread);

          if (!z_policy_tuple_get_verdict(f, &filter_type))
            {
              /* filter has no type field */
              z_policy_unlock(self->super.thread);
              z_proxy_return(self, FALSE);
            }

          z_policy_unlock(self->super.thread);

          switch (filter_type)
            {
            case HTTP_HDR_POLICY:
              z_policy_lock(self->super.thread);

              if (!z_policy_var_parse(f, "(iO)", &filter_type, &handler))
                {
                  /* error parsing HTTP_POLICY_CALL rule */
                  z_proxy_report_invalid_policy(&(self->super));
                  z_policy_unlock(self->super.thread);
                  z_proxy_return(self, FALSE);
                }

              res = z_policy_call_object(handler,
                                         z_policy_var_build("(s#s#)",
                                                            h->name->str, h->name->len,
                                                            h->value->str, h->value->len),
                                         self->super.session_id);

              if (!z_policy_var_parse(res, "i", &action))
                {
                  /*LOG
                    This message indicates that the call-back for the given
                    header was invalid. Check your request_headers and
                    response_headers hashes.
                  */
                  z_proxy_log(self, HTTP_POLICY, 1, "Policy call-back for header returned invalid value; header='%s'", self->current_header_name->str);
                  z_proxy_report_policy_abort(&(self->super));
                  z_policy_var_unref(res);
                  z_policy_unlock(self->super.thread);
                  z_proxy_return(self, FALSE);
                }

              z_policy_var_unref(res);
              z_policy_unlock(self->super.thread);
              g_string_assign_len(h->name, self->current_header_name->str, self->current_header_name->len);
              g_string_assign_len(h->value, self->current_header_value->str, self->current_header_value->len);
              break;

            case HTTP_HDR_INSERT:
              /* insert header that already exists */
              action = HTTP_HDR_ACCEPT;
              break;

            case HTTP_HDR_ACCEPT:
              break;

            case HTTP_HDR_REPLACE:
            case HTTP_HDR_DROP:
              action = HTTP_HDR_DROP;
              break;

            case HTTP_HDR_CHANGE_NAME:
              z_policy_lock(self->super.thread);

              if (!z_policy_var_parse(f, "(is)", &filter_type, &name))
                {
                  /* invalid CHANGE_NAME rule */
                  /*LOG
                    This message indicates that the HDR_CHANGE_NAME
                    parameter is invalid, for the given header.  Check your
                    request_headers and response_headers hashes.
                  */
                  z_proxy_log(self, HTTP_POLICY, 1, "Invalid HTTP_HDR_CHANGE_NAME rule in header processing; header='%s'", self->current_header_name->str);
                  z_proxy_report_invalid_policy(&(self->super));
                  z_policy_unlock(self->super.thread);
                  z_proxy_return(self, FALSE);
                }

              g_string_assign(h->name, name);
              z_policy_unlock(self->super.thread);
              action = HTTP_HDR_ACCEPT;
              break;

            case HTTP_HDR_CHANGE_VALUE:
              z_policy_lock(self->super.thread);

              if (!z_policy_var_parse(f, "(is)", &filter_type, &value))
                {
                  /* invalid CHANGE_VALUE rule */
                  /*LOG
                    This message indicates that the HDR_CHANGE_VALUE
                    parameter is invalid, for the given header.  Check your
                    request_headers and response_headers hashes.
                  */
                  z_proxy_log(self, HTTP_POLICY, 1, "Invalid HTTP_HDR_CHANGE_VALUE rule in header processing; header='%s'", self->current_header_name->str);
                  z_policy_unlock(self->super.thread);
                  z_proxy_return(self, FALSE);
                }

              g_string_assign(h->value, value);
              z_policy_unlock(self->super.thread);
              action = HTTP_HDR_ACCEPT;
              break;

            case HTTP_HDR_CHANGE_BOTH:
              z_policy_lock(self->super.thread);

              if (!z_policy_var_parse(f, "(iss)", &filter_type, &name, &value))
                {
                  /* invalid CHANGE_BOTH rule */
                  /*LOG
                    This message indicates that the HDR_CHANGE_BOTH
                    parameter is invalid, for the given header.  Check your
                    request_headers and response_headers hashes.
                  */
                  z_proxy_log(self, HTTP_POLICY, 1, "Invalid HTTP_HDR_CHANGE_BOTH rule in header processing; header='%s'", self->current_header_name->str);
                  z_policy_unlock(self->super.thread);
                  z_proxy_return(self, FALSE);
                }

              g_string_assign(h->name, name);
              g_string_assign(h->value, value);
              z_policy_unlock(self->super.thread);
              action = HTTP_HDR_ACCEPT;
              break;

            case HTTP_HDR_ABORT:
              action = HTTP_HDR_ABORT;
              break;

            default:
              action = HTTP_HDR_ABORT;
              /*LOG
                This message indicates that the action is invalid, for the
                given header.  Check your request_headers and
                response_headers hashes.
              */
              z_proxy_log(self, HTTP_POLICY, 1, "Invalid value in header action tuple; header='%s', filter_type='%d'",
                          self->current_header_name->str, filter_type);
              break;
            }
        }

      if (action == HTTP_HDR_ACCEPT && self->strict_header_checking)
        {
          HttpElementInfo *info;
          const gchar *reason;

          if (side == EP_CLIENT)
            info = http_proto_request_hdr_lookup(h->name->str);
          else
            info = http_proto_response_hdr_lookup(h->name->str);

          if (info)
            {
              if (info->max_len >= 0 && h->value->len > (guint) info->max_len)
                {
                  z_proxy_log(self, HTTP_VIOLATION, 3,
                              "Header failed strict checking, value too long; "
                              "header='%s', value='%s', length='%" G_GSIZE_FORMAT "', max_length='%" G_GSIZE_FORMAT "'",
                              h->name->str, h->value->str, h->value->len, info->max_len);
                  action = self->strict_header_checking_action;
                  goto exit_check;
                }

              if (!http_check_header_charset(self, h->value->str, info->flags, &reason))
                {
                  z_proxy_log(self, HTTP_VIOLATION, 3,
                              "Header failed strict checking, it contains invalid characters; "
                              "header='%s', value='%s', reason='%s'",
                              h->name->str, h->value->str, reason);
                  action = self->strict_header_checking_action;
                  goto exit_check;
                }

            exit_check:
              ;
            }
          else
            {
              z_proxy_log(self, HTTP_VIOLATION, 3,
                          "Header failed strict checking, it is unknown; header='%s', value='%s'",
                          h->name->str, h->value->str);
              action = self->strict_header_checking_action;
            }
        }

      switch (action)
        {
        case HTTP_HDR_ACCEPT:
          break;

        case HTTP_HDR_DROP:
          h->present = FALSE;
          break;

        default:
          self->error_code = HTTP_MSG_POLICY_VIOLATION;
          z_policy_lock(self->super.thread);
          z_proxy_report_policy_abort(&(self->super));
          z_policy_unlock(self->super.thread);
          z_proxy_return(self, FALSE);
        }

      l = g_list_next(l);

    }

  z_policy_lock(self->super.thread);
  g_hash_table_foreach(hash, (GHFunc) http_insert_headers, headers);
  z_policy_unlock(self->super.thread);
  z_proxy_return(self, TRUE);
}

HeaderParser::HeaderParser(HttpProxy* proxy, ZEndpoint side) {
    this->self = proxy;
    this->side = side;
    headers = &(proxy->headers[side]);
    count = 0;
    last_hdr = NULL;
    newHeader = NULL;
}

HeaderParser::~HeaderParser() {
    if (name) {
    	g_string_free(name, TRUE);
    }
    if (value) {
    	g_string_free(value, TRUE);
    }
}

gboolean HeaderParser::fetchall() {
    z_proxy_enter(self);
    http_clear_headers(headers);

    if (self->proto_version[side] < 0x0100)
        z_proxy_return(self, TRUE);

    try {
        while (lastLine != http_fetch_one_header_line ());
    } catch(ParserException *e) {
        z_proxy_log(self, HTTP_VIOLATION, 2, "%s", e->what());
        z_proxy_return(self, FALSE);
    }

    /*  g_string_append(headers, "\r\n"); */
    http_log_headers(self, side, "prefilter");
    z_proxy_return(self, TRUE);
}

void HeaderParser::newLine(
        const gchar* line,
        gint bufferLength) {
    LineParser::newLine(line, bufferLength);
    if (name) {
    	g_string_free(name, TRUE);
    	name=NULL;
    }
    if (value) {
    	g_string_free(value, TRUE);
    	value=NULL;
    }
}

HeaderParser::HttpHeaderStatus HeaderParser::http_fetch_one_header_line ()
{
	gchar* line;
	gsize line_length;
	GIOStatus res;
	HttpHeaderStatus state;
	res = z_stream_line_get (self->super.endpoints[side], &line, &line_length, NULL);
	if (res != G_IO_STATUS_NORMAL)
	{
		if (res == G_IO_STATUS_EOF && side == EP_SERVER && self->permit_null_response)
			return lastLine;

		throw new ParserException (ERROR_READING_FROM_PEER_WHILE_FETCHING_HEADERS, line, line_length);
	}
	newLine(line, line_length);
	state = http_header_parse_one_line ();
	count++;
	if (normalLine == state) {
		newHeader = http_initial_create_header();
		last_hdr=http_finalize_header (headers, newHeader);
	}
	else if (continuationLine == state)
		g_string_append(last_hdr->value, value->str);

	if (count > self->max_header_lines)
		throw new ParserException (line, line_length, TOO_MANY_HEADER_LINES_MAX_HEADER_LINES_D, self->max_header_lines);
	return state;
}

HeaderParser::HttpHeaderStatus HeaderParser::http_header_parse_one_line ()
{
	if (isEmpty())
		return lastLine;

	if (isAt(WHITESPACE))
	{
		if (!last_hdr)
			throw new ParserException (FIRST_HEADER_STARTS_WITH_WHITE_SPACE,  origBuffer.line, origBuffer.bufferLength);
		skipChars (WHITESPACE);
		value = makeGString ();
		return continuationLine;
	}

	name = toGStringUntilDelimiter(BLACKLIST);

	skipChars(' ');
	if (!isAt(':'))
	{
		ParserException* e = new ParserException (INVALID_HTTP_HEADER, origBuffer.line, origBuffer.bufferLength);
		if (self->strict_header_checking)
			throw e;
		else
			z_proxy_log(self, HTTP_VIOLATION, 2, "%s", e->what ());
		return skipLine;
	}
	skipOne ();

	value = stripToGString();
	return normalLine;
}

HttpHeader* HeaderParser::http_initial_create_header ()
{
	HttpHeader *h;
	h = g_new0(HttpHeader, 1);
	h->name = name;
	name=NULL;
	h->value = value;
	value=NULL;
	h->present = TRUE;
	return h;
}

gboolean
http_fetch_headers(HttpProxy *self, ZEndpoint side)
{
    HeaderParser parser = HeaderParser(self,side);
    return parser.fetchall();
}

gboolean
http_flat_headers_into(HttpHeaders *hdrs, GString *into)
{
  GList *l = g_list_last(hdrs->list);

  g_string_truncate(into, 0);

  while (l)
    {
      if (((HttpHeader *) l->data)->present)
        {
          g_string_append_len(into, ((HttpHeader *) l->data)->name->str, ((HttpHeader *) l->data)->name->len);
          g_string_append_len(into, ": ", 2);
          g_string_append_len(into, ((HttpHeader *) l->data)->value->str, ((HttpHeader *) l->data)->value->len);
          g_string_append_len(into, "\r\n", 2);
        }

      l = g_list_previous(l);
    }

  return TRUE;
}

gboolean
http_flat_headers(HttpHeaders *hdrs)
{
  return http_flat_headers_into(hdrs, hdrs->flat);
}

void
http_init_headers(HttpHeaders *hdrs)
{
  hdrs->hash = g_hash_table_new(http_header_hash, http_header_equal);
  hdrs->flat = g_string_sized_new(256);
}

void
http_destroy_headers(HttpHeaders *hdrs)
{
  http_clear_headers(hdrs);
  g_hash_table_destroy(hdrs->hash);
  g_string_free(hdrs->flat, TRUE);
}

enum HttpCookieState
  {
    HTTP_COOKIE_NAME,
    HTTP_COOKIE_VALUE,
    HTTP_COOKIE_DOTCOMA
  };

GHashTable *
http_parse_header_cookie(HttpHeaders *hdrs)
{
  GHashTable *cookie_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  HttpHeader *hdr;

  if (http_lookup_header(hdrs, "Cookie", &hdr))
    {
      gint state = HTTP_COOKIE_NAME;
      gchar key[256];
      guint key_pos = 0;
      gchar value[4096];
      guint value_pos = 0;
      gint i = 0;
      gchar *hdr_str = hdr->value->str;

      while (hdr_str[i])
        {
          switch (state)
            {
            case HTTP_COOKIE_NAME:

              if (hdr_str[i] == '=')
                {
                  key[key_pos] = 0;
                  state = HTTP_COOKIE_VALUE;
                }
              else
                {
                  key[key_pos++] = hdr_str[i];
                }

              if (key_pos > sizeof(key))
                {
                  goto exit_error;
                }

              break;
            case HTTP_COOKIE_VALUE:

              if (hdr_str[i] == ';')
                {
                  state = HTTP_COOKIE_DOTCOMA;
                }
              else
                {
                  value[value_pos++] = hdr_str[i];
                }

              if (value_pos > sizeof(value))
                {
                  goto exit_error;
                }

              break;
            case HTTP_COOKIE_DOTCOMA:

              if (hdr_str[i] != ' ' &&
                  hdr_str[i] != '\r' &&
                  hdr_str[i] != '\n' &&
                  hdr_str[i] != '\t')
                {
                  if (hdr_str[i] == '$' && FALSE)
                    {
                      value[value_pos++] = hdr_str[i];

                      if (value_pos > sizeof(value))
                        {
                          goto exit_error;
                        }

                      state = HTTP_COOKIE_VALUE;
                    }
                  else
                    {
                      value[value_pos] = 0;
                      g_hash_table_insert(cookie_hash, g_strdup(key), g_strdup(value));
                      key_pos = value_pos = 0;
                      key[key_pos++] = hdr_str[i];
                      state = HTTP_COOKIE_NAME;
                    }
                }
            }

          i++;
        }

      if (key_pos && value_pos)
        {
          value[value_pos] = 0;
          g_hash_table_insert(cookie_hash, g_strdup(key), g_strdup(value));
          key_pos = value_pos = 0;
        }

      goto exit;
    }

 exit_error:
  g_hash_table_destroy(cookie_hash);
  cookie_hash = NULL;
 exit:

  return cookie_hash;
}
