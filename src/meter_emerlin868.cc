/*
 Copyright (C) 2017-2020 Fredrik Öhrström (gpl-3.0-or-later)
 Copyright (C) 2018 David Mallon (gpl-3.0-or-later)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include"dvparser.h"
#include"meters.h"
#include"meters_common_implementation.h"
#include"wmbus.h"
#include"wmbus_utils.h"

using namespace std;

struct MeterEMerlin868 : public virtual MeterCommonImplementation {
    MeterEMerlin868(MeterInfo &mi);

    // Total water counted through the meter
    double totalWaterConsumption(Unit u);
    bool  hasTotalWaterConsumption();
    double targetWaterConsumption(Unit u);
    bool   hasTargetWaterConsumption();

private:
    void processContent(Telegram *t);

    double actual_total_water_consumption_m3_ {};
    double last_total_water_consumption_m3h_ {};
};

MeterEMerlin868::MeterEMerlin868(MeterInfo &mi) :
    MeterCommonImplementation(mi, "emerlin868")
{
    setMeterType(MeterType::WaterMeter);

    setExpectedTPLSecurityMode(TPLSecurityMode::AES_CBC_IV);

    addLinkMode(LinkMode::T1);

    // version 0x68
    // version 0x7c Sensus 640

    addPrint("total", Quantity::Volume,
             [&](Unit u){ return totalWaterConsumption(u); },
             "The total water consumption recorded by this meter.",
             PrintProperty::FIELD | PrintProperty::JSON);

    addPrint("target", Quantity::Volume,
             [&](Unit u){ return targetWaterConsumption(u); },
             "The target water consumption recorded at previous period.",
             PrintProperty::FIELD | PrintProperty::JSON);
}

shared_ptr<Meter> createEMerlin868(MeterInfo &mi)
{
    return shared_ptr<Meter>(new MeterEMerlin868(mi));
}

void MeterEMerlin868::processContent(Telegram *t)
{
    int offset;
    string key;

    if(findKey(MeasurementType::Unknown, VIFRange::Volume, 0, 0, &key, &t->dv_entries)) {
        extractDVdouble(&t->dv_entries, key, &offset, &actual_total_water_consumption_m3_);
        t->addMoreExplanation(offset, " actual total consumption (%f m3)", actual_total_water_consumption_m3_);
    }

    if(findKey(MeasurementType::Unknown, VIFRange::Volume, 1, 0, &key, &t->dv_entries)) {
        extractDVdouble(&t->dv_entries, key, &offset, &last_total_water_consumption_m3h_);
        t->addMoreExplanation(offset, " last total consumption (%f m3)", last_total_water_consumption_m3h_);
    }


}

double MeterEMerlin868::totalWaterConsumption(Unit u)
{
    assertQuantity(u, Quantity::Volume);
    return convert(actual_total_water_consumption_m3_, Unit::M3, u);
}

bool MeterEMerlin868::hasTotalWaterConsumption()
{
    return true;
}

double MeterEMerlin868::targetWaterConsumption(Unit u)
{
    assertQuantity(u, Quantity::Volume);
    return convert(last_total_water_consumption_m3h_, Unit::M3, u);
}

bool MeterEMerlin868::hasTargetWaterConsumption()
{
    return true;
}
