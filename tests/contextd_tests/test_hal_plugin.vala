/*
 * Copyright (C) 2008, 2009 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

const string halPercentage = "battery.charge_level.percentage";
const string halCharge = "battery.charge_level.current";
const string halLastFull = "battery.charge_level.last_full";
const string halRate = "battery.charge_level.rate";
const string halCharging = "battery.rechargeable.is_charging";
const string halDischarging = "battery.rechargeable.is_discharging";

const string keyChargePercentage = "Battery.ChargePercentage";
const string keyOnBattery = "Battery.OnBattery";
const string keyLowBattery = "Battery.LowBattery";
const string keyIsCharging = "Battery.IsCharging";
const string keyTimeUntilLow = "Battery.TimeUntilLow";
const string keyTimeUntilFull = "Battery.TimeUntilFull";

void test_install() {
	// Setup
	ContextProvider.initializeMock();
	Hal.initializeMock();

	// Test
	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	bool success = plugin.install ();

	// Expected results:
	// the installation succeeded
	assert (success == true);
	// the plugin declares the correct keys
	Gee.HashSet<string> keys = ContextProvider.providedKeys;
	assert (keys.contains (keyChargePercentage));
	assert (keys.contains (keyOnBattery));
	assert (keys.contains (keyLowBattery));
	assert (keys.contains (keyIsCharging));
	assert (keys.contains (keyTimeUntilLow));
	assert (keys.contains (keyTimeUntilFull));
}

void test_startsubscribing() {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 20, false);
	Hal.changePropertyInt (halCharge, 60, false);
	Hal.changePropertyInt (halLastFull, 300, false);
	Hal.changePropertyInt (halRate, 10, false);
	Hal.changePropertyBool (halCharging, true, false);
	Hal.changePropertyBool (halDischarging, false, false);
	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();

	ContextProvider.resetValues ();

	// Test
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Expected result: the initial values for all keys are set
	Gee.HashMap<string, int> intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyChargePercentage));
	assert (intValues.get_keys().contains(keyTimeUntilFull));
	Gee.HashMap<string, bool> boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyOnBattery));
	assert (boolValues.get_keys().contains(keyLowBattery));
	assert (boolValues.get_keys().contains(keyIsCharging));
	Gee.HashSet<string> unknownValues = ContextProvider.unknownValues;
	assert (unknownValues.contains(keyTimeUntilLow));
	// And the hal plugin started listening for property changes of the battery
	assert (Hal.propertyWatchAdded == true);
	assert (Hal.propertyWatchRemoved == false);
}

void test_stopsubscribing() {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 20, false);
	Hal.changePropertyInt (halCharge, 60, false);
	Hal.changePropertyInt (halLastFull, 300, false);
	Hal.changePropertyInt (halRate, 10, false);
	Hal.changePropertyBool (halCharging, true, false);
	Hal.changePropertyBool (halDischarging, false, false);
	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();

	ContextProvider.resetValues ();

	// Test
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);
	// and that nobody is listening any more
	ContextProvider.callSubscriptionCallback(false);

	// Expected result: 
	// The hal plugin stopped listening for property changes of the battery
	assert (Hal.propertyWatchAdded == true);
	assert (Hal.propertyWatchRemoved == true);
}

void test_chargepercentage () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, true, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	ContextProvider.resetValues ();
	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	Hal.changePropertyInt(halPercentage, 53, true);

	// Expected results:
	Gee.HashMap<string, int> intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyChargePercentage));
	assert (intValues.get(keyChargePercentage) == 53);
}

void test_onbattery () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halDischarging, true, true);

	// Expected results:
	Gee.HashMap<string, bool> boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyOnBattery));
	assert (boolValues.get(keyOnBattery) == true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halDischarging, false, true);

	// Expected results:
	boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyOnBattery));
	assert (boolValues.get(keyOnBattery) == false);
}

void test_lowbattery () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool (halDischarging, true, true);
	Hal.changePropertyInt(halPercentage, 5, true);
	// Note: if we set more than one properties, we need to call the
	// property_modified callback for both of them (last parameter: true)

	// Expected results:
	Gee.HashMap<string, bool> boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyLowBattery));
	assert (boolValues.get(keyLowBattery) == true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool (halDischarging, true, false);
	Hal.changePropertyInt(halPercentage, 15, true);

	// Expected results:
	boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyLowBattery));
	assert (boolValues.get(keyLowBattery) == false);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool (halDischarging, false, true);
	Hal.changePropertyInt(halPercentage, 5, true);

	// Expected results:
	boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyLowBattery));
	assert (boolValues.get(keyLowBattery) == false);
}

void test_ischarging () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halCharging, true, true);

	// Expected results:
	Gee.HashMap<string, bool> boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyIsCharging));
	assert (boolValues.get(keyIsCharging) == true);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halCharging, false, true);

	// Expected results:
	boolValues = ContextProvider.boolValues;
	assert (boolValues.get_keys().contains(keyIsCharging));
	assert (boolValues.get(keyIsCharging) == false);
}

void test_timeuntillow () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 10, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, true, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test: the initial values
	Gee.HashMap<string, int> intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilLow));

	// Takes 8 hours to reach battery 10% (= charge 20)
	assert (intValues.get(keyTimeUntilLow) == 8 * 3600);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halDischarging, true, true);
	Hal.changePropertyInt(halCharge, 7000, true);
	Hal.changePropertyInt(halLastFull, 10000, true);
	Hal.changePropertyInt(halRate, 1000, true);

	// Expected results:
	intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilLow));

	// Takes 6 hours to reach battery 10%
	assert (intValues.get(keyTimeUntilLow) == 6 * 3600);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halDischarging, true, true);
	Hal.changePropertyInt(halCharge, 5, true); // Already low!
	Hal.changePropertyInt(halLastFull, 100, true);
	Hal.changePropertyInt(halRate, 3, true);

	// Expected results:
	intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilLow));
	assert (intValues.get(keyTimeUntilLow) == 0);
}

void test_timeuntilfull () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 10, false);
	Hal.changePropertyBool (halCharging, true, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test: the initial values
	Gee.HashMap<string, int> intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilFull));

	// Takes 10 hours to reach battery charge 200
	assert (intValues.get(keyTimeUntilFull) == 10 * 3600);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halCharging, true, true);
	Hal.changePropertyInt(halCharge, 7000, true);
	Hal.changePropertyInt(halLastFull, 10000, true);
	Hal.changePropertyInt(halRate, 1000, true);

	// Expected results:
	intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilFull));
	assert (intValues.get(keyTimeUntilFull) == 3 * 3600);

	// Test
	// Command libhal mock implementation to change a value
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool(halCharging, true, true);
	Hal.changePropertyInt(halCharge, 100, true); // Already full
	Hal.changePropertyInt(halLastFull, 100, true);
	Hal.changePropertyInt(halRate, 3, true);

	// Expected results:
	intValues = ContextProvider.intValues;
	assert (intValues.get_keys().contains(keyTimeUntilFull));
	assert (intValues.get(keyTimeUntilFull) == 0);
}

void test_irrelevantchange () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Command libhal mock implementation to change two irrelevant values
	// of a property and to notify the client of the change
	ContextProvider.resetValues ();
	Hal.changePropertyBool("irrelevant", true, true);
	Hal.changePropertyInt("another_irrelevant", 42, true);

	// Expected results:
	// no changes were received
	assert(ContextProvider.intValues.get_keys().size == 0);
	assert(ContextProvider.doubleValues.get_keys().size == 0);
	assert(ContextProvider.stringValues.get_keys().size == 0);
	assert(ContextProvider.boolValues.get_keys().size == 0);
	assert(ContextProvider.unknownValues.size == 0);
}

void test_unknownchargepercentage () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, true, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Hide a property and notify the client that it was modified
	Hal.hiddenProperties.add (halPercentage);
	Hal.changePropertyInt (halPercentage, 50, true);

	// Expected results:
	Gee.HashSet<string> unknownValues = ContextProvider.unknownValues;
	assert (unknownValues.contains(keyChargePercentage));
	assert (unknownValues.contains(keyLowBattery)); 
	// note: because discharging = true, this goes unknown as well
}

void test_unknownonbattery () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Hide a property and notify the client that it was modified
	Hal.hiddenProperties.add (halDischarging);
	Hal.changePropertyBool (halDischarging, false, true);

	// Expected results:
	Gee.HashSet<string> unknownValues = ContextProvider.unknownValues;
	assert (unknownValues.contains(keyOnBattery));
}

void test_unknownischarging () {
	// Setup
	ContextProvider.initializeMock ();
	Hal.initializeMock ();
	// Set the initial values for the properties
	Hal.changePropertyInt (halPercentage, 50, false);
	Hal.changePropertyInt (halCharge, 100, false);
	Hal.changePropertyInt (halLastFull, 200, false);
	Hal.changePropertyInt (halRate, 0, false);
	Hal.changePropertyBool (halCharging, false, false);
	Hal.changePropertyBool (halDischarging, false, false);

	Plugins.HalPlugin plugin = new Plugins.HalPlugin();
	plugin.install ();
	// tell the plugin that someone is listening
	ContextProvider.callSubscriptionCallback(true);

	// Test
	// Hide a property and notify the client that it was modified
	Hal.hiddenProperties.add (halCharging);
	Hal.changePropertyBool (halCharging, false, true);

	// Expected results:
	Gee.HashSet<string> unknownValues = ContextProvider.unknownValues;
	assert (unknownValues.contains(keyIsCharging));
}



void debug_null  (string? log_domain, LogLevelFlags log_level, string message)
{
}

public static void main (string[] args) {
	Test.init (ref args);

	if (Test.quiet()) {
		Log.set_handler ("", LogLevelFlags.LEVEL_DEBUG | LogLevelFlags.FLAG_RECURSION, debug_null);
	}

	Test.add_func("/contextd/hal_plugin/test_install", test_install);
	Test.add_func("/contextd/hal_plugin/test_startsubscribing", test_startsubscribing);
	Test.add_func("/contextd/hal_plugin/test_stopsubscribing", test_stopsubscribing);
	Test.add_func("/contextd/hal_plugin/test_chargepercentage", test_chargepercentage);
	Test.add_func("/contextd/hal_plugin/test_onbattery", test_onbattery);
	Test.add_func("/contextd/hal_plugin/test_lowbattery", test_lowbattery);
	Test.add_func("/contextd/hal_plugin/test_ischarging", test_ischarging);
	Test.add_func("/contextd/hal_plugin/test_timeuntillow", test_timeuntillow);
	Test.add_func("/contextd/hal_plugin/test_timeuntilfull", test_timeuntilfull);
	Test.add_func("/contextd/hal_plugin/test_irrelevantchange", test_irrelevantchange);
	Test.add_func("/contextd/hal_plugin/test_unknownchargepercentage", test_unknownchargepercentage);
	Test.add_func("/contextd/hal_plugin/test_unknownonbattery", test_unknownonbattery);
	Test.add_func("/contextd/hal_plugin/test_unknownischarging", test_unknownischarging);
	Test.run ();
}
