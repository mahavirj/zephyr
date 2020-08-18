cmake_minimum_required(VERSION 3.8)

if (CONFIG_WIFI_ESP32)

#TODO: Additional WPA supplicant feature like Enterprise mode etc. are yet to be supported.
set(WPA_SUPPLICANT_SRCS "${ESP_IDF_PATH}/components/wpa_supplicant/port/os_xtensa.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/ap/ap_config.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/ap/ieee802_1x.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/ap/wpa_auth.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/ap/wpa_auth_ie.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/common/sae.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/common/wpa_common.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-ctr.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-siv.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha256-kdf.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-cbc.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-ccm.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-internal-dec.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-internal-enc.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-omac1.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-unwrap.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-wrap.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/aes-omac1.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha256-tlsprf.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/bignum.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/ccmp.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_mbedtls-bignum.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_mbedtls-ec.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_ops.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_internal-cipher.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_internal-modexp.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_internal-rsa.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/crypto_internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/des-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/dh_group5.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/dh_groups.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/md4-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/md5-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/md5.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/ms_funcs.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/rc4.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha1-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha1-pbkdf2.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha1.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha256-internal.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/crypto/sha256.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/esp_supplicant/esp_hostap.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/esp_supplicant/esp_wpa_main.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/esp_supplicant/esp_wpas_glue.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/esp_supplicant/esp_wpa3.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/rsn_supp/pmksa_cache.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/rsn_supp/wpa.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/rsn_supp/wpa_ie.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/base64.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/common.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/ext_password.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/uuid.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/wpabuf.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/wpa_debug.c"
                        "${ESP_IDF_PATH}/components/wpa_supplicant/src/utils/json.c"
                        )

zephyr_sources(
  src/esp_wifi_adapter.c
  src/stubs.c
  src/system_api.c
  src/esp_event.c
  ${ESP_IDF_PATH}/components/soc/src/esp32/rtc_clk.c
  ${ESP_IDF_PATH}/components/esp_timer/src/ets_timer_legacy.c
  ${ESP_IDF_PATH}/components/esp_timer/src/esp_timer.c
  ${ESP_IDF_PATH}/components/esp_timer/src/esp_timer_impl_frc_legacy.c
  ${ESP_IDF_PATH}/components/driver/periph_ctrl.c
  ${ESP_IDF_PATH}/components/esp_wifi/src/phy_init.c
  ${ESP_IDF_PATH}/components/log/log_noos.c
  ${ESP_IDF_PATH}/components/log/log.c
  ${WPA_SUPPLICANT_SRCS}
  src/esp_wifi_drv.c
  )

zephyr_include_directories(
  include
  )

zephyr_compile_definitions(
  __ets__
  ESP_PLATFORM
  ESP_SUPPLICANT
  IEEE8021X_EAPOL
  EAP_PEER_METHOD
  EAP_MSCHAPv2
  EAP_PEAP
  USE_WPA2_TASK
  ESPRESSIF_USE
  ESP32_WORKAROUND
  CONFIG_ECC
  CONFIG_IEEE80211W
  CONFIG_WPA3_SAE
  CONFIG_SHA256
  CONFIG_DPP
  )

zephyr_link_libraries(
  ## wifi
  core
  net80211
  core
  net80211
  pp
  coexist
  phy
  rtc
  mesh
  mbedTLS

  ## esp-idf wifi libs refer gcc libs symbols, so linked in libgcc
  gcc
  -L${ESP_IDF_PATH}/components/esp32
  -L${ESP_IDF_PATH}/components/esp_wifi/lib/esp32
  -L${ESP_IDF_PATH}/components/wpa_supplicant
  -Wl,--defsym,phy_printf=printf
  -Wl,--defsym,net80211_printf=printf
  -Wl,--defsym,coexist_printf=printf
  )

endif()
