#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_ip_mask(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_ip_mask_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_ip_mask_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_ip_mask_add_variables(ngx_conf_t *cf);
static ngx_int_t
ngx_http_variable_remote_addr_mask(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);


typedef struct {
    ngx_uint_t   mask;
} ngx_http_ip_mask_loc_conf_t;


static ngx_command_t ngx_http_ip_mask_commands[] = {
    {
        ngx_string("ip_mask"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_ip_mask,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};


static ngx_http_module_t ngx_http_ip_mask_module_ctx = {
    ngx_http_ip_mask_add_variables,   /* preconfiguration */
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_ip_mask_create_loc_conf,
    ngx_http_ip_mask_merge_loc_conf
};


ngx_module_t ngx_http_ip_mask_module = {
    NGX_MODULE_V1,
    &ngx_http_ip_mask_module_ctx,
    ngx_http_ip_mask_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};


static ngx_http_variable_t ngx_http_ip_mask_module_variables[] = {
    {
        ngx_string("remote_addr_masked"),
        NULL,
        ngx_http_variable_remote_addr_mask,
        0,
        NGX_HTTP_VAR_NOCACHEABLE,
        0
    },
    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


/* get handler for the 'remote_addr_mask' var */
static ngx_int_t
ngx_http_variable_remote_addr_mask(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_ip_mask_loc_conf_t   *clcf;
    struct sockaddr_in             *sin;
    in_addr_t            client, masked;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_ip_mask_module);

    if (clcf->mask == NGX_CONF_UNSET_UINT) {
        goto not_found;
    }

    switch (r->connection->sockaddr->sa_family) {
        case AF_INET:
            sin = (struct sockaddr_in *)r->connection->sockaddr;
            client = sin->sin_addr.s_addr;
            masked = client & clcf->mask;

            v->valid = 1;
            v->no_cacheable = 0;
            v->not_found = 0;
            v->data = ngx_palloc(r->pool, NGX_INET_ADDRSTRLEN);
            v->len = ngx_inet_ntop(AF_INET, &masked, v->data, NGX_INET_ADDRSTRLEN);
            goto done;

        default:
            goto not_found;
    }

not_found:
    v->not_found = 1;

done:

    return NGX_OK;
}


/* command handler for the 'ip_mask' directive */
static char *
ngx_http_ip_mask(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_ip_mask_loc_conf_t  *c;
    c = conf;

    ngx_str_t  *value;
    ngx_int_t       i;
    ngx_uint_t   mask;
    u_char         *p;

    value = cf->args->elts;

    p = value[1].data;
    if (*p != '/') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "'ip_mask' must be a CIDR mask definition");
        return NGX_CONF_ERROR;
    }

    i = ngx_atoi(++p, value[1].len - 1);
    if (i <= 0 || i > 31) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid mask \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    mask = 0xFFFFFFFF >> (32 - i);
    mask = mask << (32 - i);

#if NGX_HAVE_LITTLE_ENDIAN
    c->mask = htonl((ngx_uint_t)mask);
#else
    c->mask = (ngx_uint_t)mask;
#endif

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_ip_mask_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t *var, *v;

    for (v = ngx_http_ip_mask_module_variables; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static void *
ngx_http_ip_mask_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_ip_mask_loc_conf_t  *conf;

    conf = ngx_palloc(cf->pool, sizeof(ngx_http_ip_mask_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->mask = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_http_ip_mask_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_ip_mask_loc_conf_t  *prev = parent;
    ngx_http_ip_mask_loc_conf_t  *conf = child;

    if (conf->mask == NGX_CONF_UNSET_UINT) {
        if (prev->mask == NGX_CONF_UNSET_UINT) {
            /* default */
            conf->mask = NGX_CONF_UNSET_UINT;
        } else {
            /* merge */
            conf->mask = prev->mask;
        }
    }

    return NGX_CONF_OK;
}
