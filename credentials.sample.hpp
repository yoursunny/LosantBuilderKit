#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

static const std::pair<const char*, const char*> WIFI_NETWORKS[] = {
  {"myWpaNetwork", "xxxxxxxx"},
  {"myOpenNetwork", nullptr}
};

const char* LOSANT_DEVICE_ID = "x";
const char* LOSANT_ACCESS_KEY = "x";
const char* LOSANT_ACCESS_SECRET = "x";

const char* DYNDNS_SERVER = "dyn.dns.he.net";
const char* DYNDNS_HOST = "dyn.example.net";
const char* DYNDNS_AUTH = "dXNlcjpwYXNzd29yZA==";

const char* NDN_ROUTER_HOST = "x";
const uint16_t NDN_ROUTER_PORT = 6363;
const uint8_t NDN_HMAC_KEY[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
char* NDN_OUTPING_PREFIX = "/example/ping"; // must be mutable
char* NDN_INPING_PREFIX = "/example/user/temperature-sensor/ping"; // must be mutable

const char* NDNPREFIXREG_HTTPHOST = "example.com";
const uint16_t NDNPREFIXREG_HTTPPORT = 8266;
const char* NDNPREFIXREG_HTTPURI = "/ndn-prefix-reg.php";

#endif // CREDENTIALS_HPP
