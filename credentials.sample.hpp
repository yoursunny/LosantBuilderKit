#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

static const std::pair<const char*, const char*> WIFI_NETWORKS[] = {
  {"myWpaNetwork", "xxxxxxxx"},
  {"myOpenNetwork", nullptr}
};

const char* LOSANT_DEVICE_ID = "x";
const char* LOSANT_ACCESS_KEY = "x";
const char* LOSANT_ACCESS_SECRET = "x";

const char* NDN_ROUTER_HOST = "x";
const uint16_t NDN_ROUTER_PORT = 6363;

#endif // CREDENTIALS_HPP
