/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <mesosphere.hpp>

namespace ams::kern::svc {

    /* =============================    Common    ============================= */

    namespace {

        /* KEFIR: Atmosphere extension SVC. Allows ams::loader (after inspecting MOD0+0x34 LNY2     */
        /* on the main NSO) or sphaira-hbl (before jumping to an old NRO) to mark a process as      */
        /* using the pre-libnx-4.10.0 USER_TLS_BEGIN=0x108 layout. When the flag is set, the kernel */
        /* skips writing the cpu-tick differential at TLS+0x108 and the thread handle at TLS+0x110, */
        /* so old homebrew that owns those slots is not corrupted. Modern processes (system        */
        /* modules, applets, Nintendo applications) never have the flag set and continue to receive */
        /* the upstream Atmosphere 1.11.x kernel writes that 22.x firmware expects.                 */
        Result SetProcessLegacyTlsAbi(ams::svc::Handle process_handle, bool legacy) {
            /* Get the process from its handle. */
            KScopedAutoObject process = GetCurrentProcess().GetHandleTable().GetObject<KProcess>(process_handle);
            R_UNLESS(process.IsNotNull(), svc::ResultInvalidHandle());

            /* Set the legacy TLS ABI flag. The kernel reads this on the next scheduler tick and  */
            /* on the next thread creation; a torn read is benign because the flag is a single   */
            /* bool and only changes once during process setup.                                   */
            process->SetLegacyTlsAbi(legacy);

            R_SUCCEED();
        }

    }

    /* =============================    64 ABI    ============================= */

    Result SetProcessLegacyTlsAbi64(ams::svc::Handle process_handle, bool legacy) {
        R_RETURN(SetProcessLegacyTlsAbi(process_handle, legacy));
    }

    /* ============================= 64From32 ABI ============================= */

    Result SetProcessLegacyTlsAbi64From32(ams::svc::Handle process_handle, bool legacy) {
        R_RETURN(SetProcessLegacyTlsAbi(process_handle, legacy));
    }

}
