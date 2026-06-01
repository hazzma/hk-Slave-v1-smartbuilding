#include "CapabilityManager.h"

CapabilityManager::CapabilityManager(DHT22Module* dht, BH1750Module* lux, SCD30Module* co2, 
                                     PresenceDigitalModule* presence, IRComboModule* ir)
    : _dhtModule(dht), _luxModule(lux), _co2Module(co2), _presenceModule(presence), _irModule(ir),
      _lastTempMask(0), _lastLuxMask(0), _lastCo2Count(0), _lastPresenceMask(0),
      _lastIrProj(0), _lastIrAc1(0), _lastIrAc2(0) {}

void CapabilityManager::begin() {
    // Initial sync
    update(millis());
}

void CapabilityManager::update(uint32_t now_ms) {
    SlaveCapability& cap = runtime.getCapability();

    // 1. DHT22 Temperature Assignment
    if (cap.temp_assignment_mask != _lastTempMask) {
        _lastTempMask = cap.temp_assignment_mask;
        _dhtModule->setChannelAssignment(0, (cap.temp_assignment_mask & 8) != 0);
        _dhtModule->setChannelAssignment(1, (cap.temp_assignment_mask & 4) != 0);
        _dhtModule->setChannelAssignment(2, (cap.temp_assignment_mask & 2) != 0);
        _dhtModule->setChannelAssignment(3, (cap.temp_assignment_mask & 1) != 0);
        
        _dhtModule->setEnabled(cap.temp_assignment_mask > 0);
        _dhtModule->begin();
    }

    // 2. BH1750 Lux Assignment
    if (cap.lux_assignment_mask != _lastLuxMask) {
        _lastLuxMask = cap.lux_assignment_mask;
        _luxModule->setChannelAssignment(0, (cap.lux_assignment_mask & 8) != 0);
        _luxModule->setChannelAssignment(1, (cap.lux_assignment_mask & 4) != 0);
        _luxModule->setChannelAssignment(2, (cap.lux_assignment_mask & 2) != 0);
        _luxModule->setChannelAssignment(3, (cap.lux_assignment_mask & 1) != 0);

        _luxModule->setEnabled(cap.lux_assignment_mask > 0);
        _luxModule->begin();
    }

    // 3. SCD30 CO2 Count Assignment
    if (cap.co2_count != _lastCo2Count) {
        _lastCo2Count = cap.co2_count;
        _co2Module->setAssigned(cap.co2_count > 0);
        _co2Module->setEnabled(cap.co2_count > 0);
        _co2Module->begin();
    }

    // 4. Presence Digital Assignment
    if (cap.presence_assignment_mask != _lastPresenceMask) {
        _lastPresenceMask = cap.presence_assignment_mask;
        _presenceModule->setChannelAssignment(0, (cap.presence_assignment_mask & 8) != 0);
        _presenceModule->setChannelAssignment(1, (cap.presence_assignment_mask & 4) != 0);
        _presenceModule->setChannelAssignment(2, (cap.presence_assignment_mask & 2) != 0);
        _presenceModule->setChannelAssignment(3, (cap.presence_assignment_mask & 1) != 0);

        _presenceModule->setEnabled(cap.presence_assignment_mask > 0);
        _presenceModule->begin();
    }

    // 5. IR AC and Projector assignments
    if (cap.ir_projector_enable != _lastIrProj || 
        cap.ir_ac_1_enable != _lastIrAc1 || 
        cap.ir_ac_2_enable != _lastIrAc2) {
        
        _lastIrProj = cap.ir_projector_enable;
        _lastIrAc1 = cap.ir_ac_1_enable;
        _lastIrAc2 = cap.ir_ac_2_enable;

        _irModule->setProjectorEnabled(cap.ir_projector_enable > 0);
        _irModule->setAC1Enabled(cap.ir_ac_1_enable > 0);
        _irModule->setAC2Enabled(cap.ir_ac_2_enable > 0);

        bool anyIrEnabled = (cap.ir_projector_enable > 0 || cap.ir_ac_1_enable > 0 || cap.ir_ac_2_enable > 0);
        _irModule->setEnabled(anyIrEnabled);
        _irModule->begin();
    }
}
