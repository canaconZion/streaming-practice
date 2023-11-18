local ngx = require "ngx"
local resty_md5 = require "resty.md5"


local function genera_md5(in_url)
    local md5 = resty_md5:new()
    if not md5 then
        ngx.say("无法创建MD5对象")
        return
    end
    md5:update(in_url)
    local encrypted = md5:final()
    local md5_hex = require "resty.string".to_hex(encrypted)
    -- ngx.say("MD5加密结果：", md5_hex, "-----in url: ", in_url)
    return md5_hex
end

local function get_time()
    local current_time = os.time()
    local current_date = os.date("*t", current_time)
    current_date.min = 0
    current_date.sec = 0
    local current_whole_hour = os.time(current_date)
    return current_whole_hour
end

local client_ip = ngx.var.remote_addr
-- 打印请求的IP地址
local request_uri = ngx.var.request_uri
local filename = request_uri:match(".+/(.-)%?") or request_uri:match(".+/(.-)$")
-- local request_full = ngx.var.scheme .. "://" .. ngx.var.host .. ":" .. ngx.var.server_port.. ngx.var.uri
-- ngx.log(ngx.ERR, "-------请求url" .. ngx.var.scheme .. "://" .. ngx.var.host .. ngx.var.uri .. "\n")
if filename ~= nil then
    local file_extension = filename:match("^.+(%..+)$")
    local timestamp = tostring(get_time()).."000"
    -- local format_time = os.date("%Y%m%d%H", timestamp).."0000"
    -- ngx.log(ngx.ERR, "---------格式化时间："..format_time.."\n")
    if file_extension ~= nil then
        -- ngx.log(ngx.ERR, "文件后缀" .. file_extension)
        if file_extension == ".m3u8" then
            local args = ngx.req.get_uri_args()
            local args_ec = ngx.unescape_uri(ngx.encode_args(args))
            local rmd5 = ngx.md5(args_ec)
            -- local gmd5 = ngx.md5("http://192.168.1.41:81/vod/67niu_low/67niu.m3u8-2023080323-0-0-123456")
            local args_ec = ngx.encode_args(args)
            local request_full = "" .. ngx.var.uri
            local ser_gen_token = request_full .. "-" .. timestamp .. "-0-0-123456"
            local gen_md5 = ngx.md5(ser_gen_token)
            ngx.log(ngx.ERR, "------------md5--" .. gen_md5 .. "\n")
            ngx.log(ngx.ERR, "------------generate token-- " .. ser_gen_token .. "\n")
            ngx.log(ngx.ERR, "------------args_ec-- " .. args_ec .. "\n")
            genera_md5(ser_gen_token)
            if (args_ec ~= gen_md5) then
                ngx.exit(ngx.HTTP_FORBIDDEN)
            end
        end
    else
        ngx.exit(ngx.HTTP_FORBIDDEN)
    end
end
