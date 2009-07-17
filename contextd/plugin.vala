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

namespace ContextD {
	// The interface for Plugins. Each plugin must implement the initialization
	// it needs inside the install method. At minimum, inside install, it must
	// call install_group or install_key of the ContextProvider libary, to
	// declare the keys it provides. The plugin can choose the other parameters
	// passed, e.g., whether keys should be set to Unknown if nobody is subscribed
	// to them.
	// The install function of the plugin is called in main, after creating the
	// main loop and initializing ContextProvider but before entering the main loop.
	internal interface Plugin : GLib.Object {
		internal abstract bool install();
	}
}