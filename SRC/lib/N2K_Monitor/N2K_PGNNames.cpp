/**
 * @file N2K_PGNNames.cpp
 * @brief PGN name lookup implementation for the N2K_Monitor module.
 *
 * This file provides human-readable name mapping for NMEA2000 Parameter Group
 * Numbers (PGNs). It contains a comprehensive lookup function that translates
 * numeric PGN identifiers into descriptive string names for display purposes.
 */

#include "N2K_Monitor.h"

/**
 * @brief Retrieves the human-readable name for a given NMEA2000 PGN.
 *
 * This function provides a comprehensive lookup table that maps NMEA2000
 * Parameter Group Numbers (PGNs) to their descriptive names. The function
 * covers the following categories of PGNs:
 *
 * - System PGNs (ISO acknowledgment, requests, address claiming, etc.)
 * - Navigation PGNs (heading, position, COG/SOG, waypoints, etc.)
 * - Engine PGNs (RPM, dynamic parameters, transmission, trip data)
 * - Electrical PGNs (battery status, charger, DC systems, fluid levels)
 * - Environmental PGNs (temperature, humidity, pressure, wind, depth)
 * - Windlass PGNs (anchor windlass control and monitoring)
 * - AIS PGNs (Class A/B position reports, static data, safety messages)
 * - Entertainment PGNs (marine entertainment systems)
 *
 * @param pgn The 32-bit NMEA2000 Parameter Group Number to look up.
 *
 * @return String containing the human-readable name for the PGN.
 *         Returns "PGN XXXXX" format for unknown/unrecognized PGNs,
 *         where XXXXX is the numeric PGN value.
 */
String N2K_Monitor::getPGNName(uint32_t pgn) {
    switch(pgn) {
        // System PGNs
        case 59392:  return "ISO Ack";
        case 59904:  return "ISO Request";
        case 60160:  return "ISO Transport";
        case 60416:  return "ISO TP Conn";
        case 60928:  return "ISO Addr Claim";
        case 65240:  return "Cmd Addr";
        case 126208: return "NMEA Req/Cmd";
        case 126464: return "PGN List";
        case 126992: return "System Time";
        case 126993: return "Heartbeat";
        case 126996: return "Product Info";
        case 126998: return "Config Info";

        // Navigation PGNs
        case 127233: return "MOB";
        case 127237: return "Heading Ctrl";
        case 127245: return "Rudder";
        case 127250: return "Vessel Heading";
        case 127251: return "Rate of Turn";
        case 127252: return "Heave";
        case 127257: return "Attitude";
        case 127258: return "Mag Variation";
        case 129025: return "Position Rapid";
        case 129026: return "COG/SOG Rapid";
        case 129029: return "GNSS Position";
        case 129033: return "Time & Date";
        case 129283: return "Cross Track Err";
        case 129284: return "Nav Route Info";
        case 129285: return "Nav Route/WP";
        case 129539: return "GNSS DOPs";
        case 129540: return "GNSS Sats";

        // Engine PGNs
        case 127488: return "Engine Rapid";
        case 127489: return "Engine Dynamic";
        case 127493: return "Transmission";
        case 127497: return "Trip Params";

        // Electrical PGNs
        case 127501: return "Binary Status";
        case 127502: return "Switch Ctrl";
        case 127505: return "Fluid Level";
        case 127506: return "DC Detail";
        case 127507: return "Charger Stat";
        case 127508: return "Battery Stat";
        case 127510: return "Charger Conf";
        case 127513: return "Battery Conf";
        case 127750: return "DC Converter";
        case 127751: return "DC Volt/Curr";

        // Environmental PGNs
        case 128000: return "Leeway";
        case 128259: return "Speed Water";
        case 128267: return "Water Depth";
        case 128275: return "Distance Log";
        case 130306: return "Wind Data";
        case 130310: return "Env Outside";
        case 130311: return "Env Params";
        case 130312: return "Temperature";
        case 130313: return "Humidity";
        case 130314: return "Pressure";
        case 130316: return "Temp Extended";
        case 130323: return "Meteo Station";
        case 130576: return "Trim Tabs";
        case 130577: return "Direction";

        // Windlass PGNs
        case 128776: return "Windlass Ctrl";
        case 128777: return "Windlass Op";
        case 128778: return "Windlass Mon";

        // AIS PGNs
        case 129038: return "AIS Class A";
        case 129039: return "AIS Class B";
        case 129040: return "AIS B Ext";
        case 129041: return "AIS AtoN";
        case 129793: return "AIS UTC/Date";
        case 129794: return "AIS Static A";
        case 129795: return "AIS Addressed";
        case 129796: return "AIS Ack";
        case 129797: return "AIS Binary";
        case 129798: return "AIS SAR";
        case 129799: return "Radio Freq";
        case 129800: return "AIS UTC Query";
        case 129801: return "AIS Slot Alloc";
        case 129802: return "AIS Safety";
        case 129803: return "AIS Interrog";
        case 129804: return "AIS Assign";
        case 129805: return "AIS Data Link";
        case 129806: return "AIS Channel";
        case 129807: return "AIS Group";
        case 129808: return "DSC Call Info";
        case 129809: return "AIS Static B";
        case 129810: return "AIS Static 24";

        // Entertainment
        case 130816: return "Entertainment";

        default:
            return "PGN " + String(pgn);
    }
}
