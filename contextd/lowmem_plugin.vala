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

using Posix;
using GLib;

namespace Plugins {

	private enum Tristate {
		FALSE,
		TRUE,
		UNKNOWN
	}

	/*
	 * BoolSysfsPoller watches for changes in content of a sysfs attribute whose values are from the {"0",
	 * "1"}.  It emits a state_changed() signal whenever the value changes.
	 */
	private class BoolSysfsPoller: Object {

		public signal void state_changed();

		public Tristate state;

		private IOChannel ioc;

		/*
		 * Some rules for using poll(2) with sysfs:
		 * -- it is not supported by every attribute
		 * -- it reports POLLPRI | POLLER when something interesting happens
		 * -- you have to lseek(2) to the beginning after reading it
		 */
		private bool ioc_ready(IOChannel source, IOCondition condition) {
			if ((condition & (IOCondition.PRI | IOCondition.ERR)) !=
				(IOCondition.PRI | IOCondition.ERR))
				return true;
			state_changed();
			return true;

		}

		public void readFile() {
			try {
				char[] buf = new char[32];
				size_t bread;

				this.ioc.read_chars(buf, out bread);
				if (buf[0] == '0')
					state = Tristate.FALSE;
				else if (buf[0] == '1')
					state = Tristate.TRUE;
				else
					state = Tristate.UNKNOWN;
				this.ioc.seek_position(0, SeekType.SET);
			} catch {
				state = Tristate.UNKNOWN;
			}
		}

		BoolSysfsPoller(string filename) {
			this.state = Tristate.UNKNOWN;
			try {
				this.ioc = new IOChannel.file(filename, "r");
				this.ioc.set_encoding(null);
				this.ioc.set_buffered(false);
				this.ioc.add_watch(IOCondition.PRI | IOCondition.ERR, ioc_ready);
			} catch {
				stderr.printf("Lowmem: failed to open %s\n", filename);
			}
		}
	}

	/*
	 * The LowmemPlugin implements the System.MemoryPressure context property that has three possible
	 * values: normal, high, and critical.	It works by poll(2)ing the (Maemo specific)
	 * /sys/kernel/{low,high}_watermark sysfs attributes.  The following table describes the correspondence
	 * between the attributes and the value of the context property:
	 *
	 * low_watermark | high_watermark | System.MemoryPressure
	 * --------------+----------------+-------------------------
	 *		 0		 |		 0		  |		  normal (0)
	 *		 1		 |		 0		  |		  high (1)
	 *		 1		 |		 1		  |		  critical (2)
	 *		 x		 |		 x		  |		  unspecified (null)
	 */

	internal class LowmemPlugin: Object, ContextD.Plugin {

		private BoolSysfsPoller low_wm;
		private BoolSysfsPoller high_wm;

		/*
		 * As a side note, in Maemo terms:
		 * -- low_watermark means `background killing' is active
		 * -- high_watermark is `low memory condition' (expect the oom killer soon)
		 * Note that these are advisory, and there are cases when the OOM killer strikes without
		 * high_watermark being 1.
		 */

		private static string lowmem_key = "System.MemoryPressure";

		private void attr_changed(BoolSysfsPoller sp) {
			int val;
			low_wm.readFile();
			high_wm.readFile();
			if (low_wm.state == Tristate.FALSE && high_wm.state == Tristate.FALSE) {
				/* normal */
				val = 0;
			} else if (low_wm.state == Tristate.TRUE && high_wm.state == Tristate.FALSE) {
				/* high */
				val = 1;
			} else if (low_wm.state == Tristate.TRUE && high_wm.state == Tristate.TRUE) {
				/* critical */
				val = 2;
			} else if (low_wm.state == Tristate.FALSE && high_wm.state == Tristate.TRUE) {
				/* ignore rogue state */
				return;
			} else {
				/* undefined */
				ContextProvider.set_null(lowmem_key);
				return;
			}
			ContextProvider.set_integer(lowmem_key, val);
		}

		bool install() {
			ContextProvider.install_key(lowmem_key, false, null);
			return true;
		}

		internal LowmemPlugin() {
			low_wm = new BoolSysfsPoller("/sys/kernel/low_watermark");
			low_wm.state_changed.connect(attr_changed);
			high_wm = new BoolSysfsPoller("/sys/kernel/high_watermark");
			high_wm.state_changed.connect(attr_changed);
		}
	}
}
