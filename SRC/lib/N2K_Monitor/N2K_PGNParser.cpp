/**
 * @file N2K_PGNParser.cpp
 * @brief Comprehensive PGN data parsing implementation for the N2K_Monitor module.
 *
 * This file provides the main parsing logic for NMEA2000 messages, extracting
 * meaningful field data from raw PGN payloads. It supports a wide range of
 * standard NMEA2000 PGNs covering navigation, engine, electrical, environmental,
 * and AIS data types.
 */

#include "N2K_Monitor.h"

/**
 * @brief Parses an NMEA2000 message and extracts human-readable field data.
 *
 * This function is the main entry point for PGN data parsing. It takes a raw
 * NMEA2000 message and populates a PGNData structure with parsed field values,
 * converting raw binary data into human-readable formats with appropriate units.
 *
 * The function handles the following PGN categories:
 *
 * **Navigation PGNs:**
 * - 127250: Vessel Heading (heading, deviation, variation, reference)
 * - 127251: Rate of Turn (rotation rate in degrees/minute)
 * - 127257: Attitude (yaw, pitch, roll)
 * - 127258: Magnetic Variation (variation and source)
 * - 127245: Rudder (position and instance)
 * - 129025: Position Rapid Update (latitude, longitude)
 * - 129026: COG/SOG Rapid Update (course and speed over ground)
 * - 129029: GNSS Position Data (full GPS fix with satellites and DOP)
 * - 129033: Time and Date (UTC time with local offset)
 * - 129283: Cross Track Error (XTE and navigation status)
 * - 129284: Navigation Route/WP Information (distance, bearing, VMG)
 * - 129539: GNSS DOPs (HDOP, VDOP, TDOP, mode)
 * - 129540: GNSS Satellites in View (satellite count and details)
 *
 * **Engine PGNs:**
 * - 127488: Engine Parameters Rapid Update (RPM, boost, tilt/trim)
 * - 127489: Engine Parameters Dynamic (oil, coolant, alternator, fuel)
 * - 127493: Transmission Parameters (gear, oil pressure/temp)
 * - 127497: Trip Fuel Parameters (trip fuel, average rate)
 *
 * **Electrical PGNs:**
 * - 127501: Binary Switch Bank Status (8-switch states)
 * - 127505: Fluid Level (tank type, level percentage, capacity)
 * - 127506: DC Detailed Status (SOC, health, capacity)
 * - 127507: Charger Status (charge state, mode, enabled)
 * - 127508: Battery Status (voltage, current, temperature)
 * - 127513: Battery Configuration (type, chemistry, capacity)
 * - 127751: DC Voltage/Current (instance, voltage, current)
 *
 * **Environmental PGNs:**
 * - 128000: Leeway (leeway angle)
 * - 128259: Speed Through Water (STW and SOG)
 * - 128267: Water Depth (depth, offset, range)
 * - 128275: Distance Log (total log and trip log)
 * - 130306: Wind Data (speed, angle, reference type)
 * - 130310: Outside Environmental Parameters (water/air temp, pressure)
 * - 130311: Environmental Parameters (temp, humidity, pressure)
 * - 130312: Temperature (instance, actual, set point)
 * - 130313: Humidity (instance, actual, set point)
 * - 130314: Pressure (instance, pressure value)
 * - 130316: Temperature Extended Range (with source type)
 * - 130576: Trim Tab Status (port and starboard positions)
 * - 130577: Direction Data (COG, SOG, heading, set, drift)
 *
 * **System PGNs:**
 * - 60928: ISO Address Claim (NAME field parsing: manufacturer, class, etc.)
 * - 126992: System Time (days since epoch, UTC time)
 * - 126993: Heartbeat (interval, sequence, controller states)
 * - 126996: Product Information (version, product code, model, software)
 * - 127252: Heave (heave and delay values)
 *
 * **AIS PGNs:**
 * - 129038: AIS Class A Position Report (MMSI, position, COG, SOG)
 * - 129039: AIS Class B Position Report (MMSI, position, COG, SOG)
 * - 129794: AIS Class A Static Data (MMSI, name, callsign, dimensions)
 * - 129809: AIS Class B CS Static Data Part A (MMSI, name)
 * - 129810: AIS Class B CS Static Data Part B (MMSI, callsign, dimensions)
 *
 * For unrecognized PGNs, the function displays raw hexadecimal data bytes.
 *
 * @param N2kMsg Reference to the incoming NMEA2000 message to parse.
 *               Contains PGN number, data length, source address, and raw data.
 * @param pgnData Reference to the PGNData structure to populate with parsed results.
 *                On return, contains:
 *                - pgn: The PGN number
 *                - name: Human-readable PGN name
 *                - lastUpdate: Timestamp of when this was parsed
 *                - fields: Vector of parsed field name/value/unit tuples
 *                - rawData: Copy of the raw message data
 *                - dataLen: Length of the raw data
 *
 * @note Unit conversions are applied automatically:
 *       - Angles: Radians to degrees
 *       - Temperatures: Kelvin to Celsius
 *       - Speeds: m/s to knots
 *       - Pressures: Pa to kPa or mbar as appropriate
 *       - Distances: meters to nautical miles where appropriate
 *
 * @note Fields with N/A (Not Available) values as defined by NMEA2000 are
 *       typically omitted from the output to avoid cluttering the display.
 *
 * @see getPGNName() for the PGN name lookup function.
 * @see PGNData structure for the output data format.
 */
void N2K_Monitor::parsePGNData(const tN2kMsg &N2kMsg, PGNData &pgnData) {
    pgnData.pgn = N2kMsg.PGN;
    pgnData.name = getPGNName(N2kMsg.PGN);
    pgnData.lastUpdate = millis();
    pgnData.fields.clear();

    // Store raw data for potential re-parsing
    pgnData.dataLen = min((int)N2kMsg.DataLen, 256);
    memcpy(pgnData.rawData, N2kMsg.Data, pgnData.dataLen);

    // Parse based on PGN
    switch(N2kMsg.PGN) {
        case 127250: { // Vessel Heading
            unsigned char SID;
            double Heading, Deviation, Variation;
            tN2kHeadingReference ref;
            if(ParseN2kHeading(N2kMsg, SID, Heading, Deviation, Variation, ref)) {
                pgnData.fields.push_back({"Heading", String(RadToDeg(Heading), 1), "deg"});
                if(!N2kIsNA(Deviation)) pgnData.fields.push_back({"Deviation", String(RadToDeg(Deviation), 1), "deg"});
                if(!N2kIsNA(Variation)) pgnData.fields.push_back({"Variation", String(RadToDeg(Variation), 1), "deg"});
                pgnData.fields.push_back({"Reference", ref == N2khr_magnetic ? "Mag" : "True", ""});
            }
            break;
        }

        case 127251: { // Rate of Turn
            unsigned char SID;
            double RateOfTurn;
            if(ParseN2kRateOfTurn(N2kMsg, SID, RateOfTurn)) {
                pgnData.fields.push_back({"Rate", String(RadToDeg(RateOfTurn) * 60, 2), "deg/min"});
            }
            break;
        }

        case 127257: { // Attitude
            unsigned char SID;
            double Yaw, Pitch, Roll;
            if(ParseN2kAttitude(N2kMsg, SID, Yaw, Pitch, Roll)) {
                if(!N2kIsNA(Yaw)) pgnData.fields.push_back({"Yaw", String(RadToDeg(Yaw), 1), "deg"});
                if(!N2kIsNA(Pitch)) pgnData.fields.push_back({"Pitch", String(RadToDeg(Pitch), 1), "deg"});
                if(!N2kIsNA(Roll)) pgnData.fields.push_back({"Roll", String(RadToDeg(Roll), 1), "deg"});
            }
            break;
        }

        case 127258: { // Magnetic Variation
            unsigned char SID;
            tN2kMagneticVariation Source;
            uint16_t DaysSince1970;
            double Variation;
            if(ParseN2kMagneticVariation(N2kMsg, SID, Source, DaysSince1970, Variation)) {
                if(!N2kIsNA(Variation)) pgnData.fields.push_back({"Var", String(RadToDeg(Variation), 1), "deg"});
                const char* sourceNames[] = {"Manual", "Chart", "Table", "Calc", "WMM2000", "WMM2005", "WMM2010", "WMM2015", "WMM2020"};
                if((int)Source < 9) {
                    pgnData.fields.push_back({"Source", sourceNames[(int)Source], ""});
                }
            }
            break;
        }

        case 127245: { // Rudder
            double RudderPosition;
            unsigned char Instance;
            tN2kRudderDirectionOrder RudderDirectionOrder;
            double AngleOrder;
            if(ParseN2kRudder(N2kMsg, RudderPosition, Instance, RudderDirectionOrder, AngleOrder)) {
                pgnData.fields.push_back({"Position", String(RadToDeg(RudderPosition), 1), "deg"});
                pgnData.fields.push_back({"Instance", String(Instance), ""});
            }
            break;
        }

        case 127488: { // Engine Rapid
            unsigned char EngineInstance;
            double EngineSpeed, EngineBoostPressure;
            int8_t EngineTiltTrim;
            if(ParseN2kEngineParamRapid(N2kMsg, EngineInstance, EngineSpeed, EngineBoostPressure, EngineTiltTrim)) {
                pgnData.fields.push_back({"Instance", String(EngineInstance), ""});
                pgnData.fields.push_back({"RPM", String(EngineSpeed, 0), "rpm"});
                if(!N2kIsNA(EngineBoostPressure)) pgnData.fields.push_back({"Boost", String(EngineBoostPressure/100, 1), "kPa"});
                if(EngineTiltTrim != N2kInt8NA) pgnData.fields.push_back({"Tilt/Trim", String(EngineTiltTrim), "%"});
            }
            break;
        }

        case 127489: { // Engine Dynamic
            unsigned char EngineInstance;
            double EngineOilPress, EngineOilTemp, EngineCoolantTemp, AlternatorVoltage;
            double FuelRate, EngineHours, EngineCoolantPress, EngineFuelPress;
            int8_t EngineLoad, EngineTorque;
            tN2kEngineDiscreteStatus1 Status1;
            tN2kEngineDiscreteStatus2 Status2;
            if(ParseN2kEngineDynamicParam(N2kMsg, EngineInstance, EngineOilPress, EngineOilTemp,
                                          EngineCoolantTemp, AlternatorVoltage, FuelRate, EngineHours,
                                          EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                                          Status1, Status2)) {
                pgnData.fields.push_back({"Instance", String(EngineInstance), ""});
                if(!N2kIsNA(EngineOilPress)) pgnData.fields.push_back({"Oil Press", String(EngineOilPress/1000, 1), "kPa"});
                if(!N2kIsNA(EngineOilTemp)) pgnData.fields.push_back({"Oil Temp", String(KelvinToC(EngineOilTemp), 1), "C"});
                if(!N2kIsNA(EngineCoolantTemp)) pgnData.fields.push_back({"Coolant", String(KelvinToC(EngineCoolantTemp), 1), "C"});
                if(!N2kIsNA(AlternatorVoltage)) pgnData.fields.push_back({"Alt Volt", String(AlternatorVoltage, 1), "V"});
                if(!N2kIsNA(FuelRate)) pgnData.fields.push_back({"Fuel Rate", String(FuelRate * 3600, 1), "L/h"});
                if(!N2kIsNA(EngineHours)) pgnData.fields.push_back({"Hours", String(EngineHours / 3600, 1), "h"});
                if(EngineLoad != N2kInt8NA) pgnData.fields.push_back({"Load", String(EngineLoad), "%"});
            }
            break;
        }

        case 127508: { // Battery Status
            unsigned char BatteryInstance;
            double BatteryVoltage, BatteryCurrent, BatteryTemperature;
            unsigned char SID;
            if(ParseN2kDCBatStatus(N2kMsg, BatteryInstance, BatteryVoltage, BatteryCurrent, BatteryTemperature, SID)) {
                pgnData.fields.push_back({"Instance", String(BatteryInstance), ""});
                if(!N2kIsNA(BatteryVoltage)) pgnData.fields.push_back({"Voltage", String(BatteryVoltage, 2), "V"});
                if(!N2kIsNA(BatteryCurrent)) pgnData.fields.push_back({"Current", String(BatteryCurrent, 1), "A"});
                if(!N2kIsNA(BatteryTemperature)) pgnData.fields.push_back({"Temp", String(KelvinToC(BatteryTemperature), 1), "C"});
            }
            break;
        }

        case 127505: { // Fluid Level
            unsigned char Instance;
            tN2kFluidType FluidType;
            double Level, Capacity;
            if(ParseN2kFluidLevel(N2kMsg, Instance, FluidType, Level, Capacity)) {
                pgnData.fields.push_back({"Instance", String(Instance), ""});
                const char* fluidNames[] = {"Fuel", "Water", "Gray", "LiveWell", "Oil", "Black", "Gasoline", "Error", "Unavail"};
                if((int)FluidType < 9) pgnData.fields.push_back({"Type", fluidNames[(int)FluidType], ""});
                pgnData.fields.push_back({"Level", String(Level, 1), "%"});
                if(!N2kIsNA(Capacity)) pgnData.fields.push_back({"Capacity", String(Capacity, 0), "L"});
            }
            break;
        }

        case 128259: { // Speed Water
            unsigned char SID;
            double WaterReferenced, GroundReferenced;
            tN2kSpeedWaterReferenceType SWRT;
            if(ParseN2kBoatSpeed(N2kMsg, SID, WaterReferenced, GroundReferenced, SWRT)) {
                if(!N2kIsNA(WaterReferenced)) pgnData.fields.push_back({"STW", String(msToKnots(WaterReferenced), 2), "kn"});
                if(!N2kIsNA(GroundReferenced)) pgnData.fields.push_back({"SOG", String(msToKnots(GroundReferenced), 2), "kn"});
            }
            break;
        }

        case 128267: { // Water Depth
            unsigned char SID;
            double DepthBelowTransducer, Offset, Range;
            if(ParseN2kWaterDepth(N2kMsg, SID, DepthBelowTransducer, Offset, Range)) {
                pgnData.fields.push_back({"Depth", String(DepthBelowTransducer, 1), "m"});
                if(!N2kIsNA(Offset)) pgnData.fields.push_back({"Offset", String(Offset, 2), "m"});
                if(!N2kIsNA(Range)) pgnData.fields.push_back({"Range", String(Range, 0), "m"});
            }
            break;
        }

        case 129025: { // Position Rapid
            double Latitude, Longitude;
            if(ParseN2kPositionRapid(N2kMsg, Latitude, Longitude)) {
                pgnData.fields.push_back({"Lat", String(Latitude, 6), "deg"});
                pgnData.fields.push_back({"Lon", String(Longitude, 6), "deg"});
            }
            break;
        }

        case 129026: { // COG/SOG Rapid
            unsigned char SID;
            tN2kHeadingReference ref;
            double COG, SOG;
            if(ParseN2kCOGSOGRapid(N2kMsg, SID, ref, COG, SOG)) {
                if(!N2kIsNA(COG)) pgnData.fields.push_back({"COG", String(RadToDeg(COG), 1), "deg"});
                if(!N2kIsNA(SOG)) pgnData.fields.push_back({"SOG", String(msToKnots(SOG), 2), "kn"});
                pgnData.fields.push_back({"Reference", ref == N2khr_magnetic ? "Mag" : "True", ""});
            }
            break;
        }

        case 129029: { // GNSS Position
            unsigned char SID;
            uint16_t DaysSince1970;
            double SecondsSinceMidnight;
            double Latitude, Longitude, Altitude;
            tN2kGNSStype GNSStype;
            tN2kGNSSmethod GNSSmethod;
            unsigned char nSatellites;
            double HDOP, PDOP, GeoidalSeparation;
            unsigned char nReferenceStations;
            tN2kGNSStype ReferenceStationType;
            uint16_t ReferenceSationID;
            double AgeOfCorrection;
            if(ParseN2kGNSS(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight,
                            Latitude, Longitude, Altitude, GNSStype, GNSSmethod,
                            nSatellites, HDOP, PDOP, GeoidalSeparation,
                            nReferenceStations, ReferenceStationType, ReferenceSationID, AgeOfCorrection)) {
                pgnData.fields.push_back({"Lat", String(Latitude, 6), "deg"});
                pgnData.fields.push_back({"Lon", String(Longitude, 6), "deg"});
                if(!N2kIsNA(Altitude)) pgnData.fields.push_back({"Alt", String(Altitude, 1), "m"});
                pgnData.fields.push_back({"Sats", String(nSatellites), ""});
                if(!N2kIsNA(HDOP)) pgnData.fields.push_back({"HDOP", String(HDOP, 1), ""});
            }
            break;
        }

        case 130306: { // Wind Data
            unsigned char SID;
            double WindSpeed, WindAngle;
            tN2kWindReference WindReference;
            if(ParseN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference)) {
                pgnData.fields.push_back({"Speed", String(msToKnots(WindSpeed), 1), "kn"});
                pgnData.fields.push_back({"Angle", String(RadToDeg(WindAngle), 1), "deg"});
                const char* refNames[] = {"True N", "Mag N", "Apparent", "True Boat", "True Water"};
                if((int)WindReference < 5) pgnData.fields.push_back({"Ref", refNames[(int)WindReference], ""});
            }
            break;
        }

        case 130310: { // Environmental Outside
            unsigned char SID;
            double WaterTemperature, OutsideAmbientAirTemperature, AtmosphericPressure;
            if(ParseN2kOutsideEnvironmentalParameters(N2kMsg, SID, WaterTemperature,
                                                       OutsideAmbientAirTemperature, AtmosphericPressure)) {
                if(!N2kIsNA(WaterTemperature)) pgnData.fields.push_back({"Water Temp", String(KelvinToC(WaterTemperature), 1), "C"});
                if(!N2kIsNA(OutsideAmbientAirTemperature)) pgnData.fields.push_back({"Air Temp", String(KelvinToC(OutsideAmbientAirTemperature), 1), "C"});
                if(!N2kIsNA(AtmosphericPressure)) pgnData.fields.push_back({"Pressure", String(AtmosphericPressure/100, 1), "mbar"});
            }
            break;
        }

        case 130311: { // Environmental Parameters
            unsigned char SID;
            tN2kTempSource TempSource;
            double Temperature;
            tN2kHumiditySource HumiditySource;
            double Humidity, AtmosphericPressure;
            if(ParseN2kEnvironmentalParameters(N2kMsg, SID, TempSource, Temperature,
                                               HumiditySource, Humidity, AtmosphericPressure)) {
                if(!N2kIsNA(Temperature)) pgnData.fields.push_back({"Temp", String(KelvinToC(Temperature), 1), "C"});
                if(!N2kIsNA(Humidity)) pgnData.fields.push_back({"Humidity", String(Humidity, 1), "%"});
                if(!N2kIsNA(AtmosphericPressure)) pgnData.fields.push_back({"Pressure", String(AtmosphericPressure/100, 1), "mbar"});
            }
            break;
        }

        case 130312: { // Temperature
            unsigned char SID, TempInstance;
            tN2kTempSource TempSource;
            double ActualTemperature, SetTemperature;
            if(ParseN2kTemperature(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature)) {
                pgnData.fields.push_back({"Instance", String(TempInstance), ""});
                if(!N2kIsNA(ActualTemperature)) pgnData.fields.push_back({"Actual", String(KelvinToC(ActualTemperature), 1), "C"});
                if(!N2kIsNA(SetTemperature)) pgnData.fields.push_back({"Set", String(KelvinToC(SetTemperature), 1), "C"});
            }
            break;
        }

        case 130313: { // Humidity
            unsigned char SID, HumidityInstance;
            tN2kHumiditySource HumiditySource;
            double ActualHumidity, SetHumidity;
            if(ParseN2kHumidity(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity, SetHumidity)) {
                pgnData.fields.push_back({"Instance", String(HumidityInstance), ""});
                if(!N2kIsNA(ActualHumidity)) pgnData.fields.push_back({"Actual", String(ActualHumidity, 1), "%"});
                if(!N2kIsNA(SetHumidity)) pgnData.fields.push_back({"Set", String(SetHumidity, 1), "%"});
            }
            break;
        }

        case 130314: { // Pressure
            unsigned char SID, PressureInstance;
            tN2kPressureSource PressureSource;
            double Pressure;
            if(ParseN2kPressure(N2kMsg, SID, PressureInstance, PressureSource, Pressure)) {
                pgnData.fields.push_back({"Instance", String(PressureInstance), ""});
                pgnData.fields.push_back({"Pressure", String(Pressure/100, 1), "mbar"});
            }
            break;
        }

        case 126992: { // System Time
            unsigned char SID;
            uint16_t SystemDate;
            double SystemTime;
            tN2kTimeSource TimeSource;
            if(ParseN2kSystemTime(N2kMsg, SID, SystemDate, SystemTime, TimeSource)) {
                pgnData.fields.push_back({"Days", String(SystemDate), ""});
                int hours = (int)(SystemTime / 3600);
                int mins = (int)((SystemTime - hours * 3600) / 60);
                int secs = (int)(SystemTime - hours * 3600 - mins * 60);
                char timeStr[16];
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, mins, secs);
                pgnData.fields.push_back({"Time", String(timeStr), "UTC"});
            }
            break;
        }

        case 127493: { // Transmission Parameters
            unsigned char EngineInstance;
            tN2kTransmissionGear TransmissionGear;
            double OilPressure, OilTemperature;
            unsigned char DiscreteStatus1;
            if(ParseN2kTransmissionParameters(N2kMsg, EngineInstance, TransmissionGear,
                                              OilPressure, OilTemperature, DiscreteStatus1)) {
                pgnData.fields.push_back({"Instance", String(EngineInstance), ""});
                const char* gearNames[] = {"Forward", "Neutral", "Reverse", "Unknown"};
                if((int)TransmissionGear < 4) pgnData.fields.push_back({"Gear", gearNames[(int)TransmissionGear], ""});
                if(!N2kIsNA(OilPressure)) pgnData.fields.push_back({"Oil Press", String(OilPressure/1000, 1), "kPa"});
                if(!N2kIsNA(OilTemperature)) pgnData.fields.push_back({"Oil Temp", String(KelvinToC(OilTemperature), 1), "C"});
            }
            break;
        }

        case 128275: { // Distance Log
            uint16_t DaysSince1970;
            double SecondsSinceMidnight;
            uint32_t Log, TripLog;
            if(ParseN2kDistanceLog(N2kMsg, DaysSince1970, SecondsSinceMidnight, Log, TripLog)) {
                if(Log != N2kUInt32NA) pgnData.fields.push_back({"Log", String((double)Log/1852.0, 1), "nm"});
                if(TripLog != N2kUInt32NA) pgnData.fields.push_back({"Trip", String((double)TripLog/1852.0, 2), "nm"});
            }
            break;
        }

        case 60928: { // ISO Address Claim
            // Parse the 64-bit NAME field
            if(N2kMsg.DataLen >= 8) {
                uint64_t name = 0;
                for(int i = 0; i < 8; i++) {
                    name |= ((uint64_t)N2kMsg.Data[i]) << (i * 8);
                }

                // Extract fields from NAME using bit manipulation
                uint32_t uniqueNumber = name & 0x1FFFFF;  // 21 bits
                uint16_t mfrCode = (name >> 21) & 0x7FF;  // 11 bits
                uint8_t devInstance = (name >> 32) & 0xFF;  // 8 bits
                uint8_t devFunction = (name >> 40) & 0xFF;  // 8 bits
                uint8_t devClass = ((name >> 49) & 0x7F);  // 7 bits
                uint8_t sysInstance = (name >> 56) & 0x0F;  // 4 bits
                uint8_t indGroup = (name >> 60) & 0x07;  // 3 bits

                pgnData.fields.push_back({"Mfr Code", String(mfrCode), ""});
                pgnData.fields.push_back({"Unique#", String(uniqueNumber), ""});
                pgnData.fields.push_back({"Dev Func", String(devFunction), ""});
                pgnData.fields.push_back({"Dev Class", String(devClass), ""});
                pgnData.fields.push_back({"Instance", String(devInstance), ""});

                // Industry group names
                const char* indNames[] = {"Global", "Highway", "Agri", "Constr", "Marine", "Indust"};
                if(indGroup < 6) {
                    pgnData.fields.push_back({"Industry", indNames[indGroup], ""});
                } else {
                    pgnData.fields.push_back({"Industry", String(indGroup), ""});
                }

                pgnData.fields.push_back({"Sys Inst", String(sysInstance), ""});
            }
            break;
        }

        case 126993: { // Heartbeat
            if(N2kMsg.DataLen >= 8) {
                // Heartbeat interval in ms (first 4 bytes, little endian)
                uint32_t interval = N2kMsg.Data[0] | (N2kMsg.Data[1] << 8) |
                                   (N2kMsg.Data[2] << 16) | (N2kMsg.Data[3] << 24);

                // Sequence counter (byte 4, lower 4 bits)
                uint8_t seqCounter = N2kMsg.Data[4] & 0x0F;

                // Controller states
                uint8_t ctrl1State = N2kMsg.Data[5] & 0x03;
                uint8_t ctrl2State = (N2kMsg.Data[5] >> 2) & 0x03;

                if(interval != 0xFFFFFFFF) {
                    pgnData.fields.push_back({"Interval", String(interval / 1000.0, 1), "sec"});
                }
                pgnData.fields.push_back({"Sequence", String(seqCounter), ""});

                const char* stateNames[] = {"Ctrl", "Auto", "Remote", "N/A"};
                pgnData.fields.push_back({"Ctrl1", stateNames[ctrl1State], ""});
                pgnData.fields.push_back({"Ctrl2", stateNames[ctrl2State], ""});
            }
            break;
        }

        case 126996: { // Product Information
            unsigned short N2kVersion, ProductCode;
            unsigned char CertificationLevel, LoadEquivalency;
            char ModelID[33], SwCode[41], ModelVersion[25], ModelSerialCode[33];

            if(ParseN2kPGN126996(N2kMsg, N2kVersion, ProductCode,
                                 sizeof(ModelID), ModelID,
                                 sizeof(SwCode), SwCode,
                                 sizeof(ModelVersion), ModelVersion,
                                 sizeof(ModelSerialCode), ModelSerialCode,
                                 CertificationLevel, LoadEquivalency)) {
                pgnData.fields.push_back({"N2K Ver", String(N2kVersion), ""});
                pgnData.fields.push_back({"Prod Code", String(ProductCode), ""});
                if(strlen(ModelID) > 0) {
                    String modelStr = String(ModelID);
                    if(modelStr.length() > 14) modelStr = modelStr.substring(0, 14);
                    pgnData.fields.push_back({"Model", modelStr, ""});
                }
                if(strlen(SwCode) > 0) {
                    String swStr = String(SwCode);
                    if(swStr.length() > 12) swStr = swStr.substring(0, 12);
                    pgnData.fields.push_back({"SW", swStr, ""});
                }
            }
            break;
        }

        case 127252: { // Heave
            unsigned char SID;
            double Heave, Delay;
            tN2kDelaySource DelaySource;
            if(ParseN2kHeave(N2kMsg, SID, Heave, Delay, DelaySource)) {
                if(!N2kIsNA(Heave)) pgnData.fields.push_back({"Heave", String(Heave, 2), "m"});
                if(!N2kIsNA(Delay)) pgnData.fields.push_back({"Delay", String(Delay, 3), "s"});
            }
            break;
        }

        case 127497: { // Trip Fuel Parameters
            unsigned char EngineInstance;
            double TripFuelUsed, FuelRateAverage, FuelRateEconomy, InstantaneousFuelEconomy;
            if(ParseN2kEngineTripParameters(N2kMsg, EngineInstance, TripFuelUsed, FuelRateAverage, FuelRateEconomy, InstantaneousFuelEconomy)) {
                pgnData.fields.push_back({"Engine", String(EngineInstance), ""});
                if(!N2kIsNA(TripFuelUsed)) pgnData.fields.push_back({"Trip Fuel", String(TripFuelUsed, 1), "L"});
                if(!N2kIsNA(FuelRateAverage)) pgnData.fields.push_back({"Avg Rate", String(FuelRateAverage * 3600, 1), "L/h"});
            }
            break;
        }

        case 127501: { // Binary Switch Status
            unsigned char DeviceBankInstance;
            tN2kBinaryStatus BankStatus;
            if(ParseN2kBinaryStatus(N2kMsg, DeviceBankInstance, BankStatus)) {
                pgnData.fields.push_back({"Bank", String(DeviceBankInstance), ""});
                // Show first 8 switch states
                String states = "";
                for(int i = 0; i < 8; i++) {
                    tN2kOnOff status = N2kGetStatusOnBinaryStatus(BankStatus, i + 1);
                    states += (status == N2kOnOff_On) ? "1" : ((status == N2kOnOff_Off) ? "0" : "-");
                }
                pgnData.fields.push_back({"Sw 1-8", states, ""});
            }
            break;
        }

        case 127506: { // DC Detailed Status
            unsigned char SID, DCInstance;
            tN2kDCType DCType;
            uint8_t StateOfCharge, StateOfHealth;
            double TimeRemaining, RippleVoltage, Capacity;
            if(ParseN2kDCStatus(N2kMsg, SID, DCInstance, DCType, StateOfCharge, StateOfHealth, TimeRemaining, RippleVoltage, Capacity)) {
                pgnData.fields.push_back({"Instance", String(DCInstance), ""});
                if(StateOfCharge <= 100) pgnData.fields.push_back({"SOC", String(StateOfCharge), "%"});
                if(StateOfHealth <= 100) pgnData.fields.push_back({"Health", String(StateOfHealth), "%"});
                if(!N2kIsNA(Capacity)) pgnData.fields.push_back({"Capacity", String(Capacity, 0), "Ah"});
            }
            break;
        }

        case 127507: { // Charger Status
            unsigned char Instance, BatteryInstance;
            tN2kChargeState ChargeState;
            tN2kChargerMode ChargerMode;
            tN2kOnOff Enabled;
            tN2kOnOff EqualizationPending;
            double EqualizationTimeRemaining;
            if(ParseN2kChargerStatus(N2kMsg, Instance, BatteryInstance, ChargeState, ChargerMode, Enabled, EqualizationPending, EqualizationTimeRemaining)) {
                pgnData.fields.push_back({"Charger", String(Instance), ""});
                pgnData.fields.push_back({"Battery", String(BatteryInstance), ""});
                const char* stateNames[] = {"Not Chg", "Bulk", "Absorb", "Overchg", "Equal", "Float", "No Float", "Fault"};
                if((int)ChargeState < 8) pgnData.fields.push_back({"State", stateNames[(int)ChargeState], ""});
                pgnData.fields.push_back({"Enabled", Enabled == N2kOnOff_On ? "Yes" : "No", ""});
            }
            break;
        }

        case 127513: { // Battery Configuration
            unsigned char BatInstance;
            tN2kBatType BatType;
            tN2kBatEqSupport SupportsEqual;
            tN2kBatNomVolt BatNominalVoltage;
            tN2kBatChem BatChemistry;
            double BatCapacity;
            int8_t BatTemperatureCoefficient;
            double PeukertExponent;
            int8_t ChargeEfficiencyFactor;
            if(ParseN2kBatConf(N2kMsg, BatInstance, BatType, SupportsEqual, BatNominalVoltage, BatChemistry, BatCapacity, BatTemperatureCoefficient, PeukertExponent, ChargeEfficiencyFactor)) {
                pgnData.fields.push_back({"Instance", String(BatInstance), ""});
                const char* typeNames[] = {"Flooded", "Gel", "AGM"};
                if((int)BatType < 3) pgnData.fields.push_back({"Type", typeNames[(int)BatType], ""});
                const char* chemNames[] = {"Lead Acid", "LiIon", "NiCad", "NiMH"};
                if((int)BatChemistry < 4) pgnData.fields.push_back({"Chemistry", chemNames[(int)BatChemistry], ""});
                if(!N2kIsNA(BatCapacity)) pgnData.fields.push_back({"Capacity", String(BatCapacity / 3600, 0), "Ah"});
            }
            break;
        }

        case 127751: { // DC Voltage/Current
            unsigned char Instance, SID;
            double Voltage, Current;
            if(ParseN2kPGN127751(N2kMsg, Instance, Voltage, Current, SID)) {
                pgnData.fields.push_back({"Instance", String(Instance), ""});
                if(!N2kIsNA(Voltage)) pgnData.fields.push_back({"Voltage", String(Voltage, 2), "V"});
                if(!N2kIsNA(Current)) pgnData.fields.push_back({"Current", String(Current, 1), "A"});
            }
            break;
        }

        case 128000: { // Leeway
            unsigned char SID;
            double Leeway;
            if(ParseN2kLeeway(N2kMsg, SID, Leeway)) {
                if(!N2kIsNA(Leeway)) pgnData.fields.push_back({"Leeway", String(RadToDeg(Leeway), 1), "deg"});
            }
            break;
        }

        case 129033: { // Time & Date
            uint16_t DaysSince1970;
            double SecondsSinceMidnight;
            int16_t LocalOffset;
            if(ParseN2kLocalOffset(N2kMsg, DaysSince1970, SecondsSinceMidnight, LocalOffset)) {
                int hours = (int)(SecondsSinceMidnight / 3600);
                int mins = (int)((SecondsSinceMidnight - hours * 3600) / 60);
                int secs = (int)(SecondsSinceMidnight - hours * 3600 - mins * 60);
                char timeStr[12];
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, mins, secs);
                pgnData.fields.push_back({"Time", String(timeStr), ""});
                pgnData.fields.push_back({"Offset", String(LocalOffset), "min"});
            }
            break;
        }

        case 129038: { // AIS Class A Position Report
            uint8_t MessageID;
            tN2kAISRepeat Repeat;
            uint32_t UserID;
            double Latitude, Longitude, COG, SOG, Heading, ROT;
            bool Accuracy, RAIM;
            uint8_t Seconds;
            tN2kAISNavStatus NavStatus;
            tN2kAISTransceiverInformation AISInfo;
            unsigned char SID;
            if(ParseN2kPGN129038(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude,
                                 Accuracy, RAIM, Seconds, COG, SOG, Heading, ROT, NavStatus, AISInfo, SID)) {
                pgnData.fields.push_back({"MMSI", String(UserID), ""});
                if(!N2kIsNA(Latitude)) pgnData.fields.push_back({"Lat", String(Latitude, 4), "deg"});
                if(!N2kIsNA(Longitude)) pgnData.fields.push_back({"Lon", String(Longitude, 4), "deg"});
                if(!N2kIsNA(SOG)) pgnData.fields.push_back({"SOG", String(msToKnots(SOG), 1), "kn"});
                if(!N2kIsNA(COG)) pgnData.fields.push_back({"COG", String(RadToDeg(COG), 0), "deg"});
            }
            break;
        }

        case 129039: { // AIS Class B Position Report
            uint8_t MessageID;
            tN2kAISRepeat Repeat;
            uint32_t UserID;
            double Latitude, Longitude, COG, SOG, Heading;
            bool Accuracy, RAIM;
            uint8_t Seconds;
            tN2kAISUnit Unit;
            bool Display, DSC, Band, Msg22;
            tN2kAISMode Mode;
            bool State;
            tN2kAISTransceiverInformation AISInfo;
            unsigned char SID;
            if(ParseN2kPGN129039(N2kMsg, MessageID, Repeat, UserID, Latitude, Longitude,
                                 Accuracy, RAIM, Seconds, COG, SOG, AISInfo, Heading,
                                 Unit, Display, DSC, Band, Msg22, Mode, State, SID)) {
                pgnData.fields.push_back({"MMSI", String(UserID), ""});
                if(!N2kIsNA(Latitude)) pgnData.fields.push_back({"Lat", String(Latitude, 4), "deg"});
                if(!N2kIsNA(Longitude)) pgnData.fields.push_back({"Lon", String(Longitude, 4), "deg"});
                if(!N2kIsNA(SOG)) pgnData.fields.push_back({"SOG", String(msToKnots(SOG), 1), "kn"});
                if(!N2kIsNA(COG)) pgnData.fields.push_back({"COG", String(RadToDeg(COG), 0), "deg"});
            }
            break;
        }

        case 129283: { // Cross Track Error
            unsigned char SID;
            tN2kXTEMode XTEMode;
            bool NavigationTerminated;
            double XTE;
            if(ParseN2kXTE(N2kMsg, SID, XTEMode, NavigationTerminated, XTE)) {
                if(!N2kIsNA(XTE)) pgnData.fields.push_back({"XTE", String(XTE, 0), "m"});
                pgnData.fields.push_back({"Nav Term", NavigationTerminated ? "Yes" : "No", ""});
            }
            break;
        }

        case 129284: { // Navigation Route/WP Information
            unsigned char SID;
            double DistanceToWaypoint, BearingPositionToWaypoint, BearingOriginToWaypoint;
            double ETATime;
            int16_t ETADate;
            tN2kHeadingReference BearingRef;
            bool PerpendicularCrossed, ArrivalCircleEntered;
            tN2kDistanceCalculationType CalculationType;
            uint32_t OriginWaypointNumber, DestinationWaypointNumber;
            double DestinationLatitude, DestinationLongitude, WaypointClosingVelocity;
            if(ParseN2kNavigationInfo(N2kMsg, SID, DistanceToWaypoint, BearingRef, PerpendicularCrossed,
                                      ArrivalCircleEntered, CalculationType, ETATime, ETADate,
                                      BearingOriginToWaypoint, BearingPositionToWaypoint,
                                      OriginWaypointNumber, DestinationWaypointNumber,
                                      DestinationLatitude, DestinationLongitude, WaypointClosingVelocity)) {
                if(!N2kIsNA(DistanceToWaypoint)) pgnData.fields.push_back({"Dist WP", String(DistanceToWaypoint, 0), "m"});
                if(!N2kIsNA(BearingPositionToWaypoint)) pgnData.fields.push_back({"Bearing", String(RadToDeg(BearingPositionToWaypoint), 0), "deg"});
                if(!N2kIsNA(WaypointClosingVelocity)) pgnData.fields.push_back({"VMG", String(msToKnots(WaypointClosingVelocity), 1), "kn"});
                pgnData.fields.push_back({"Arrived", ArrivalCircleEntered ? "Yes" : "No", ""});
            }
            break;
        }

        case 129539: { // GNSS DOPs
            unsigned char SID;
            tN2kGNSSDOPmode DesiredMode, ActualMode;
            double HDOP, VDOP, TDOP;
            if(ParseN2kGNSSDOPData(N2kMsg, SID, DesiredMode, ActualMode, HDOP, VDOP, TDOP)) {
                if(!N2kIsNA(HDOP)) pgnData.fields.push_back({"HDOP", String(HDOP, 2), ""});
                if(!N2kIsNA(VDOP)) pgnData.fields.push_back({"VDOP", String(VDOP, 2), ""});
                if(!N2kIsNA(TDOP)) pgnData.fields.push_back({"TDOP", String(TDOP, 2), ""});
                const char* modeNames[] = {"1D", "2D", "3D", "Auto", "Reserved", "Error"};
                if((int)ActualMode < 6) pgnData.fields.push_back({"Mode", modeNames[(int)ActualMode], ""});
            }
            break;
        }

        case 129540: { // GNSS Satellites in View
            unsigned char SID;
            tN2kRangeResidualMode Mode;
            uint8_t NumberOfSVs;
            if(ParseN2kPGN129540(N2kMsg, SID, Mode, NumberOfSVs)) {
                pgnData.fields.push_back({"Sats", String(NumberOfSVs), ""});
                // Get info for first few satellites
                for(uint8_t i = 0; i < min((uint8_t)3, NumberOfSVs); i++) {
                    tSatelliteInfo satInfo;
                    if(ParseN2kPGN129540(N2kMsg, i, satInfo)) {
                        String satStr = "SV" + String(satInfo.PRN) + " El" + String(RadToDeg(satInfo.Elevation), 0);
                        pgnData.fields.push_back({"", satStr, ""});
                    }
                }
            }
            break;
        }

        case 129794: { // AIS Class A Static Data
            uint8_t MessageID;
            tN2kAISRepeat Repeat;
            uint32_t UserID;
            uint32_t IMONumber;
            char Callsign[8];
            char Name[21];
            uint8_t VesselType;
            double Length, Beam, PosRefStbd, PosRefBow;
            uint16_t ETAdate;
            double ETAtime, Draught;
            char Destination[21];
            tN2kAISVersion AISversion;
            tN2kGNSStype GNSStype;
            tN2kAISDTE DTE;
            tN2kAISTransceiverInformation AISinfo;
            uint8_t SID;
            if(ParseN2kPGN129794(N2kMsg, MessageID, Repeat, UserID, IMONumber, Callsign, sizeof(Callsign), Name, sizeof(Name),
                                 VesselType, Length, Beam, PosRefStbd, PosRefBow,
                                 ETAdate, ETAtime, Draught, Destination, sizeof(Destination), AISversion, GNSStype, DTE, AISinfo, SID)) {
                pgnData.fields.push_back({"MMSI", String(UserID), ""});
                if(strlen(Name) > 0) pgnData.fields.push_back({"Name", String(Name).substring(0, 12), ""});
                if(strlen(Callsign) > 0) pgnData.fields.push_back({"Call", String(Callsign), ""});
                if(!N2kIsNA(Length)) pgnData.fields.push_back({"Length", String(Length, 0), "m"});
            }
            break;
        }

        case 129809: { // AIS Class B CS Static Data Part A
            uint8_t MessageID;
            tN2kAISRepeat Repeat;
            uint32_t UserID;
            char Name[21];
            tN2kAISTransceiverInformation AISInfo;
            uint8_t SID;
            if(ParseN2kPGN129809(N2kMsg, MessageID, Repeat, UserID, Name, sizeof(Name), AISInfo, SID)) {
                pgnData.fields.push_back({"MMSI", String(UserID), ""});
                if(strlen(Name) > 0) pgnData.fields.push_back({"Name", String(Name).substring(0, 12), ""});
            }
            break;
        }

        case 129810: { // AIS Class B CS Static Data Part B
            uint8_t MessageID;
            tN2kAISRepeat Repeat;
            uint32_t UserID, MothershipUserID;
            uint8_t VesselType;
            char Callsign[8];
            char Vendor[8];
            double Length, Beam, PosRefStbd, PosRefBow;
            tN2kAISTransceiverInformation AISInfo;
            uint8_t SID;
            if(ParseN2kPGN129810(N2kMsg, MessageID, Repeat, UserID, VesselType, Vendor, sizeof(Vendor), Callsign, sizeof(Callsign),
                                 Length, Beam, PosRefStbd, PosRefBow, MothershipUserID, AISInfo, SID)) {
                pgnData.fields.push_back({"MMSI", String(UserID), ""});
                if(strlen(Callsign) > 0) pgnData.fields.push_back({"Call", String(Callsign), ""});
                if(!N2kIsNA(Length)) pgnData.fields.push_back({"Length", String(Length, 0), "m"});
                if(!N2kIsNA(Beam)) pgnData.fields.push_back({"Beam", String(Beam, 1), "m"});
            }
            break;
        }

        case 130316: { // Temperature Extended Range
            unsigned char SID, TempInstance;
            tN2kTempSource TempSource;
            double ActualTemperature, SetTemperature;
            if(ParseN2kTemperatureExt(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature)) {
                pgnData.fields.push_back({"Instance", String(TempInstance), ""});
                const char* srcNames[] = {"Sea", "Outside", "Inside", "Engine", "Cabin", "LiveWell", "Bait", "Refrig", "Heat", "Dew", "Wind", "App Wind", "Exh", "Shift"};
                if((int)TempSource < 14) pgnData.fields.push_back({"Source", srcNames[(int)TempSource], ""});
                if(!N2kIsNA(ActualTemperature)) pgnData.fields.push_back({"Temp", String(KelvinToC(ActualTemperature), 1), "C"});
                if(!N2kIsNA(SetTemperature)) pgnData.fields.push_back({"Set", String(KelvinToC(SetTemperature), 1), "C"});
            }
            break;
        }

        case 130576: { // Trim Tab Status
            int8_t PortTrimTab, StbdTrimTab;
            if(ParseN2kTrimTab(N2kMsg, PortTrimTab, StbdTrimTab)) {
                if(PortTrimTab != N2kInt8NA) pgnData.fields.push_back({"Port", String(PortTrimTab), "%"});
                if(StbdTrimTab != N2kInt8NA) pgnData.fields.push_back({"Stbd", String(StbdTrimTab), "%"});
            }
            break;
        }

        case 130577: { // Direction Data
            tN2kDataMode DataMode;
            tN2kHeadingReference CogReference;
            unsigned char SID;
            double COG, SOG, Heading, SpeedThroughWater, Set, Drift;
            if(ParseN2kDirectionData(N2kMsg, DataMode, CogReference, SID, COG, SOG, Heading, SpeedThroughWater, Set, Drift)) {
                if(!N2kIsNA(COG)) pgnData.fields.push_back({"COG", String(RadToDeg(COG), 0), "deg"});
                if(!N2kIsNA(SOG)) pgnData.fields.push_back({"SOG", String(msToKnots(SOG), 1), "kn"});
                if(!N2kIsNA(Heading)) pgnData.fields.push_back({"Heading", String(RadToDeg(Heading), 0), "deg"});
                if(!N2kIsNA(Set)) pgnData.fields.push_back({"Set", String(RadToDeg(Set), 0), "deg"});
                if(!N2kIsNA(Drift)) pgnData.fields.push_back({"Drift", String(msToKnots(Drift), 1), "kn"});
            }
            break;
        }

        default: {
            // For unknown PGNs, show raw data bytes
            pgnData.fields.push_back({"DataLen", String(N2kMsg.DataLen), "bytes"});
            String hexData = "";
            for(int i = 0; i < min((int)N2kMsg.DataLen, 8); i++) {
                if(i > 0) hexData += " ";
                if(N2kMsg.Data[i] < 16) hexData += "0";
                hexData += String(N2kMsg.Data[i], HEX);
            }
            pgnData.fields.push_back({"Data", hexData, ""});
            if(N2kMsg.DataLen > 8) {
                hexData = "";
                for(int i = 8; i < min((int)N2kMsg.DataLen, 16); i++) {
                    if(i > 8) hexData += " ";
                    if(N2kMsg.Data[i] < 16) hexData += "0";
                    hexData += String(N2kMsg.Data[i], HEX);
                }
                pgnData.fields.push_back({"", hexData, ""});
            }
            break;
        }
    }
}
