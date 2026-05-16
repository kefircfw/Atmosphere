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
#include <stratosphere.hpp>
#include "fsmitm_save_utils.hpp"
#include <string>

namespace ams::mitm::fs {

    using namespace ams::fs;

    namespace {

        Result GetSaveDataSpaceIdString(const char **out_str, u8 space_id) {
            switch (static_cast<SaveDataSpaceId>(space_id)) {
                case SaveDataSpaceId::System:
                case SaveDataSpaceId::ProperSystem:
                    *out_str = "sys";
                    break;
                case SaveDataSpaceId::User:
                    *out_str = "user";
                    break;
                case SaveDataSpaceId::SdSystem:
                    *out_str = "sd_sys";
                    break;
                case SaveDataSpaceId::Temporary:
                    *out_str = "temp";
                    break;
                case SaveDataSpaceId::SdUser:
                    *out_str = "sd_user";
                    break;
                case SaveDataSpaceId::SafeMode:
                    *out_str = "safe";
                    break;
                default:
                    R_THROW(fs::ResultInvalidSaveDataSpaceId());
            }

            R_SUCCEED();
        }

        Result GetSaveDataTypeString(const char **out_str, SaveDataType save_data_type) {
            switch (save_data_type) {
                case SaveDataType::System:
                    *out_str = "system";
                    break;
                case SaveDataType::Account:
                    *out_str = "account";
                    break;
                case SaveDataType::Bcat:
                    *out_str = "bcat";
                    break;
                case SaveDataType::Device:
                    *out_str = "device";
                    break;
                case SaveDataType::Temporary:
                    *out_str = "temp";
                    break;
                case SaveDataType::Cache:
                    *out_str = "cache";
                    break;
                case SaveDataType::SystemBcat:
                    *out_str = "system_bcat";
                    break;
                default:
                    /* TODO: Better result? */
                    R_THROW(fs::ResultInvalidArgument());
            }

            R_SUCCEED();
        }

        constexpr inline bool IsEmptyAccountId(const UserId &uid) {
            return uid == InvalidUserId;
        }


    }

    Result SaveUtil::GetDirectorySaveDataPath(char *dst, size_t dst_size, ncm::ProgramId program_id, u8 space_id, const fs::SaveDataAttribute &attribute) {
        /* Saves are stored next to the Nintendo folder for the active emuMMC. */
        char base_save_dir[0x100];
        util::TSNPrintf(base_save_dir, sizeof(base_save_dir), "/atmosphere/saves");

        if (emummc::IsActive()) {
            if (const char *nintendo_path = emummc::GetNintendoDirPath(); nintendo_path != nullptr) {
                util::TSNPrintf(base_save_dir, sizeof(base_save_dir), "%s", nintendo_path);
                /* Strip the last component (e.g., /Nintendo) */
                for (int i = std::strlen(base_save_dir) - 1; i >= 0; --i) {
                    if (base_save_dir[i] == '/' || base_save_dir[i] == '\\') {
                        base_save_dir[i] = '\0';
                        break;
                    }
                }
                /* Append /saves */
                std::strncat(base_save_dir, "/saves", sizeof(base_save_dir) - std::strlen(base_save_dir) - 1);
            }
        }

        /* Get space_id, save_data_type strings. */
        const char *space_id_str, *save_type_str;
        R_TRY(GetSaveDataSpaceIdString(&space_id_str, space_id));
        R_TRY(GetSaveDataTypeString(&save_type_str, attribute.type));

        /* Initialize the path. */
        const bool is_system = attribute.system_save_data_id != InvalidSystemSaveDataId && IsEmptyAccountId(attribute.user_id);
        size_t out_path_len;
        if (is_system) {
            out_path_len = static_cast<size_t>(util::SNPrintf(dst, dst_size, "%s/%016lx/%s", base_save_dir, attribute.system_save_data_id, save_type_str));
        } else {
            out_path_len = static_cast<size_t>(util::SNPrintf(dst, dst_size, "%s/%016lx%016lx/%016lx/%s", base_save_dir, attribute.user_id.data[1], attribute.user_id.data[0], static_cast<u64>(program_id), save_type_str));
        }

        R_UNLESS(out_path_len < dst_size, fs::ResultTooLongPath());

        R_SUCCEED();
    }
}
