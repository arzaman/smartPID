/*
 * WiFi.h
 *
 * Wi-Fi communication manager
 */

#ifndef WIFI_H_
#define WIFI_H_

#include <AsyncClient.h>
#include <IPAddress.h>
#include <SparkFunESP8266Server.h>
#include <stdint.h>

#define WIFI_OWN_PWD	"smartpid!"

class WiFi {
public:
	static const int BAUD_RATE_DEFAULT = 57600;
	static const int SSID_MAX_LEN = 32;
	static const int PWD_MAX_LEN = 63;
	enum {
		MODE_OFF,
		MODE_STA,
		MODE_AP,

		/* Automatic mode means station mode if parameters for connecting to an
		 * access point are configured correctly, access point mode otherwise.
		 */
		MODE_AUTO,
	};
	enum {
		STATUS_NA,	/* Wi-Fi not available */
		STATUS_OFF,
		STATUS_STA,
		STATUS_AP,
	} status;
	enum {
		STATUS_DISCONNECTED,
		STATUS_CONNECTED,
	} staStatus;
	AsyncClient *client;
	ESP8266Server *server;
	WiFi();

	/* Initialize the Wi-Fi interface
	 * Parameters:
	 * - baud: baud rate for communication with Wi-Fi module
	 * Returns: WiFi interface status (STATUS_* enum values)
	 */
	int init(uint32_t baud);

	/* Configure the Wi-Fi interface
	 * Parameters:
	 * - mode: Wi-Fi mode (MODE_* enum values)
	 * - ssid: SSID of the access point to connect to (used only in MODE_STA and
	 *   MODE_AUTO
	 * - pwd: password of the access point to connect to (used only in MODE_STA
	 *   and MODE_AUTO
	 * Returns: none
	 */
	void config(int mode, const char *ssid, const char *pwd);

	/* Configure the Wi-Fi interface in station mode and connect to an access
	 * point
	 * Parameters:
	 * - ssid: SSID of the access point to connect to
	 * - pwd: password of the access point to connect to
	 * Returns: none
	 */
	void configSta(const char *ssid, const char *pwd);

	/* Configure the Wi-Fi interface in access point mode
	 * The SSID of the access point is built from the device serial number,
	 * while the password is hard-coded.
	 * Parameters: none
	 * Returns: none
	 */
	void configAP(void);

	/* Get the IP address of the Wi-Fi interface
	 * Parameters: none
	 * Returns: IP address of the Wi-Fi interface (INADDR_NONE if no IP address
	 *          has been assigned to the interfaces)
	 */
	IPAddress getIP();

	/* Get the SSID of the Wi-Fi interface.
	 * If the interface is in station mode, this method returns the SSID of the
	 * access point the interface is connected (or trying to connect) to; if the
	 * interface is in access point mode, this method returns the interface's
	 * own SSID.
	 * Parameters:
	 * - buf: Buffer where the returned SSID is stored
	 * Returns: 0 if the interface is in either station or access point mode, a
	 * negative value otherwise (in which case nothing is copied to the input
	 * buffer)
	 */
	int getSSID(char *buf);

	/* Get the password of the Wi-Fi interface.
	 * If the interface is in station mode, this method returns the password of
	 * the access point the interface is connected (or trying to connect) to; if
	 * the interface is in access point mode, this method returns the
	 * interface's own password.
	 * Parameters:
	 * - buf: Buffer where the returned password is stored
	 * Returns: 0 if the interface is in either station or access point mode, a
	 * negative value otherwise (in which case nothing is copied to the input
	 * buffer)
	 */
	int getPwd(char *buf);

	/* Set the port at which the TCP server will be listening
	 * This method can be called to customize the server port before starting
	 * the server with a call to ESP8266Server::begin() or
	 * ESP8266Server::begin_check().
	 * Parameters:
	 * - port: TPC port number
	 * Returns: none
	 */
	void setServerPort(uint16_t port);

	/* Reset the Wi-Fi interface
	 * Calling this method is equivalent to doing a hardware reset on the Wi-Fi
	 * module.
	 * Parameters: none
	 * Returns: none
	 */
	void reset();

	/* Method to be called periodically, that manages the Wi-Fi interface
	 * This method is called by the WiFiTick() function.
	 * Parameters: none
	 * Returns: none
	 */
	void tick();

private:
	char own_ssid[SSID_MAX_LEN + 1];	/* null-terminated string */
};

extern WiFi wiFi;

/* Function to be called periodically, that manages the Wi-Fi interface
 * Parameters: none
 * Returns: none
 */
void WiFiTick();

#endif /* WIFI_H_ */
