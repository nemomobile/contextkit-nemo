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

namespace Hal {


	public Context? contextUsedByPlugin;
	Context.DevicePropertyModified propertyModifiedCallback;

	public const string mockUdi = "mock.udi";

	// Values stored in the mock implementation
	Gee.HashMap<string, int> intProperties;
	Gee.HashMap<string, bool> boolProperties;
	public bool propertyWatchAdded;
	public bool propertyWatchRemoved;
	
	// Hidden properties. If the client tries to query these, it gets an error.
	Gee.HashSet<string> hiddenProperties;

	// Functions to be used by the test program
	public void initializeMock() {
		intProperties = new Gee.HashMap<string, int>(str_hash, str_equal);
		boolProperties = new Gee.HashMap<string, bool>(str_hash, str_equal);
		hiddenProperties = new Gee.HashSet<string>(str_hash, str_equal);

		propertyWatchAdded = false;
		propertyWatchRemoved = false;
	}

	// Sets the value of a property in the mock implementation.
	// If callTheCallback is true, also notifies the client of the change.
	public void changePropertyInt(string key, int value, bool callTheCallback) {

		intProperties.set(key, value);
		if (callTheCallback) {
			propertyModifiedCallback(contextUsedByPlugin, mockUdi, key, false, false);
		}
	} 

	// Sets the value of a property in the mock implementation.
	// If callTheCallback is true, also notifies the client of the change.
	public void changePropertyBool(string key, bool value, bool callTheCallback) {

		boolProperties.set(key, value);
		if (callTheCallback) {
			propertyModifiedCallback(contextUsedByPlugin, mockUdi, key, false, false);
		}
	} 

	public class Context {
		public static delegate void DevicePropertyModified (Context ctx, string udi, string key, bool is_removed, bool is_added);
	

		// Mocked functions:
		public bool init (ref DBus.RawError error) {

			// Store the Context (created by the hal plugin, the class that is
			// currently being tested) into the mock implementation so that
			// we can command it to do things.

			contextUsedByPlugin = this;

			return true;
		}

		public bool set_dbus_connection (DBus.RawConnection conn) {
			return true;
		}

		public bool set_device_property_modified (DevicePropertyModified _callback) {
			
			// Store the callback so that the test can command the mock implementation
			// to call it
			propertyModifiedCallback = _callback;

			return true;
		}

		public string[] find_device_by_capability (string capability, ref DBus.RawError error) {
			string [] toReturn = {mockUdi};
			return toReturn;
		}

		public int device_get_property_int (string udi, string key, ref DBus.RawError error) {
			if (hiddenProperties.contains (key)) {
				error.name = "property hidden";
				return 0;
			}
			return intProperties.get (key);
		}

		public bool device_get_property_bool (string udi, string key, ref DBus.RawError error) {
			
			if (hiddenProperties.contains (key)) {
				error.name = "property hidden";
				return false;
			}
			return boolProperties.get (key);
		}

		public bool device_add_property_watch (string udi, ref DBus.RawError error) {
			if (udi == mockUdi) {
				propertyWatchAdded = true;
			}
			return true;
		}

		public bool device_remove_property_watch (string udi, ref DBus.RawError error) {
			if (udi == mockUdi) {
				debug ("removing property watch");
				propertyWatchRemoved = true;
			}
			return true;
		}
	}
}