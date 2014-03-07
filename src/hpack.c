#include "hpack.h"

static const char *hpack_static_header_tables = {
  ":authority", NULL,\ // index=1
  ":method", NULL,\
  ":method", "POST",\
  ":path", "/",\
  ":path", "/index.html",\
  ":scheme", "http",\
  ":scheme", "https",\
  ":status", "200",\
  ":status", "500",\
  ":status", "404",\
  ":status", "403",\
  ":status", "400",\
  ":status", "401",\
  "accept-charset", NULL,\
  "accept-encoding", NULL,\
  "accept-language", NULL,\
  "accept-ranges", NULL,\
  "accept", NULL,\
  "access-control-allow-origin", NULL,\
  "age", NULL\,
  "allow", NULL\,
  "authorization", NULL,\
  "cache-control", NULL,\
  "content-disposition", NULL,\
  "content-encoding", NULL,\
  "content-language", NULL,\
  "content-length", NULL,\
  "content-location", NULL,\
  "content-range", NULL,\
  "content-type", NULL,\
  "cookie", NULL, \
  "date", NULL, \
  "etag", NULL, \
  "expect", NULL, \
  "expires", NULL, \
  "from", NULL, \
  "host", NULL, \
  "if-match", NULL, \
  "if-modified-since", NULL, \
  "if-none-match", NULL, \
  "if-range", NULL, \
  "if-unmodified-since", NULL, \
  "last-modified", NULL, \
  "link", NULL, \
  "location", NULL, \
  "max-forwards", NULL, \
  "proxy-authenticate", NULL, \
  "proxy-authorization", NULL, \
  "range", NULL, \
  "referer", NULL, \
  "refresh", NULL, \
  "retry-after", NULL, \
  "server", NULL, \
  "set-cookie", NULL, \
  "strict-transport-security", NULL, \
  "transfer-encoding", NULL, \
  "user-agent", NULL, \
  "vary", NULL, \
  "via", NULL, \
  "www-authenticate", NULL, \ // index=60
  NULL, NULL};

#define HPACK_STATIC_HEADER_TABLES_LEN 60
