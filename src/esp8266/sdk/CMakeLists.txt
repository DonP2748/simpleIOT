set(COMPONENT_SRCS 
    	"flash/flash.c"
    	"mqtt/mqtt.c"
    	"ota/ota.c"
    	"security/aes.c"
    	"sntp_server/sntp.c"
    	"tcp/tcp.c"
	"timer/systimer.c"
	"util/event.c"
    	"wifi/wifi.c"
    ) 

set(COMPONENT_ADD_INCLUDEDIRS 
	"." 
	"flash"
	"mqtt" 
	"ota"
	"security"
	"sntp_server"
	"tcp" 
	"timer"
	"util"
	"wifi"
	)
set(COMPONENT_PRIV_REQUIRES  esp_http_server esp_http_client esp_https_ota  nvs_flash app_update tcpip_adapter mqtt json spiffs mbedtls wpa_supplicant)

set(COMPONENT_EMBED_TXTFILES ../server_certs/ca_cert.pem)

register_component()
