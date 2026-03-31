//
// Created by KELs-truepunk on 31.03.2026.
//

#ifndef SERVER_HTTP_CODES_H
#define SERVER_HTTP_CODES_H
struct success_codes {
    char* ok;                       //200
    char* created;                  //201
    char* accepted;                 //202
    char* no_content;               //204
};
    struct redirection_codes {
        char* moved_permanently;        //301
        char* found;                    //302
        char* not_modified;             //304
        char* temporary_redirect;       //307
        char* permanent_redirect;       //308
    };
    struct client_error_codes {
        char* bad_request;              //400
        char* unauthorized;             //401
        char* payment_required;         //402
        char* forbidden;                //403
        char* not_found;                //404
        char* method_not_allowed;       //405
        char* conflict;                 //409
        char* gone;                     //410
        char* payload_too_large;        //413
        char* unprocessable_entity;     //422
        char* too_many_requests;        //429
    };
    struct server_error_codes {
        char* internal_server_error;    //500
        char* not_implemented;          //501
        char* bad_gateway;              //502
        char* service_unavailable;      //503
        char* gateway_timeout;          //504
    };

    typedef struct{
        struct success_codes       success;
        struct redirection_codes  redirection;
        struct client_error_codes client_error;
        struct server_error_codes server_error;
    } http_codes;

static const http_codes HTTP = {
    {
        "HTTP/1.1 200 OK",
        "HTTP/1.1 201 Created",
        "HTTP/1.1 202 Accepted",
        "HTTP/1.1 204 No Content"
    },
    {
        "HTTP/1.1 301 Moved Permanently",
        "HTTP/1.1 302 Found",
        "HTTP/1.1 304 Not Modified",
        "HTTP/1.1 307 Temporary Redirect",
        "HTTP/1.1 308 Permanent Redirect"
    },
    {
        "HTTP/1.1 400 Bad Request",
        "HTTP/1.1 401 Unauthorized",
        "HTTP/1.1 402 Payment Required",
        "HTTP/1.1 403 Forbidden",
        "HTTP/1.1 404 Not Found",
        "HTTP/1.1 405 Method Not Allowed",
        "HTTP/1.1 409 Conflict",
        "HTTP/1.1 410 Gone",
        "HTTP/1.1 413 Payload Too Large",
        "HTTP/1.1 422 Unprocessable Entity",
        "HTTP/1.1 429 Too Many Requests"
    },
    {
        "HTTP/1.1 500 Internal Server Error",
        "HTTP/1.1 501 Not Implemented",
        "HTTP/1.1 502 Bad Gateway",
        "HTTP/1.1 503 Service Unavailable",
        "HTTP/1.1 504 Gateway Timeout"
    }
};

#endif //SERVER_HTTP_CODES_H