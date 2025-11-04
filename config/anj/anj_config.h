/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay Lite LwM2M SDK
 * All rights reserved.
 *
 * Licensed under AVSystem Anjay Lite LwM2M Client SDK - Non-Commercial License.
 * See the attached LICENSE file for details.
 */

#ifndef ANJ_CONFIG_H
#define ANJ_CONFIG_H

/**
 * @file anj_config.h
 *
 * Anjay Lite library configuration.
 *
 * The preferred way to compile Anjay Lite is to use CMake, in which case this
 * file will be generated automatically by CMake.
 *
 * However, to provide compatibility with various build systems used especially
 * by embedded platforms, it is alternatively supported to compile Anjay by
 * other means, in which case this file will need to be provided manually.
 */

/******************************************************************************\
 * Input/Output buffer sizes
\******************************************************************************/
/**
 * Configures the size of the buffer for handling incoming messages.
 *
 * Default value: 1200
 * It affects statically allocated RAM.
 */
#define ANJ_IN_MSG_BUFFER_SIZE 1200

/**
 * Configures the size of the buffer for handling outgoing messages.
 *
 * Default value: 1200
 * It affects statically allocated RAM.
 */
#define ANJ_OUT_MSG_BUFFER_SIZE 1200

/**
 * Configures the size of the buffer for preparing outgoing messages payload.
 * Must be lower than @ref ANJ_OUT_MSG_BUFFER_SIZE to fit CoAP header in it as
 * well.
 *
 * Default value: 1024
 * It affects statically allocated RAM.
 */
#define ANJ_OUT_PAYLOAD_BUFFER_SIZE 1024

/******************************************************************************\
 * Data Model configuration
\******************************************************************************/
/**
 * Configures the maximum number of LwM2M Objects that Anjay Lite Data Model can
 * simultaneously hold.
 *
 * Default value: 10
 * It affects statically allocated RAM.
 */
#define ANJ_DM_MAX_OBJECTS_NUMBER 4

/**
 * Enable Composite Operations support (Read-Composite, Write-Composite)
 *
 * If not active, Anjay will respond with message code 5.01 Not Implemented to
 * any composite type request.
 * To enable Observe-Composite as well, see @ref ANJ_WITH_COMPOSITE_OPERATIONS .
 */
/* #undef ANJ_WITH_COMPOSITE_OPERATIONS */

/**
 * Configures the maximum number of entries (paths) in a composite read
 * operation.
 *
 * Default value: 5
 * This option is meaningful if @ref ANJ_WITH_COMPOSITE_OPERATIONS is enabled.
 */
/* #undef ANJ_DM_MAX_COMPOSITE_ENTRIES */

/******************************************************************************\
 * Device Object configuration
\******************************************************************************/
/**
 * Enable default, built-in implementation of /3 Device Object.
 *
 * If this flag is disabled, the user should provide an alternative, custom
 * implementation. It's mandated by the LwM2M Specification, but not necassary
 * to connect to a LwM2M Server.
 */
#define ANJ_WITH_DEFAULT_DEVICE_OBJ

/******************************************************************************\
 * Security Object configuration
\******************************************************************************/
/**
 * Enable default, built-in implementation of /0 Security Object.
 *
 * If this flag is disabled, the user must provide an alternative, custom
 * implementation.
 */
#define ANJ_WITH_DEFAULT_SECURITY_OBJ

/**
 * Configures the size of the buffer that holds Public Key or Identity
 * (/0/x/3 Resource).
 *
 * Default value: 255
 * It affects statically allocated RAM.
 */
#define ANJ_SEC_OBJ_MAX_PUBLIC_KEY_OR_IDENTITY_SIZE 64

/**
 * Configures the size of the buffer that holds Server Public Key
 * (/0/x/4 Resource).
 *
 * Default value: 255
 * It affects statically allocated RAM.
 */
#define ANJ_SEC_OBJ_MAX_SERVER_PUBLIC_KEY_SIZE 64

/**
 * Configures the size of the buffer that holds Secret Key
 * (/0/x/5 Resource).
 *
 * Default value: 255
 * It affects statically allocated RAM.
 */
#define ANJ_SEC_OBJ_MAX_SECRET_KEY_SIZE 64

/******************************************************************************\
 * Server Object configuration
\******************************************************************************/
/**
 * Enable default, built-in implementation of /1 Server Object.
 *
 * If this flag is disabled, the user must provide an alternative, custom
 * implementation.
 */
#define ANJ_WITH_DEFAULT_SERVER_OBJ

/******************************************************************************\
 * FOTA Object configuration
\******************************************************************************/
/**
 * Enable default, built-in implementation of /5 Firmware Update Object.
 */
/* #undef ANJ_WITH_DEFAULT_FOTA_OBJ */

/**
 * Enable PULL method of Firmware delivery in FOTA.
 * Allows LwM2M Server to write an URI from which FW package can be downloaded
 * to /5/0/1 Resource.
 *
 * This method actually requires the application to implement a downloader and
 * get the FW package with handlers provided in
 * @ref anj_dm_fw_update_handlers_t.
 */
/* #undef ANJ_FOTA_WITH_PULL_METHOD */

/**
 * Enable PUSH method of Firmware delivery in FOTA.
 * Allows LwM2M Server to write FW package with a series of LwM2M Write
 * operations to /5/0/0 Resource.
 */
/* #undef ANJ_FOTA_WITH_PUSH_METHOD */

/**
 * Enable FOTA with CoAP support.
 */
/* #undef ANJ_FOTA_WITH_COAP */

/**
 * Enable FOTA with CoAPs (CoAP over DTLS) support.
 */
/* #undef ANJ_FOTA_WITH_COAPS */

/**
 * Enable FOTA with HTTP support.
 */
/* #undef ANJ_FOTA_WITH_HTTP */

/**
 * Enable FOTA with HTTPS support.
 */
/* #undef ANJ_FOTA_WITH_HTTPS */

/**
 * Enable FOTA with CoAP over TCP support.
 */
/* #undef ANJ_FOTA_WITH_COAP_TCP */

/**
 * Enable FOTA with CoAP over TLS support.
 */
/* #undef ANJ_FOTA_WITH_COAPS_TCP */

/******************************************************************************\
 * CoAP Downloader configuration
\******************************************************************************/
/**
 * Enable CoAP Downloader Interface.
 *
 * This interface allows to download resources using CoAP protocol.
 * It is used along with FOTA Object, but can also be used independently.
 */
/* #undef ANJ_WITH_COAP_DOWNLOADER */

/**
 * Configures the maximum number of paths that can be used in CoAP Download request.
 *
 * Default value: 3
 * It affects statically allocated RAM.
 */
/* #define ANJ_COAP_DOWNLOADER_MAX_PATHS_NUMBER 3 */

/**
 * Configures the maximum size of a single request/response message in CoAP Downloader.
 * This is the size of the whole message, including CoAP header and payload.
 *
 * Default value: 1200
 * It affects statically allocated RAM.
 */
/* #undef ANJ_COAP_DOWNLOADER_MAX_MSG_SIZE 1200 */

/******************************************************************************\
 * Observe configuration
\******************************************************************************/
/**
 * Enable Observe, Notify, Cancel Observation and Write-Attributes operations in
 * Information Reporting Interface.
 */
#define ANJ_WITH_OBSERVE

/**
 * Enable Observe-Composite and Cancel Observation-composite operations in
 * Information Reporting Interface.
 *
 * Requires @ref ANJ_WITH_OBSERVE to be enabled.
 * Requires @ref ANJ_WITH_COMPOSITE_OPERATIONS to be enabled.
 */
/* #undef ANJ_WITH_OBSERVE_COMPOSITE */

/**
 * Configure the number of Observations that can be set on Anjay Lite at the
 * same time.
 *
 * Default value: 10
 * It affects statically allocated RAM.
 * If an incoming Observation can't be handled due to this limit, Anjay Lite
 * will respond with message code 5.00 Internal Server Error.
 */
#define ANJ_OBSERVE_MAX_OBSERVATIONS_NUMBER 10

/**
 * Configure the number of paths that have Observation-Class Attributes set with
 * Write-Attributes operation at the same time. Attributes set with Observe
 * operation (possible with LwM2M 1.2) are stored independently.
 *
 * Default value: 10
 * It affects statically allocated RAM.
 * If an incoming Write-Attributes can't be handled due to this limit, Anjay
 * Lite will respond with message code 5.00 Internal Server Error.
 */
#define ANJ_OBSERVE_MAX_WRITE_ATTRIBUTES_NUMBER 10

/******************************************************************************\
 * Bootstrap configuration
\******************************************************************************/
/**
 * Enable Bootstrap Interface.
 */
/* #undef ANJ_WITH_BOOTSTRAP */

/**
 * Enable Bootstrap-Discover operation support in Bootstrap Interface.
 */
/* #undef ANJ_WITH_BOOTSTRAP_DISCOVER */

/******************************************************************************\
 * Discover configuration
\******************************************************************************/
/**
 * Enable Discover operation support in Device Management and Service Enablement
 * Interface.
 */
#define ANJ_WITH_DISCOVER

/**
 * Enable reporting Observation-Class Attributes in response to Discover
 * operation.
 *
 * Requires @ref ANJ_WITH_DISCOVER and @ref ANJ_WITH_OBSERVE to be enabled.
 */
#define ANJ_WITH_DISCOVER_ATTR

/******************************************************************************\
 * Send configuration
\******************************************************************************/
/**
 * Enable LwM2M Send operation in Information Reporting Interface.
 */
/* #undef ANJ_WITH_LWM2M_SEND */

/**
 * Configures the number of Send messages that can be queued simultaneously.
 * They are never sent right away, so 1 is the minimum.
 *
 * Default value: 1
 * It affects statically allocated RAM.
 */
/* #undef ANJ_LWM2M_SEND_QUEUE_SIZE */

/******************************************************************************\
 * Compat layer configuration
\******************************************************************************/
/**
 * Enable implementation of Anjay Lite time API for POSIX-compilant platforms.
 *
 * If disabled, user must provide the compatibility layer.
 */
/* #undef ANJ_WITH_TIME_POSIX_COMPAT */

/**
 * Enable implementation of Anjay Lite Random Number Generator (RNG) API for
 * POSIX-compilant platforms.
 *
 * If disabled, user must provide the compatibility layer.
 */
/* #define ANJ_WITH_RNG_POSIX_COMPAT */

/**
 * Enable implementation of Anjay Lite socket API for POSIX-compilant plafroms.
 *
 * If disabled, user must provide the compatibility layer.
 */
/* #undef ANJ_WITH_SOCKET_POSIX_COMPAT */

/**
 * Enable default implementation of crypto storage API. Default implementation
 * is integrated with MbedTLS crypto library.
 *
 * If disabled, user must provide the compatibility layer.
 *
 * This option is meaningful only if @ref ANJ_WITH_EXTERNAL_CRYPTO_STORAGE is enabled.
 */
/* #undef ANJ_WITH_CRYPTO_STORAGE_DEFAULT */

/**
 * Enable communication using IPv4 protocol.
 *
 * Must be enabled, if
 * @link anj_net_socket_configuration_t.af_setting anj_t.net_socket_cfg.raw_socket_config.af_setting @endlink
 * forces IPv4 usage.
 * This option is meaningful if underlaying, platform sockets implementation
 * supports IPv4.
 */
#define ANJ_NET_WITH_IPV4

/**
 * Enable communication using IPv6 protocol.
 *
 * Must be enabled, if
 * @link anj_net_socket_configuration_t.af_setting anj_t.net_socket_cfg.raw_socket_config.af_setting @endlink
 * forces IPv6 usage.
 * This option is meaningful if underlaying, platform sockets implementation
 * supports IPv6.
 */
/* #undef ANJ_NET_WITH_IPV6 */

/**
 * Enable communication using UDP protocol.
 *
 * This option is meaningful if underlaying, platform sockets implementation
 * supports UDP.
 */
#define ANJ_NET_WITH_UDP

/**
 * Enable communication using TCP protocol.
 *
 * This option is meaningful if underlaying, platform sockets implementation
 * supports TCP.
 */
/* #undef ANJ_NET_WITH_TCP */

/**
 * Enable communication using DTLS protocol.
 *
 * This option is meaningful if underlaying, platform sockets implementation
 * supports DTLS.
 */
#define ANJ_NET_WITH_DTLS

/**
 * Enable support for MbedTLS library.
 *
 * This option enables default implementation of Anjay Lite DTLS sockets
 * using MbedTLS library.
 */
#define ANJ_WITH_MBEDTLS

/**
 * Configures the maximum length of PSK Identity (used in PSK-based DTLS).
 *
 * Default value: 64
 *
 * This option is meaningful only if @ref ANJ_WITH_MBEDTLS is enabled.
*/
#define ANJ_MBEDTLS_PSK_IDENTITY_MAX_LEN 128

/**
 * (D)TLS version supported by client, supported values are
 * MBEDTLS_SSL_VERSION_TLS1_2 and MBEDTLS_SSL_VERSION_TLS1_3.
 *
 * This option is meaningful only if @ref ANJ_WITH_MBEDTLS is enabled.
 */
#define ANJ_MBEDTLS_TLS_VERSION MBEDTLS_SSL_VERSION_TLS1_2

/**
 * Defines the allowed ciphersuites for default MbedTLS DTLS sockets implementation.
 *
 * You may specify multiple ciphersuites as a comma-separated list.
 * Example: <c>MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256,MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8</c>
 *
 * This option is meaningful only if @ref ANJ_WITH_MBEDTLS is enabled.
 *
 * @note Defaults to <c>MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8,MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8</c>
 *       if not defined.
 */
#define ANJ_MBEDTLS_ALLOWED_CIPHERSUITES MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8,MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8

/**
 * Defines the initial handshake timeout value in milliseconds.
 *
 * Default value: 1000
 *
 * This option is meaningful only if @ref ANJ_WITH_MBEDTLS is enabled.
 */
#define ANJ_MBEDTLS_HS_INITIAL_TIMEOUT_VALUE_MS 1000

/**
 * Defines the maximum handshake timeout value in milliseconds.
 *
 * Default value: 60000
 *
 * This option is meaningful only if @ref ANJ_WITH_MBEDTLS is enabled.
 */
#define ANJ_MBEDTLS_HS_MAXIMUM_TIMEOUT_VALUE_MS 60000

/******************************************************************************\
 * Security configuration
\******************************************************************************/
/**
 * Enable support for security communication in Anjay Lite.
 */
#define ANJ_WITH_SECURITY

/**
 * Enable support for certificates in Anjay Lite.
 *
 * If enabled, Anjay Lite will support certificate-based security mode
 * (/0/x/2 Resource in Security Object).
 *
 * @note This option will disable PSK-based security mode. NoSec mode
 *       will still be available.
 */
/* #undef ANJ_WITH_CERTIFICATES */

/**
 * Enable support for external crypto storage API.
 *
 * If enabled, Anjay Lite is allowed to use external crypto storage API
 * to store security information (e.g. certificates, keys).
 *
 * During bootstrap or persistence process, Anjay Lite will use external
 * crypto storage API to store all provided security information.
 */
/* #undef ANJ_WITH_EXTERNAL_CRYPTO_STORAGE */

/******************************************************************************\
 * Data Formats configuration
\******************************************************************************/
/**
 * Enable support for external data types: @ref ANJ_DATA_TYPE_EXTERNAL_BYTES
 * and @ref ANJ_DATA_TYPE_EXTERNAL_STRING. This allows to use external data
 * sources (e.g. files) as a source of data for the output payload.
 */
/* #undef ANJ_WITH_EXTERNAL_DATA */

/**
 * Enable CBOR Content Format (application/cbor, numerical-value 60) encoder and
 * decoder.
 */
#define ANJ_WITH_CBOR

/**
 * Enable support for decoding CBOR Tag 4 - Decimal Fractions.
 */
#define ANJ_WITH_CBOR_DECODE_DECIMAL_FRACTIONS

/**
 * Enable support for decoding 16-bit IEEE 754 half-precision format
 * ("half floats") in CBOR.
 */
#define ANJ_WITH_CBOR_DECODE_HALF_FLOAT

/**
 * Enable support for decoding CBOR indefinite-length byte strings and arrays.
 */
#define ANJ_WITH_CBOR_DECODE_INDEFINITE_BYTES

/**
 * Enable support for docoding CBOR time values represented as text strings.
 */
#define ANJ_WITH_CBOR_DECODE_STRING_TIME

/**
 * Enable LwM2M CBOR Content Format (application/vnd.oma.lwm2m+cbor,
 * numerical-value 11544) encoder and decoder.
 *
 * Requires @ref ANJ_WITH_LWM2M12 to be enabled.
 * At least one of @ref ANJ_WITH_SENML_CBOR or @ref ANJ_WITH_LWM2M_CBOR
 * must enabled for payload encoding, because other content formats can't encode
 * multiple records in one message.
 */
/* #undef ANJ_WITH_LWM2M_CBOR */

/**
 * Enable SenML CBOR Content Format (application/senml+cbor, numerical-value
 * 112) and SenML-ETCH CBOR (application/senml-etch+cbor, numerical-value 322)
 * encoder and decoder.
 *
 * At least one of @ref ANJ_WITH_SENML_CBOR or @ref ANJ_WITH_LWM2M_CBOR
 * must enabled for payload encoding, because other content formats can't encode
 * multiple records in one message.
 */
#define ANJ_WITH_SENML_CBOR

/**
 * Enable Plaintext Content Format (text/plain , numerical-value 0) encoder and
 * decoder.
 */
#define ANJ_WITH_PLAINTEXT

/**
 * Enable Opaque Content Format (application/octet-stream , numerical-value 42)
 * encoder and decoder.
 */
#define ANJ_WITH_OPAQUE

/**
 * Enable TLV Content Format (application/vnd.oma.lwm2m+tlv, numerical-value
 * 11542) decoder.
 *
 * @note Encoder is not implemented.
 */
#define ANJ_WITH_TLV

/******************************************************************************\
 * CoAP configuration
\******************************************************************************/
/**
 * Enable communication with LwM2M Server using CoAP over UDP.
 *
 * Requires @ref ANJ_NET_WITH_UDP to be enabled.
 */
#define ANJ_COAP_WITH_UDP

/**
 * Enable communication with LwM2M Server using CoAP over TCP.
 *
 * Requires @ref ANJ_NET_WITH_TCP to be enabled.
 */
/* #undef ANJ_COAP_WITH_TCP */

/**
 * Configures the maximum allowed number of Options present in CoAP header, in a
 * single frame.
 *
 * Default value: 15
 * If an incomming message has more options, Anjay Lite will not process it or
 * respond to it.
 */
#define ANJ_COAP_MAX_OPTIONS_NUMBER 15

/**
 * Configures the maximum length of a single Uri-Query CoAP Option related to
 * Register, Request Bootstrap, Attribute setting with Write-Attributes or
 * Observe operation.
 * This option directly limits Client Endpoint name length.
 *
 * Default value: 40
 * Calculated as "attr_name" + "=" + "attr_value", e.g.:
 * - "gt=700.12" -> 9
 * - "ep=anjay_lite_client" -> 20
 */
#define ANJ_COAP_MAX_ATTR_OPTION_SIZE 40

/**
 * Configures the number of Location-Path options that can be set in the LwM2M
 * Server's response to a Register message, in the CoAP header. Register
 * response will be rejected if Location-Paths count in it exeeds this value.
 *
 * Default value: 2
 * It affects statically allocated RAM.
 */
#define ANJ_COAP_MAX_LOCATION_PATHS_NUMBER 2

/**
 * Configures the maximum size of each Location-Path option in the LwM2M
 * Server's response to a Register message, in the CoAP header.
 *
 * Default value: 40
 * It affects statically allocated RAM.
 */
#define ANJ_COAP_MAX_LOCATION_PATH_SIZE 40

/**
 * Enables respones caching mechanism.
 * When eneabled, Anjay Lite caches responses to ensure that duplicate messages
 * (e.g. due to retransmissions or retries) are processed exactly once. This
 * guarantees idempotent behavior for operations with side effects.
 *
 * It affects statically allocated RAM.
 */
/* #undef ANJ_WITH_CACHE */

/**
 * Configures the number of cached responses.
 *
 * Default value: 10
 * It affects statically allocated RAM.
 */
/* #undef ANJ_CACHE_ENTRIES_NUMBER 10 */

/******************************************************************************\
 * Logger configuration
\******************************************************************************/
/**
 * Enable logger in full mode. The header for log messages contains:
 * - module name
 * - level of log message
 * - name of source file the log is called from
 * - line number of the log call
 */
#define ANJ_LOG_FULL

/**
 * Enable logger in alternative mode. This should be defined to path to header
 * file that specifes an alternative implementation of
 * @c ANJ_LOG_HANDLER_IMPL_MACRO with arguments:
 * - Module: module name, in the same form as in @ref anj_log() calls
 *   (stringifiable identifier).
 * - Level: identifier, one of: @c L_TRACE, @c L_DEBUG, @c L_INFO, @c L_WARNING,
 *   @c L_ERROR, @c L_MUTE. @c L_MUTE must not generate a message.
 * - format string as in <c>printf()</c> function and arguments
 *
 * This header file is included by by @ref anj/log.h. This is useful to
 * completely replace log calls with a custom definition, e.g. to emit log calls
 * compatible with other logging libraries, such as the built-in system in
 * Zephyr.
 */
/* #undef ANJ_LOG_ALT_IMPL_HEADER */

/**
 * Use <c>vsnprintf()</c> to format log messages. This is the preferred option
 * if the application already uses other functions from <c>printf()</c> family.
 *
 * @note This option affects only the builds that use built-in log handler,
 *       i.e. @ref ANJ_LOG_FULL.
 */
#define ANJ_LOG_FORMATTER_PRINTF

/**
 * Size of the buffer used to format log messages. If the message is larger than
 * the buffer, it will be truncated.
 *
 * Caution: the buffer is allocated on stack when calling @ref anj_log() macro.
 *
 * @note This option affects only the builds that use built-in log handler,
 *       i.e. @ref ANJ_LOG_FULL.
 */
#define ANJ_LOG_FORMATTER_BUF_SIZE 512

/**
 * Output log messages on stderr.
 *
 * @note This option affects only the builds that use built-in log handler,
 *       i.e. @ref ANJ_LOG_FULL.
 */
/* #undef ANJ_LOG_HANDLER_OUTPUT_STDERR */

/**
 * Use alternative implementation of output function for log handler. End user
 * must implement the @ref anj_log_handler_output() function, declared in
 * @ref anj/compat/log_impl_decls.h.
 */
#define ANJ_LOG_HANDLER_OUTPUT_ALT

/**
 * If enabled, @ref ANJ_LOG_DISPOSABLE() will evaluate to <c>" "</c>.
 */
/* #undef ANJ_LOG_STRIP_CONSTANTS */

/**
 * Default log level. Any log statements with a level lower than this will be
 * excluded from the build.
 *
 * @note Defaults to @c L_INFO if not defined.
 *
 * @note Log level may be overridden for specific modules, see @ref
 *       ANJ_LOG_FILTERING_CONFIG_HEADER.
 */
#define ANJ_LOG_LEVEL_DEFAULT L_TRACE

/**
 * Override log level for specific modules. This should be defined to path to
 * header file with declarations in form:
 * <c>ANJ_LOG_LEVEL_FOR_MODULE_Module</c>, where <c>Module</c> is the name of
 * the module, in the same form as in @ref anj_log() calls (stringifiable
 * identifier).
 *
 * <example>
 * @code
 * // Enable trace logs for exchange module
 * #define ANJ_LOG_LEVEL_FOR_MODULE_exchange L_TRACE
 *
 * // Disable logs for observe module
 * #define ANJ_LOG_LEVEL_FOR_MODULE_observe L_MUTED
 * @endcode
 * </example>
 */
/* #undef ANJ_LOG_FILTERING_CONFIG_HEADER */

/******************************************************************************\
 * Persistence configuration
\******************************************************************************/
/**
 * Enable Persistence support.
 *
 * When enabled, Anjay Lite can store and restore its state e.g. state of
 * Security and Server objects.
 */
/* #undef ANJ_WITH_PERSISTENCE */

/******************************************************************************\
 * Other configuration
\******************************************************************************/
/**
 * Enable features introduced in LwM2M protocol version 1.2.
 */
/* #undef ANJ_WITH_LWM2M12 */

/**
 * Enables custom convertion functions implementation that do not require
 * <c>sprintf()</c> and <c>sscanf()</c> in Anjay Lite for string<->number
 * convertions.
 *
 * Useful on constrained platforms which do not use these functions.
 */
#define ANJ_WITH_CUSTOM_CONVERSION_FUNCTIONS

/**
 * Configures the numerical converters to treat the platform as big endian.
 * Disabling this option will make Anjay Lite treat the platform as little
 * endian.
 */
/* #undef ANJ_PLATFORM_BIG_ENDIAN */

#endif // ANJ_CONFIG_H
