#include <fcntl.h>
#include <libgen.h>
#include <libwebsockets.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

#ifndef _WIN32
#include <pwd.h>
#include <sys/types.h>
#endif

#include "html.h"
#include "server.h"
#include "utils.h"

static void gen_random_string(char* s, const int len) {
  if (len <= 0) return;
  static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  unsigned char* rnd = xmalloc(len);
  if (lws_get_random(context, rnd, len) == len) {
    for (int i = 0; i < len; ++i) {
      s[i] = alphanum[rnd[i] % (sizeof(alphanum) - 1)];
    }
  }
  s[len] = 0;
  free(rnd);
}

static void add_download_token(const char* token, const char* filepath) {
  struct download_token* t = xmalloc(sizeof(struct download_token));
  strncpy(t->token, token, sizeof(t->token) - 1);
  t->token[sizeof(t->token) - 1] = 0;
  t->filepath = strdup(filepath);
  t->created_at = time(NULL);
  t->next = download_tokens;
  download_tokens = t;
}

static char* get_and_revoke_download_token(const char* token) {
  struct download_token** curr = &download_tokens;
  while (*curr) {
    if (timingsafe_strcmp((*curr)->token, token) == 0) {
      struct download_token* found = *curr;
      *curr = found->next;
      char* fp = found->filepath;
      free(found);
      return fp;
    }
    curr = &(*curr)->next;
  }
  return NULL;
}

enum { AUTH_OK, AUTH_FAIL, AUTH_ERROR };

static char* html_cache = NULL;
static size_t html_cache_len = 0;

static int send_unauthorized(struct lws* wsi, unsigned int code, enum lws_token_indexes header) {
  unsigned char buffer[1024 + LWS_PRE], *p, *end;
  p = buffer + LWS_PRE;
  end = p + sizeof(buffer) - LWS_PRE;

  if (lws_add_http_header_status(wsi, code, &p, end) ||
      lws_add_http_header_by_token(wsi, header, (unsigned char*)"Basic realm=\"ttyd\"", 18, &p, end) ||
      lws_add_http_header_content_length(wsi, 0, &p, end) || lws_finalize_http_header(wsi, &p, end) ||
      lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS) < 0)
    return AUTH_FAIL;

  return lws_http_transaction_completed(wsi) ? AUTH_FAIL : AUTH_ERROR;
}

static int check_auth(struct lws* wsi, struct pss_http* pss) {
  if (server->auth_header != NULL) {
    if (lws_hdr_custom_length(wsi, server->auth_header, strlen(server->auth_header)) > 0) return AUTH_OK;
    return send_unauthorized(wsi, HTTP_STATUS_PROXY_AUTH_REQUIRED, WSI_TOKEN_HTTP_PROXY_AUTHENTICATE);
  }

  if (server->credential != NULL) {
    char buf[256];
    int len = lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_AUTHORIZATION);
    if (len >= 7 && strstr(buf, "Basic ")) {
      if (timingsafe_strcmp(buf + 6, server->credential) == 0) return AUTH_OK;
    }
    return send_unauthorized(wsi, HTTP_STATUS_UNAUTHORIZED, WSI_TOKEN_HTTP_WWW_AUTHENTICATE);
  }

  return AUTH_OK;
}

static bool accept_gzip(struct lws* wsi) {
  char buf[256];
  int len = lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_ACCEPT_ENCODING);
  return len > 0 && strstr(buf, "gzip") != NULL;
}

static bool uncompress_html(char** output, size_t* output_len) {
  if (html_cache == NULL || html_cache_len == 0) {
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    if (inflateInit2(&stream, 16 + 15) != Z_OK) return false;

    html_cache_len = index_html_size;
    html_cache = xmalloc(html_cache_len);

    stream.avail_in = index_html_len;
    stream.avail_out = html_cache_len;
    stream.next_in = (void*)index_html;
    stream.next_out = (void*)html_cache;

    int ret = inflate(&stream, Z_SYNC_FLUSH);
    inflateEnd(&stream);
    if (ret != Z_STREAM_END) {
      free(html_cache);
      html_cache = NULL;
      html_cache_len = 0;
      return false;
    }
  }

  *output = html_cache;
  *output_len = html_cache_len;

  return true;
}

static void pss_buffer_free(struct pss_http* pss) {
  if (pss->buffer != (char*)index_html && pss->buffer != html_cache) free(pss->buffer);
  pss->buffer = NULL;
}

static void access_log(struct lws* wsi, const char* path) {
  char rip[50];

  lws_get_peer_simple(lws_get_network_wsi(wsi), rip, sizeof(rip));
  lwsl_notice("HTTP %s - %s\n", path, rip);
}

static int send_json(struct lws* wsi, struct pss_http* pss, int status, const char* body, size_t body_len) {
  unsigned char buffer[1024 + LWS_PRE], *p, *end;
  p = buffer + LWS_PRE;
  end = p + sizeof(buffer) - LWS_PRE;

  if (lws_add_http_header_status(wsi, status, &p, end) ||
      lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (unsigned char*)"application/json", 16, &p, end) ||
      lws_add_http_header_content_length(wsi, (unsigned long)body_len, &p, end) || lws_finalize_http_header(wsi, &p, end) ||
      lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS) < 0) {
    return 1;
  }

  pss->buffer = pss->ptr = strdup(body);
  pss->len = body_len;
  lws_callback_on_writable(wsi);

  return 0;
}

static int send_error(struct lws* wsi, struct pss_http* pss, int status, const char* msg) {
  char body[512];
  int body_len = snprintf(body, sizeof(body), "{\"error\": \"%s\"}", msg);
  if (body_len >= (int)sizeof(body)) body_len = sizeof(body) - 1;

  return send_json(wsi, pss, status, body, (size_t)body_len);
}

int callback_http(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len) {
  struct pss_http* pss = (struct pss_http*)user;
  unsigned char buffer[4096 + LWS_PRE], *p, *end;
  char buf[4096];
  bool done = false;

  switch (reason) {
    case LWS_CALLBACK_HTTP:
      access_log(wsi, (const char*)in);
      snprintf(pss->path, sizeof(pss->path), "%s", (const char*)in);

      pss->upload_fd = -1;

      switch (check_auth(wsi, pss)) {
        case AUTH_OK:
          break;
        case AUTH_FAIL:
          return 0;
        case AUTH_ERROR:
        default:
          return 1;
      }

      p = buffer + LWS_PRE;
      end = p + sizeof(buffer) - LWS_PRE;

      // Handle Download Token: GET /download_token?path=...
      if (strncmp(pss->path, "/download_token", 15) == 0) {
        if (!server->writable) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File download is disabled (read-only mode)");
        }
        char path_arg[4096] = "";
        if (lws_get_urlarg_by_name(wsi, "path", path_arg, sizeof(path_arg)) < 0) {
          return send_error(wsi, pss, HTTP_STATUS_BAD_REQUEST, "Missing path arg");
        }

        if (!check_path(path_arg, server->cwd ? server->cwd : ".")) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File outside of working directory");
        }

        if (access(path_arg, R_OK) != 0) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File not readable or not found");
        }

        char token[37];
        gen_random_string(token, 36);
        add_download_token(token, path_arg);

        int n = snprintf(buf, sizeof(buf), "{\"token\": \"%s\"}", token);
        return send_json(wsi, pss, HTTP_STATUS_OK, buf, (size_t)n);
      }

      // Handle Download: GET /download/<token>
      if (strncmp(pss->path, "/download/", 10) == 0) {
        char* token = pss->path + 10;
        char* filepath = get_and_revoke_download_token(token);
        if (!filepath) {
          return send_error(wsi, pss, HTTP_STATUS_NOT_FOUND, "Invalid or expired token");
        }

        char* filename = strrchr(filepath, '/');
        if (filename) {
          filename++;  // skip the slash
        } else {
          filename = filepath;
        }

        char headers[1024];
        int headers_len =
            snprintf(headers, sizeof(headers), "Content-Disposition: attachment; filename=\"%s\"\r\n", filename);

        int n = lws_serve_http_file(wsi, filepath, "application/octet-stream", headers, headers_len);
        free(filepath);
        if (n < 0 || (n > 0 && lws_http_transaction_completed(wsi))) return 1;
        return 0;
      }

      // Handle Upload: POST /upload?path=...
      if (strncmp(pss->path, "/upload", 7) == 0) {
        if (!server->writable) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File upload is disabled (read-only mode)");
        }
        char path_arg[4096] = "";
        if (lws_get_urlarg_by_name(wsi, "path", path_arg, sizeof(path_arg)) < 0) {
          return send_error(wsi, pss, HTTP_STATUS_BAD_REQUEST, "Missing path arg");
        }

        char full_path[4096];
        strncpy(full_path, path_arg, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = 0;

        // Check if path is a directory
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
          char filename[256] = "";
          if (lws_get_urlarg_by_name(wsi, "filename", filename, sizeof(filename)) > 0) {
            if (!validate_filename(filename)) {
              return send_error(wsi, pss, HTTP_STATUS_BAD_REQUEST, "Invalid filename");
            }
            size_t current_len = strlen(full_path);
            bool add_slash = current_len > 0 && full_path[current_len - 1] != '/';
            snprintf(full_path + current_len, sizeof(full_path) - current_len, "%s%s", add_slash ? "/" : "", filename);
          } else {
            return send_error(wsi, pss, HTTP_STATUS_BAD_REQUEST, "Target is a directory but no filename provided");
          }
        }

#ifndef _WIN32
        struct stat lst;
        if (lstat(full_path, &lst) == 0 && S_ISLNK(lst.st_mode)) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File is a symbolic link");
        }
#endif

        bool path_ok = false;
        if (stat(full_path, &st) == 0) {
          path_ok = check_path(full_path, server->cwd ? server->cwd : ".");
        } else {
          char path_dup[4096];
          strncpy(path_dup, full_path, sizeof(path_dup));
          path_dup[sizeof(path_dup) - 1] = 0;
          path_ok = check_path(dirname(path_dup), server->cwd ? server->cwd : ".");
        }

        if (!path_ok) {
          return send_error(wsi, pss, HTTP_STATUS_FORBIDDEN, "File outside of working directory");
        }

        pss->upload_fd = open(full_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (pss->upload_fd < 0) {
          int err = errno;  // Capture errno immediately
          lwsl_err("Failed to open file for upload: %s (errno: %d)\n", full_path, err);

          char err_msg[256];
          snprintf(err_msg, sizeof(err_msg), "Could not open file: %s", strerror(err));

          return send_error(wsi, pss, HTTP_STATUS_INTERNAL_SERVER_ERROR, err_msg);
        }
#ifndef _WIN32
        if (server->chown_uploaded && geteuid() == 0 && server->username != NULL) {
          struct passwd *pw = getpwnam(server->username);
          if (pw != NULL) {
            if (fchown(pss->upload_fd, pw->pw_uid, pw->pw_gid) != 0) {
              lwsl_warn("Failed to chown uploaded file to user %s: %s\n", server->username, strerror(errno));
            }
          }
        }
#endif
        break;
      }

      if (strcmp(pss->path, endpoints.token) == 0) {
        const char* credential = server->credential != NULL ? server->credential : "";
        int n = snprintf(buf, sizeof(buf), "{\"token\": \"%s\"}", credential);
        return send_json(wsi, pss, HTTP_STATUS_OK, buf, (size_t)n);
      }

      // redirects `/base-path` to `/base-path/`
      if (strcmp(pss->path, endpoints.parent) == 0) {
        if (lws_add_http_header_status(wsi, HTTP_STATUS_FOUND, &p, end) ||
            lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_LOCATION, (unsigned char*)endpoints.index,
                                         (int)strlen(endpoints.index), &p, end) ||
            lws_add_http_header_content_length(wsi, 0, &p, end) || lws_finalize_http_header(wsi, &p, end) ||
            lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS) < 0)
          return 1;
        goto try_to_reuse;
      }

      if (strcmp(pss->path, endpoints.index) != 0) {
        lws_return_http_status(wsi, HTTP_STATUS_NOT_FOUND, NULL);
        goto try_to_reuse;
      }

      const char* content_type = "text/html";
      if (server->index != NULL) {
        int n = lws_serve_http_file(wsi, server->index, content_type, NULL, 0);
        if (n < 0 || (n > 0 && lws_http_transaction_completed(wsi))) return 1;
      } else {
        char* output = (char*)index_html;
        size_t output_len = index_html_len;
        if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end) ||
            lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (const unsigned char*)content_type, 9, &p,
                                         end))
          return 1;
#ifdef LWS_WITH_HTTP_STREAM_COMPRESSION
        if (!uncompress_html(&output, &output_len)) return 1;
#else
        if (accept_gzip(wsi)) {
          if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_ENCODING, (unsigned char*)"gzip", 4, &p, end))
            return 1;
        } else {
          if (!uncompress_html(&output, &output_len)) return 1;
        }
#endif

        if (lws_add_http_header_content_length(wsi, (unsigned long)output_len, &p, end) ||
            lws_finalize_http_header(wsi, &p, end) ||
            lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS) < 0)
          return 1;

        pss->buffer = pss->ptr = output;
        pss->len = output_len;
        lws_callback_on_writable(wsi);
      }
      break;

    case LWS_CALLBACK_HTTP_BODY:
      if (pss->upload_fd != -1) {
        if (write(pss->upload_fd, in, len) < (ssize_t)len) {
          lwsl_err("Write failed during upload\n");
          close(pss->upload_fd);
          pss->upload_fd = -1;
          return 1;
        }
      }
      break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
      if (pss->upload_fd != -1) {
        close(pss->upload_fd);
        pss->upload_fd = -1;

        p = buffer + LWS_PRE;
        end = p + sizeof(buffer) - LWS_PRE;

        if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end) ||
            lws_add_http_header_content_length(wsi, 0, &p, end) || lws_finalize_http_header(wsi, &p, end) ||
            lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE), LWS_WRITE_HTTP_HEADERS) < 0)
          return 1;

        return lws_http_transaction_completed(wsi) ? 1 : 0;
      }
      break;

    case LWS_CALLBACK_CLOSED_HTTP:
      if (pss && pss->upload_fd != -1) {
        close(pss->upload_fd);
        pss->upload_fd = -1;
      }
      if (pss) pss_buffer_free(pss);
      break;

    case LWS_CALLBACK_HTTP_WRITEABLE:
      if (!pss->buffer || pss->len == 0) {
        goto try_to_reuse;
      }

      do {
        int n = sizeof(buffer) - LWS_PRE;
        int m = lws_get_peer_write_allowance(wsi);
        if (m == 0) {
          lws_callback_on_writable(wsi);
          return 0;
        } else if (m != -1 && m < n) {
          n = m;
        }
        if (pss->ptr + n > pss->buffer + pss->len) {
          n = (int)(pss->len - (pss->ptr - pss->buffer));
          done = true;
        }
        memcpy(buffer + LWS_PRE, pss->ptr, n);
        pss->ptr += n;
        if (lws_write_http(wsi, buffer + LWS_PRE, (size_t)n) < n) {
          pss_buffer_free(pss);
          return -1;
        }
      } while (!lws_send_pipe_choked(wsi) && !done);

      if (!done && pss->ptr < pss->buffer + pss->len) {
        lws_callback_on_writable(wsi);
        break;
      }

      pss_buffer_free(pss);
      goto try_to_reuse;

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
      goto try_to_reuse;
#if (defined(LWS_OPENSSL_SUPPORT) || defined(LWS_WITH_TLS)) && !defined(LWS_WITH_MBEDTLS)
    case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:
      if (!len || (SSL_get_verify_result((SSL*)in) != X509_V_OK)) {
        int err = X509_STORE_CTX_get_error((X509_STORE_CTX*)user);
        int depth = X509_STORE_CTX_get_error_depth((X509_STORE_CTX*)user);
        const char* msg = X509_verify_cert_error_string(err);
        lwsl_err("client certificate verification error: %s (%d), depth: %d\n", msg, err, depth);
        return 1;
      }
      break;
#endif
    default:
      break;
  }

  return 0;

  /* if we're on HTTP1.1 or 2.0, will keep the idle connection alive */
try_to_reuse:
  if (lws_http_transaction_completed(wsi)) return -1;

  return 0;
}
