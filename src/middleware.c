#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include "middleware.h"
#include "main.h"

/* Route */
typedef struct _Route {
    HTTPMethod method;
    char *uri;
    SAF saf;
} Route;

Route routes[MAX_HTTP_ROUTES];
int routes_used = 0;

/* Add an URI and the corresponding server application function into the route
   table. */
int AddRoute(HTTPMethod method, char *uri, SAF saf) {
    if (routes_used < MAX_HTTP_ROUTES) {
        routes[routes_used].method = method;
        routes[routes_used].uri = uri;
        routes[routes_used].saf = saf;
        routes_used++;

        return routes_used;
    } else {
        return 0;
    }
}

static char *_GetContentType(ini_t *inifile, char *uri) {
    char *ret = "application/octet-stream";
    size_t len = strlen(uri);
    char ext[4];

    // convert extension toupper case
    if (len > 4) {
        ext[3] = 0;
        ext[2] = toupper(uri[len - 1]);
        ext[1] = toupper(uri[len - 2]);
        ext[0] = toupper(uri[len - 3]);

        char *tmp = (char *)ini_get(inifile, "mimetype", ext);
        if (tmp) {
            ret = tmp;
        }
    }
    LOGF("MIME type for %s is %s\n", uri, ret);

    return ret;
}

/* Try to read static files under static folder. */
static uint8_t _ReadStaticFiles(HTTPServer *srv, HTTPReqMessage *req, HTTPResMessage *res) {
    uint8_t found = 0;
    int8_t depth = 0;
    char *uri = req->Header.URI;
    size_t n = strlen(uri);
    size_t i;

    FILE *fp;
    int size;
    char path[128];
    strncpy(path, srv->static_folder, sizeof(path) - 1);

    char header[1024];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\nConnection: close\r\n"
             "Content-Type: %s; charset=UTF-8\r\n\r\n",
             _GetContentType(srv->ini, uri));

    /* Prevent Path Traversal. */
    for (i = 0; i < n; i++) {
        if (uri[i] == '/') {
            if (((n - i) > 2) && (uri[i + 1] == '.') && ((uri[i + 2] == '.'))) {
                depth -= 1;
                if (depth < 0) break;
            } else if (((n - i) > 1) && (uri[i + 1] == '.'))
                continue;
            else
                depth += 1;
        }
    }

    if ((depth >= 0) && (uri[i - 1] != '/')) {
        /* Try to open and load the static file. */
        memcpy(path + strlen(srv->static_folder), uri, strlen(uri));
        LOGF("Path    : %s\n", path);
        fp = fopen(path, "rb");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            LOGF("Size    : %d\n", size);

            if (size < srv->maxsize) {
                /* Build HTTP OK header. */
                n = strlen(header);
                memcpy(res->_buf, header, n);
                i = n;

                /* Build HTTP body. */
                n = fread(res->_buf + i, 1, size, fp);
                i += n;
                LOGF("Read    : %ld\n", n);

                res->_index = i;

                found = 1;
            }
            fclose(fp);
        }
    }
    LOGF("Found   : %d\n", found);

    return found;
}

static void _NotFound(HTTPReqMessage *req, HTTPResMessage *res) {
    uint8_t n;
    char header[] = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";

    /* Build HTTP OK header. */
    n = strlen(header);
    memcpy(res->_buf, header, n);
    res->_index = n;
}

/* Dispatch an URI according to the route table. */
void Dispatch(HTTPServer *srv, HTTPReqMessage *req, HTTPResMessage *res) {
    uint16_t i;
    size_t n;
    char *req_uri = req->Header.URI;
    uint8_t found = 0;

    LOGF("Method  : %d\n", req->Header.Method);
    LOGF("URI     : %s\n", req->Header.URI);
    LOGF("Version : %s\n", req->Header.Version);

    /* Check the routes. */
    for (i = 0; i < routes_used; i++) {
        /* Compare method. */
        if (req->Header.Method == routes[i].method) {
            /* Compare URI. */
            n = strlen(routes[i].uri);
            if (memcmp(req_uri, routes[i].uri, n) == 0)
                found = 1;
            else
                continue;

            if ((found == 1) && ((req_uri[n] == '\0') || (req_uri[n] == '\?'))) {
                /* Found and dispatch the server application function. */
                routes[i].saf(srv, req, res);
                break;
            } else {
                found = 0;
            }
        }
    }

    /* Check static files. */
    if (found != 1) {
        // handle index file
        if (strcmp(req->Header.URI, "/") == 0) {
            char *index = (char *)ini_get(srv->ini, "httpd", "index");
            if (index) {
                req->Header.URI = index;
            }
        }
        found = _ReadStaticFiles(srv, req, res);
    }

    /* It is really not found. */
    if (found != 1) {
        _NotFound(req, res);
    }
}
