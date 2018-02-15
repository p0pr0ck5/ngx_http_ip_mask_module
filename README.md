# Name

ngx_http_ip_mask_module - Simple module to mask portions of the client IP address in access logs.

# Synopsis
```
http {
    log_format  main_mask  '$remote_addr - $remote_user [$time_local] "$request" '
        '$status $body_bytes_sent "$http_referer" '
        '"$http_user_agent" "$http_x_forwarded_for" "$remote_addr_masked"';

    access_log logs/access.log main_mask;

    server {
        ip_mask /24;
    }
}
```

```
poprocks@ubuntu:~/nginx$ cat logs/access.log
140.156.43.93 - - [15/Feb/2018:09:00:45 -0800] "GET / HTTP/1.1" 200 612 "-" "curl/7.47.0" "-" "140.156.43.0"
```

# Description

This module provides a new Nginx variable `$remote_addr_masked`. This variable contains a string representation of the client IP address, with the lowest bits masked as defined by the `ip_mask` directive. This module was written as a quick hack in response to http://mailman.nginx.org/pipermail/nginx/2018-February/055610.html.

Currently only IPv4 addresses are supported. If IPv6 or Unix sockets are used to connect to the server, the `$remote_addr_masked` variable is empty.

# Directives

## ip_mask

**syntax**: ip_mask */mask*

**default**: -

**context**: http, server, location

Defines the mask by which to produce the `$remote_addr_masked` variable. Address bits beyond the mask will be zeroed out.

This directive must be given in the form `/n`, where `n` is a mask size between 1 and 31, inclusive.

# License

Copyright 2018 Robert Paprocki.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
