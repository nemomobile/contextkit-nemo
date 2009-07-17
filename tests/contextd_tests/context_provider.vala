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

/* This is a mock implementation to be used in tests. */

namespace ContextProvider {

	// Variables for logging etc.
	public static Gee.HashSet<string> providedKeys;
	
	public static Gee.HashMap<string, int> intValues;
	public static Gee.HashMap<string, double?> doubleValues;
	public static Gee.HashMap<string, bool> boolValues;
	public static Gee.HashMap<string, string> stringValues;
	public static Gee.HashSet<string> unknownValues;

	// Note: dummy assumption: only one callback is needed
	SubscriptionChangedCallback subscriptionCallback;

	// Functions to be called by the test program:

	// Initializes this mock implementation.
	public void initializeMock() {
		
		providedKeys = new Gee.HashSet<string>(str_hash, str_equal);
		
		intValues = new Gee.HashMap<string, int>(str_hash, str_equal);
		doubleValues = new Gee.HashMap<string, double?>(str_hash, str_equal);
		boolValues = new Gee.HashMap<string, bool>(str_hash, str_equal);
		stringValues = new Gee.HashMap<string, string>(str_hash, str_equal);
		unknownValues = new Gee.HashSet<string>(str_hash, str_equal);
	}

	public void resetValues () {
		intValues.clear ();
		doubleValues.clear ();
		boolValues.clear ();
		stringValues.clear ();
		unknownValues.clear ();
	}

	public void callSubscriptionCallback(bool subscribe) {
		subscriptionCallback(subscribe);
	}

	// Mocked functions:

	public void set_integer (string key, int value) {
		intValues.set(key, value);
	}

	public void set_double (string key, double value) {
		doubleValues.set(key, value);
	}

	public void set_boolean (string key, bool value) {
		boolValues.set(key, value);
	}

	public void set_string (string key, string value) {
		stringValues.set(key, value);
	}

	public void set_null (string key) {
		unknownValues.add(key);
	}

	public delegate void SubscriptionChangedCallback(bool subscribe);

	public void install_group ([CCode (array_length = false, array_null_terminated = true)] string[] key_group, bool clear_values_on_subscribe, SubscriptionChangedCallback? subscription_changed_cb) {
		foreach (var key in key_group) {
			providedKeys.add(key);
		}
		subscriptionCallback = subscription_changed_cb;
	}

	public void install_key(string key, bool clear_values_on_subscribe, SubscriptionChangedCallback? subscription_changed_cb) {
		providedKeys.add(key);
		subscriptionCallback = subscription_changed_cb;
	}
}
